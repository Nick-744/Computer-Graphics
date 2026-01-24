#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include <algorithm> 

using namespace glm;

#define PLAYER_HEIGHT 1.8f
#define WALKING_SPEED 3.5f
#define STICK_DEADZONE 0.2f

extern float cameraFarPlane; // Optimization for shadow mapping...
extern float getTerrainHeight(float x, float z);

Camera::Camera(GLFWwindow* window) : window(window)
{
    position        = vec3(-7.22759f, 60.8374f, 32.76994916f);
    horizontalAngle = 3.14f / 2.1f;
    verticalAngle   = -0.3f;
    FoV             = 45.0f;
    speed           = WALKING_SPEED;
    mouseSpeed      = 0.001f;
    fovSpeed        = 100.0f;
}

bool Camera::update(float deltaTime)
{
    // Static variable to track walking time for head bobbing...
    static float walkTimer = 0.0f;

    int present = glfwJoystickPresent(GLFW_JOYSTICK_1); // Check if joystick is connected!

    int axesCount, buttonCount;
    const float* axes            = NULL;
    const unsigned char* buttons = NULL;
    if (present)
    {
        axes    = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
        buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
    }



    // --- LOOK LOGIC --- //
    if (present && axesCount >= 4)
    {
        float rsX = axes[2];
        float rsY = axes[5]; // Right stick Y

        if (abs(rsX) < STICK_DEADZONE) rsX = 0.0f;
        if (abs(rsY) < STICK_DEADZONE) rsY = 0.0f;



        float controllerSens = 2.5f;
        horizontalAngle     += mouseSpeed * -rsX * controllerSens * 500.0f * deltaTime;
        verticalAngle       += mouseSpeed * -rsY * controllerSens * 500.0f * deltaTime;
    }
    else
    {
        // Get mouse position
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Reset mouse position for next frame
        glfwSetCursorPos(window, width / 2, height / 2);



        // Task 5.3: Compute new horizontal and vertical angles, given windows size
        // and cursor position
        horizontalAngle += mouseSpeed * float(width / 2 - xPos);
        verticalAngle   += mouseSpeed * float(height / 2 - yPos);
    }

    // Don't flip over...
    float temp    = 1.1f;
    verticalAngle = clamp(verticalAngle, -temp, temp);



    // Task 5.4: right and up vectors of the camera coordinate system
    // use spherical coordinates
    vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );

    // Right vector
    vec3 right(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );

    // Up vector
    vec3 up = cross(right, direction);



    // Task 5.5: update camera position using the direction/right vectors
    bool isMoving = false; // For head bobbing!
    vec3 moveDir(0.0f);    // Movement direction vector...
    if (present && axesCount >= 2)
    {
        float lsX = axes[0];
        float lsY = axes[1];

        if (abs(lsX) > STICK_DEADZONE || abs(lsY) > STICK_DEADZONE)
        {
            vec3 forwardFlat = normalize(vec3(direction.x, 0.0f, direction.z));
            vec3 rightFlat   = normalize(vec3(right.x, 0.0f, right.z));

            vec3 moveBasisF = flyingMode ? direction : forwardFlat;
            vec3 moveBasisR = flyingMode ? right : rightFlat;

            moveDir += moveBasisF * -lsY;
            moveDir += moveBasisR * lsX;
        }
    }
    else if (flyingMode)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += direction;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= direction;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += right;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= right;
    }
    else
    {
        // Prevent glitching when looking straight down and moving!
        vec3 forwardFlat = normalize(vec3(direction.x, 0.0f, direction.z));
        vec3 rightFlat   = normalize(vec3(right.x,     0.0f, right.z));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += forwardFlat;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= forwardFlat;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += rightFlat;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= rightFlat;
    }

    if (length(moveDir) > 0.001f)
    {
        if (!present) moveDir = normalize(moveDir); // Prevent faster diagonal movement...
        position += moveDir * deltaTime * speed;

        isMoving = true;
    }



    // ===< FLYING MODE TOGGLE >=== //
    static int lastToggleState = GLFW_RELEASE; // Remembered between frames
    int currentToggleState     = glfwGetKey(window, GLFW_KEY_F);

    if (currentToggleState == GLFW_PRESS && lastToggleState == GLFW_RELEASE)
    {
        flyingMode = !flyingMode;
        speed      = flyingMode ? 20.0f : WALKING_SPEED;
    }
    lastToggleState = currentToggleState; // Didn't want to overcomplicate things with pollKeyboard...

    vec3 eyePosition = position;
    if (!flyingMode) // Realistic camera movement...
    {
        // ===< TERRAIN HEIGHT ADJUSTMENT >=== //
        {
            // Calculate height of the ground at current (x, z)
            float groundHeight = getTerrainHeight(position.x, position.z);
            float targetHeight = groundHeight + PLAYER_HEIGHT;

            // Smoothly interpolate current Y towards target Y!
            float climbSpeed = 8.0f; // Smoothing speed...
            position.y      += (targetHeight - position.y) * climbSpeed * deltaTime;

            // Prevent glitching...
            if (abs(position.y - targetHeight) < 0.01f) position.y = targetHeight;
        }

        // ===< HEAD BOBBING >=== //

        static float bobBlend = 0.0f; // 0 = stopped - 1 = moving
        if (isMoving)
        {
            bobBlend += deltaTime * 4.0f; // Start walk...
            if (bobBlend > 1.0f) bobBlend = 1.0f;
        }
        else
        {
            bobBlend -= deltaTime * 4.0f; // Smooth stop...
            if (bobBlend < 0.0f) bobBlend = 0.0f;
        }

        if (bobBlend > 0.001f) // Apply bobbing only if walking/stopping!
        {
            // Increment timer based on movement speed
            walkTimer += deltaTime * speed * 2.5f;

            // Vertical Bob (Up/Down)
            float bobOffset = sin(walkTimer) * 0.1f * bobBlend;
            eyePosition.y  += bobOffset;

            // Horizontal Sway (Left/Right) - Simulates weight shifting...
            float swayOffset = cos(walkTimer * 0.5f) * 0.08f * bobBlend;
            eyePosition     += right * swayOffset;
        }
        else walkTimer = 0.0f; // Reset timer to 0!
    }



    // Task 5.6: handle ZOOM EFFECT!
    float targetFoV = 45.0f; // Default Base FoV

    // If SPACE is held, ZOOOOOOOOM IN!
    bool zoomPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (present && buttonCount >= 6)
        if (buttons[5] == GLFW_PRESS) zoomPressed = true; // Check R1 button...

    if (zoomPressed) targetFoV = 20.0f;

    // Smoothly move current FoV towards the Target...
    if (FoV < targetFoV)
    {
        FoV += fovSpeed * deltaTime;
        if (FoV > targetFoV) FoV = targetFoV; // Clamp
    }
    else if (FoV > targetFoV)
    {
        FoV -= fovSpeed * deltaTime;
        if (FoV < targetFoV) FoV = targetFoV; // Clamp
    }



    // Task 5.7: construct projection and view matrices
    projectionMatrix = perspective(radians(FoV), 4.0f / 3.0f, 0.1f, cameraFarPlane);
    viewMatrix       = lookAt(
        eyePosition,
        eyePosition + direction,
        up
    );

    return isMoving;
}
