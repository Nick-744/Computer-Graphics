#include <glm/glm.hpp>

using namespace glm;

class Light {
public:

    GLFWwindow* window;

    // Light parameters
    mat4 viewMatrix;
    mat4 projectionMatrix;

    vec3 lightPosition_worldspace;

    vec4 La;
    vec4 Ld;
    vec4 Ls;

    float nearPlane;
    float farPlane;

    vec3 direction;

    // Where the light will look at
    vec3 targetPosition;

    // Constructor
    Light(GLFWwindow* window,
        vec4 init_La,
        vec4 init_Ld,
        vec4 init_Ls,
        vec3 init_position
    );
    
    void update(); // Calculate the view matrix!

    mat4 lightVP();

    // Fit the orthographic projection to the camera's view frustum!
    void fitToCameraFrustum(const mat4& cameraView, const mat4& cameraProj); // Calculate the projection matrix!
};
