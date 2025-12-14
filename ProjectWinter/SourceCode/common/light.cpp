#include <glfw3.h>
#include <iostream>
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include "light.h"

Light::Light(GLFWwindow* window, 
             vec4 init_La,
             vec4 init_Ld,
             vec4 init_Ls,
             vec3 init_position) : window(window)
{
    La = init_La;
    Ld = init_Ld;
    Ls = init_Ls;
    lightPosition_worldspace = init_position;
    
    // Initialize to stop compiler warnings...
    nearPlane = 1.0f;
    farPlane  = 100.0f;

    targetPosition = vec3(0.0, 0.0, 0.0);
    direction      = normalize(targetPosition - lightPosition_worldspace);
}

void Light::update() // I assume that my light source will remain static...
{
    // converting direction to cylidrical coordinates
    float x = direction.x;
    float y = direction.y;
    float z = direction.z;

    // We don't need to calculate the vertical angle
    
    float horizontalAngle;
    if      (z > 0.0) horizontalAngle = atan(x/z);
    else if (z < 0.0) horizontalAngle = atan(x/z) + 3.1415f;
    else              horizontalAngle = 3.1415f / 2.0f;

    // Right vector
    vec3 right(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );

    // Up vector
    vec3 up = cross(right, direction);
   
    viewMatrix = lookAt(
        lightPosition_worldspace,
        targetPosition,
        up 
    );
}

mat4 Light::lightVP() { return projectionMatrix * viewMatrix; }

void Light::fitToCameraFrustum(const mat4& cameraView, const mat4& cameraProj)
{
    // Get the 8 corners of the camera frustum in world space
    mat4 invCam = inverse(cameraProj * cameraView);

    vec4 frustumCornersWS[8];
    int i = 0;
    for (int x = 0; x < 2; ++x)
        for (int y = 0; y < 2; ++y)
            for (int z = 0; z < 2; ++z)
            {
                vec4 ndc(
                    x ? 1.0f : -1.0f,
                    y ? 1.0f : -1.0f,
                    z ? 1.0f : -1.0f,
                    1.0f
                );

                vec4 world            = invCam * ndc;
                world                /= world.w; // Perspective divide
                frustumCornersWS[i++] = world;
            }

    // Transform corners to light view space
    float minX =  std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY =  std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float minZ =  std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for (int j = 0; j < 8; ++j)
    {
        vec4 ls = viewMatrix * frustumCornersWS[j];

        minX = std::min(minX, ls.x);
        maxX = std::max(maxX, ls.x);
        minY = std::min(minY, ls.y);
        maxY = std::max(maxY, ls.y);
        minZ = std::min(minZ, ls.z);
        maxZ = std::max(maxZ, ls.z);
    }

    // Small padding so objects right on the edge don't flicker!
    const float padding = 8.0f;
    minX -= padding; maxX += padding;
    minY -= padding; maxY += padding;
    minZ -= padding; maxZ += padding;

    // Setting near and far plane affects the detail of the shadow
    nearPlane = -maxZ;
    farPlane  = -minZ;

    // Build the ortho projection that tightly fits the camera frustum
    projectionMatrix = ortho(minX, maxX, minY, maxY, nearPlane, farPlane);
}
