#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <vector>

using namespace std;

class Camera
{
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
    
    bool flyingMode = false; // If true, no terrain height adjustment!

    Camera(GLFWwindow* window);
    bool update(float deltaTime);
};

#endif
