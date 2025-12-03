#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera {
public:
    GLFWwindow* window;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    glm::vec3 position;    // Initial position         : on +Z
    float horizontalAngle; // Initial horizontal angle : toward -Z
    float verticalAngle;   // Initial vertical angle   : none

    // Field of View
    float FoV;
    float speed; // units / second
    float mouseSpeed;
    float fovSpeed;

    Camera(GLFWwindow* window);
    void update();
};

#endif
