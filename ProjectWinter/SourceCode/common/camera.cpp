#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

using namespace glm;

Camera::Camera(GLFWwindow* window) : window(window)
{
    position        = vec3(0, 0.2, 0);
    horizontalAngle = 3.14f;
    verticalAngle   = 0.0f;
    FoV             = 45.0f;
    speed           = 1.0f;
    mouseSpeed      = 0.001f;
    fovSpeed        = 2.0f;
}

void Camera::update()
{
    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime    = float(currentTime - lastTime);

    // Get mouse position
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Reset mouse position for next frame
    glfwSetCursorPos(window, width / 2, height / 2);

    // Compute new horizontal and vertical angles, given windows size
    // and cursor position
    horizontalAngle += mouseSpeed * float(width / 2 - xPos);
    verticalAngle   += mouseSpeed * float(height / 2 - yPos);

    // Right and up vectors of the camera coordinate system
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

    // Update camera position using the direction/right vectors
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // Move forward
        position += direction * deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // Move backward
        position -= direction * deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // Strafe right
        position += right * deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // Strafe left
        position -= right * deltaTime * speed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        position -= up * deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        position += up * deltaTime * speed;

    // Handle zoom in/out effects
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        FoV -= fovSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        FoV += fovSpeed;
    FoV = clamp(FoV, 1.0f, 45.0f);

    // Construct projection and view matrices
    projectionMatrix = perspective(radians(FoV), 4.0f / 3.0f, 0.01f, 100.0f);
    viewMatrix = lookAt(
        position,
        position + direction,
        up
    );

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}
