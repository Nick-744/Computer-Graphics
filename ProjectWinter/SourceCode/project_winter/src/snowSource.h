#ifndef SNOWSOURCE_H
#define SNOWSOURCE_H

#include <glm/glm.hpp>

using namespace glm;

class SnowSource
{
public:
    // Matrices required for the Depth Pass
    mat4 viewMatrix;
    mat4 projectionMatrix;

    vec3 snowSourcePosition_worldspace;
    vec3 targetPosition;
	vec3 direction;

    float nearPlane;
    float farPlane;

    SnowSource(vec3 init_position);

    void update();

    mat4 snowVP();

    void fitToCameraFrustum(const mat4& cameraView, const mat4& cameraProj);
};

#endif
