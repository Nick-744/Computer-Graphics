#include "boat.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>

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
    float speed     = 2.5f; // Movement speed
    float turnSpeed = 0.3f; // Rotation speed
    float animSpeed = 5.0f;

    bool movingForward  = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    bool movingBackward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    bool steeringLeft   = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    bool steeringRight  = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    // Calculate Direction Vector from current rotation
    vec3 direction = vec3(sin(rotationAngle), 0, cos(rotationAngle));

    // Movement updates
    if (movingForward)  position += direction * speed * deltaTime;
    if (movingBackward) position -= direction * speed * deltaTime;
    if (steeringLeft)   rotationAngle += turnSpeed * deltaTime;
    if (steeringRight)  rotationAngle -= turnSpeed * deltaTime;

    // --- SYNCHRONIZATION LOGIC --- //
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
    float swing =  sin(timer) * 0.5f; // Horizontal rowing swing
    float dip   = -cos(timer) * 0.4f; // Vertical dip into water

    return modelMatrix
         * rotate(mat4(), swing, vec3(0, isLeft? 1 : -1, 0))
         * rotate(mat4(), dip,   vec3(1, 0, 0))
         * translate(mat4(),     vec3(0.0f, 0.05f, 0.0f));
}
