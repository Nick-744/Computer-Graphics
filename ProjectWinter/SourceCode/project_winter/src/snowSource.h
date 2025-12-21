#ifndef SNOWSOURCE_H
#define SNOWSOURCE_H

#include <GL/glew.h>
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
    
    GLuint snowDepthFBO     = 0;
    GLuint snowDepthTexture = 0;
};

#endif
