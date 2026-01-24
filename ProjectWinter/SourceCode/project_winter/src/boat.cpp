#include "boat.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>

extern SoundManager soundSystem;

// Constructor now correctly assigns the window pointer to prevent crashes
Boat::Boat(GLuint shaderProgram, GLFWwindow* window) : window(window)
{
    // Initial world placement
    position      = INITIAL_POSITION;
    rotationAngle = INITIAL_ROTATION;
    scaleFactor   = 2.0f;

    // Initialize animation timers
    totalTime        = 0.0f;
    leftPaddleTimer  = 0.0f;
    rightPaddleTimer = 0.0f;

    updateModelMatrix();

    // Link shader uniforms
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");

    // Load Assets
    boatMesh = new Drawable("assets/boat/boat.obj");
    paddleL  = new Drawable("assets/boat/paddle_left.obj");
    paddleR  = new Drawable("assets/boat/paddle_right.obj");

    boatTexture = loadBMP("assets/boat/wood_BaseColor.bmp");
}

Boat::~Boat()
{
    delete boatMesh;
    delete paddleL;
    delete paddleR;

    glDeleteTextures(1, &boatTexture);
}

mat4 Boat::getViewMatrix()
{
    // Calculate direction based on the boat's rotation angle!
    vec3 direction = vec3(sin(rotationAngle), 0, cos(rotationAngle));

    // Offset the player to be slightly above and inside the boat
    vec3 eye = position + vec3(0.0f, 1.5f, 0.0f) - direction * 2.5f;

    return lookAt(eye, eye + direction, vec3(0, 1, 0));
}

void Boat::updateModelMatrix()
{
    // Procedural wobble to simulate water waves!
    float pitch = sin(totalTime * 1.5f) * 0.04f; // Forward/Backward tip
    float roll  = cos(totalTime * 1.0f) * 0.03f; // Side-to-side tip

    modelMatrix = translate(mat4(), position)
                * rotate(mat4(), rotationAngle, vec3(0, 1, 0))
                * rotate(mat4(), pitch, vec3(1, 0, 0))
                * rotate(mat4(), roll, vec3(0, 0, 1))
                * scale(mat4(), vec3(scaleFactor));
}

void Boat::draw()
{
    // Render boat over everything when player is on it!
    if (onBoat) glClear(GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(useTextureLocation, 1);

    // Bind and Draw Boat
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, boatTexture);
    glUniform1i(diffuseColorSampler, 0);

    boatMesh->bind(); boatMesh->draw();

    // Draw Animated Paddles
    mat4 mLeft = getPaddleTransform(leftPaddleTimer, true);
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &mLeft[0][0]);
    paddleL->bind(); paddleL->draw();

    mat4 mRight = getPaddleTransform(rightPaddleTimer, false);
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &mRight[0][0]);
    paddleR->bind(); paddleR->draw();
}

void Boat::drawOnlyObjects(GLuint shadowModelLocation)
{
    // Ensure animated shadows match the rendered models
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    boatMesh->bind(); boatMesh->draw();

    mat4 mLeft = getPaddleTransform(leftPaddleTimer, true);
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &mLeft[0][0]);
    paddleL->bind(); paddleL->draw();

    mat4 mRight = getPaddleTransform(rightPaddleTimer, false);
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &mRight[0][0]);
    paddleR->bind(); paddleR->draw();
}

// Wobble effect update - keeps the boat animated even when idle!
void Boat::update(float deltaTime)
{
    totalTime += deltaTime;
    updateModelMatrix();
}

