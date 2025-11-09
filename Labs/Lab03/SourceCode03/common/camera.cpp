#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

using namespace glm;

Camera::Camera(GLFWwindow* window) : window(window)
{
    position        = glm::vec3(0, 0, 5);
    horizontalAngle = 0.0f;
    verticalAngle   = 0.0f;
    FoV             = 45.0f;
    speed           = 3.0f;
    mouseSpeed      = 0.1f;
    fovSpeed        = 10.0f;

    tiltAngle = 0.0f;
    tiltSpeed = 4.0f;
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

    // Set up the view matrix to orient and position the camera
    // Looking from 'position' towards 'position - forward direction' with 'up' direction as (0, 1, 0)
    // viewMatrix = lookAt(position, position - vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));


    
    // Task 2: Orient the camera using the mouse

    // Update the horizontal angle based on the mouse’s horizontal movement (left-right).
    horizontalAngle += deltaTime * mouseSpeed * ((float) xPos - width / 2);
    // Update the vertical angle based on the mouse’s vertical movement (up-down).
    verticalAngle   -= deltaTime * mouseSpeed * ((float) yPos - height / 2);

    // Calculate the direction vector for the camera
    vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		- cos(verticalAngle) * cos(horizontalAngle)
    );

    // Compute the right vector of the camera's coordinate system
    vec3 right = cross(direction, vec3(0.0f, 1.0f, 0.0f));
    // Calculate the up vector for the camera's coordinate system
    vec3 up    = cross(right, direction);



    // Update camera position using the direction and right vectors based on user input

    // Task 1: Navigate using WSAD keys

    // Move forward: If 'W' is pressed, move the camera forward along the direction vector
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position += speed * deltaTime * direction;
    // Move backward: If 'S' is pressed, move the camera backward along the direction vector
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position -= speed * deltaTime * direction;
    // Strafe right: If 'D' is pressed, move the camera to the right along the right vector
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position += speed * deltaTime * right;
    // Strafe left: If 'A' is pressed, move the camera to the left along the right vector
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position -= speed * deltaTime * right;



    // Task 3: Zoom in/out effect based on arrow key input

    // Zoom in if the up arrow key is pressed
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        FoV -= deltaTime * fovSpeed;
    // Zoom out if down arrow key is pressed
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        FoV += deltaTime * fovSpeed;



    // Move camera up/down using SPACE and CONTROL keys
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        position += speed * deltaTime * up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        position -= speed * deltaTime * up;
    
    
    
    // Tilt camera sideways left/right using Q and E keys
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        tiltAngle += deltaTime * tiltSpeed; // Αντιορολογιακά
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        tiltAngle -= deltaTime * tiltSpeed; // Ορολογιακά
    else
    {   // Επιστροφή στην αρχική θέση
        tiltAngle -= deltaTime * tiltSpeed * tiltAngle;
        if (abs(tiltAngle) < 0.002f) tiltAngle = 0.0f;
    }

    tiltAngle = clamp(tiltAngle, -3.14f / 6.0f, 3.14f / 6.0f); // Μην στραβολεμιάσει κιόλας...
    mat4 rotationMatrix = rotate(mat4(1.0f), tiltAngle, direction);
    right = vec3(rotationMatrix * vec4(right, 0.0f));
    up    = vec3(rotationMatrix * vec4(up, 0.0f));



    // Update the projection matrix with the new field of view
    projectionMatrix = perspective(radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

    // Alternatively, uncomment the line below for orthographic projection
    // projectionMatrix = ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 15.0f);

    // Update the view matrix to reflect the camera's current position and orientation
    viewMatrix = lookAt(position, position + direction, up);
    
    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}
