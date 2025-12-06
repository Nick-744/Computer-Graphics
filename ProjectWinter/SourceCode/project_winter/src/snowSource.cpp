#include "snowSource.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

SnowSource::SnowSource(glm::vec3 init_position)
{
    snowSourcePosition_worldspace = init_position;

    nearPlane = 200.0f;
    farPlane  = 600.0f;
        
    float snowAreaSize = 200.0f;

    projectionMatrix = ortho(
        -snowAreaSize, snowAreaSize,
        -snowAreaSize, snowAreaSize,
        nearPlane, farPlane
    );
    
    targetPosition = vec3(0.0, 0.0, 0.0);
    direction      = normalize(targetPosition - snowSourcePosition_worldspace);
}

void SnowSource::update()
{
    // converting direction to cylidrical coordinates
    float x = direction.x;
    float y = direction.y;
    float z = direction.z;

    // We don't need to calculate the vertical angle

    float horizontalAngle;
    if (z > 0.0)      horizontalAngle = atan(x / z);
    else if (z < 0.0) horizontalAngle = atan(x / z) + 3.1415f;
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
        snowSourcePosition_worldspace,
        targetPosition,
        up
    );
}

glm::mat4 SnowSource::snowVP() { return projectionMatrix * viewMatrix; }