void Boat::steer(float deltaTime)
{
    // Capture Input
    bool movingForward  = false;
    bool movingBackward = false;
    bool steeringLeft   = false;
    bool steeringRight  = false;

    float moveAxis = 0.0f; // Forward/Backward
    float turnAxis = 0.0f; // Left/Right

    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);

    if (present)
    {
        int axesCount;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);

        if (axesCount >= 2)
        {
            // Left Stick: Axis 0 (Horizontal), Axis 1 (Vertical)
            float lsX = axes[0];
            float lsY = axes[1];

            // Apply deadzone to avoid "ghost" steering
            if (abs(lsX) < 0.2f) lsX = 0.0f;
            if (abs(lsY) < 0.2f) lsY = 0.0f;

            moveAxis = -lsY; // Invert Y because pushing up is usually -1.0
            turnAxis = -lsX; // Invert X for correct steering direction

            // Determine if we are moving enough to trigger animations
            if (moveAxis >  0.1f) movingForward  = true;
            if (moveAxis < -0.1f) movingBackward = true;
            if (turnAxis >  0.1f) steeringLeft   = true;
            if (turnAxis < -0.1f) steeringRight  = true;
        }
    }
    else // Fallback to Keyboard
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { movingForward  = true; moveAxis =  1.0f; }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { movingBackward = true; moveAxis = -1.0f; }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { steeringLeft   = true; turnAxis =  1.0f; }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { steeringRight  = true; turnAxis = -1.0f; }
    }

    // --- BOAT INERTIA (Linear) --- //
    float targetSpeed = 0.0f;
    if (movingForward)  targetSpeed =  MAX_SPEED;
    if (movingBackward) targetSpeed = -MAX_SPEED;

    if (abs(targetSpeed) > 0.1f)
    {
        // Accelerate toward target
        if (currentSpeed < targetSpeed) currentSpeed += ACCELERATION * deltaTime;
        if (currentSpeed > targetSpeed) currentSpeed -= ACCELERATION * deltaTime;
    }
    else
    {
        // Apply friction when no input
        if (currentSpeed > 0) currentSpeed = (std::max) (0.0f, currentSpeed - FRICTION * deltaTime);
        if (currentSpeed < 0) currentSpeed = (std::min) (0.0f, currentSpeed + FRICTION * deltaTime);
    }

    // --- STEERING INERTIA (Angular) --- //
    float targetTurn = 0.0f;
    if (steeringLeft)  targetTurn =  MAX_TURN_SPEED;
    if (steeringRight) targetTurn = -MAX_TURN_SPEED;

    if (abs(targetTurn) > 0.1f)
        currentTurnSpeed += (targetTurn > 0 ? TURN_ACCEL : -TURN_ACCEL) * deltaTime;
    else
    {
        if (currentTurnSpeed > 0) currentTurnSpeed = (std::max) (0.0f, currentTurnSpeed - TURN_FRICTION * deltaTime);
        if (currentTurnSpeed < 0) currentTurnSpeed = (std::min) (0.0f, currentTurnSpeed + TURN_FRICTION * deltaTime);
    }
    currentTurnSpeed = glm::clamp(currentTurnSpeed, -MAX_TURN_SPEED, MAX_TURN_SPEED);

    // Update World Transforms
    rotationAngle += currentTurnSpeed * deltaTime;
    vec3 direction = vec3(sin(rotationAngle), 0, cos(rotationAngle));
    position      += direction * currentSpeed * deltaTime;

    // --- SYNCHRONIZATION LOGIC --- //
    float animSpeed = 4.0f;
    if (movingForward || movingBackward)
    {
        // When moving straight, both paddles must use the same timer...
        leftPaddleTimer += deltaTime * animSpeed;
        rightPaddleTimer = leftPaddleTimer; // Lock them together
    }
    else
    {
        // When only turning, update them independently!
        if (steeringRight) leftPaddleTimer  += deltaTime * animSpeed;
        if (steeringLeft)  rightPaddleTimer += deltaTime * animSpeed;
    }

    // Finalize by updating the model matrix so the changes render
    updateModelMatrix();
}

// Paddles animation...
mat4 Boat::getPaddleTransform(float timer, bool isLeft)
{
    if (!onBoat) // Idle position!
    {
        if (isLeft)
            return modelMatrix
                 * translate(mat4(), vec3(0.0f, -0.221f, -0.5f))
                 * rotate(mat4(), radians(-15.0f), vec3(1, 0, 0))
                 * rotate(mat4(), radians(-120.0f), vec3(0, 1, 0));

        return modelMatrix
             * translate(mat4(), vec3(-0.2f, -0.228f, -0.5f))
             * rotate(mat4(), radians(5.0f), vec3(1, 0, 0))
             * rotate(mat4(), radians(140.0f), vec3(0, 1, 0))
             * rotate(mat4(), radians(-25.0f), vec3(0, 0, 1));
    }

    float swing =  sin(timer) * 0.5f; // Horizontal rowing swing
    float dip   = -cos(timer) * 0.4f; // Vertical dip into water

    {   // Play splash sound on paddle dip!
        static float lastDipLeft     = 0.0f;
        static float lastDipRight    = 0.0f;
        static bool soundPlayedLeft  = false;
        static bool soundPlayedRight = false;

        float& lastDip    = isLeft ? lastDipLeft     : lastDipRight;
        bool& soundPlayed = isLeft ? soundPlayedLeft : soundPlayedRight;

        if (lastDip > dip && dip <= -0.35f && !soundPlayed)
        {
            soundSystem.play("assets/sounds/boat_paddles.ogg");
            soundPlayed = true;
        }
        // Reset flag when paddle comes back up
        if (dip > -0.2f) soundPlayed = false;

        lastDip = dip;
    }

    return modelMatrix
         * translate(mat4(), vec3(isLeft ? 0.1f : -0.1f, 0.0f, 0.0f))
         * rotate(mat4(), swing, vec3(0, isLeft? 1 : -1, 0))
         * rotate(mat4(), dip,   vec3(1, 0, 0));
}

bool Boat::checkCollision(const vec3& position, float radius)
{
    if (boatMesh->checkCollision(position, radius, modelMatrix)) return true;

    return false;
}
