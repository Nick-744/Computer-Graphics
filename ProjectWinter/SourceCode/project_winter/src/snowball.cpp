#include "snowball.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

extern float getTerrainHeight(float x, float z);

Snowball::Snowball(Drawable* sphereMesh, float maxRadiusOverride) : mesh(sphereMesh)
{
    velocity = vec3(0.0f);
    radius   = 0.1f;
    mass     = 1.0f;

    // Set max radius (default 0.36f or override for smaller ball)
    MAX_RADIUS = (maxRadiusOverride > 0.0f) ? maxRadiusOverride : 0.36f;
}

Snowball::~Snowball() {}

void Snowball::update(float deltaTime, vec3 currentPlayerPos, vec3 prevPlayerPos)
{
    if (!active || stacked) return;

    if (held)
    {
        velocity = vec3(0.0f);
        return;
    }

    // Don't push max-sized balls...
    if (isMaxSize)
    {
        velocity = vec3(0.0f);
        return;
    }

    // Pushing Physics Logic
    vec3 movementVec = currentPlayerPos - prevPlayerPos;
    float moveDist   = length(movementVec);

    // Treat the player as a wall if moving
    if (moveDist > 0.0001f)
    {
        vec3 wallNormal = normalize(movementVec);
        wallNormal.y    = 0; // Keep the push horizontal!

        // Vector from player to snowball
        vec3 toBall = position - currentPlayerPos;

        // Projected distances
        float distAlongNormal = dot(toBall, wallNormal);

        // How far the ball is "to the side" of the player center
        vec3 lateralVec   = toBall - (distAlongNormal * wallNormal);
        float distLateral = length(lateralVec);

        // Wall Dimensions
        float wallWidth     = 1.2f;
        float wallThreshold = radius * 1.4f; // Slightly ahead for smoother contact

        // Collision Check
        if (distAlongNormal > 0.0f && distAlongNormal < wallThreshold && distLateral < wallWidth)
        {
            // Physics: Use the player's actual velocity for the push strength
            float pushStrength = moveDist / (deltaTime + 0.0001f);
            float acceleration = (pushStrength * 10.0f) / mass;

            // Smooth the velocity change
            vec3 targetVelocity = velocity + wallNormal * acceleration * deltaTime;
            velocity            = mix(velocity, targetVelocity, 0.4f); // Smooth interpolation

            // Smooth Static Resolution with interpolation
            vec3 targetPos    = currentPlayerPos + wallNormal * wallThreshold + lateralVec;
            float penetration = wallThreshold - distAlongNormal;

            // Only correct position if there's significant penetration
            if (penetration > 0.01f)
            {
                // Gradual correction instead of instant snap
                float correctionFactor = min(penetration / radius, 1.0f);
                position               = mix(position, targetPos, correctionFactor * 0.5f);
            }
        }
    }

    // --- Movement & Growth --- //
    if (length(velocity) > 0.01f)
    {
        float moveStep = length(velocity) * deltaTime;
        position      += velocity * deltaTime;

        if (radius < MAX_RADIUS)
        {
            radius += moveStep * GROWTH_RATE;
            mass    = 1.0f + ((pow(radius, 3.0f)) * 100.0f); // Noticeable mass

            // Check if reached max size
            if (radius >= MAX_RADIUS)
            {
                radius    = MAX_RADIUS;
                isMaxSize = true;
            }
        }

        velocity *= FRICTION;
    }
    else velocity = vec3(0.0f);

    // Ensure it reaches minimum start size quickly...
    if (radius < 0.1f) radius += deltaTime * 0.4f;

    // Terrain Clamping
    float terrainHeight = getTerrainHeight(position.x, position.z);
    position.y          = terrainHeight + radius * 1.6f; // * 1.6 to match the obj's radius!
}

void Snowball::spawn(vec3 pos)
{
    position  = pos;
    velocity  = vec3(0.0f);
    radius    = 0.01f;
    active    = true;
    held      = false;
    stacked   = false;
	isMaxSize = false;
    stackedOn = nullptr;
}

mat4 Snowball::getModelMatrix()
{
    return translate(mat4(), position) * scale(mat4(), vec3(radius));
}

void Snowball::draw(
    GLuint modelMatrixLocation,
    GLuint useTextureLocation,
    GLuint KaLoc, GLuint KdLoc, GLuint KsLoc, GLuint NsLoc)
{
    mat4 modelMatrix = getModelMatrix();
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    // Material: White Snow
    glUniform4f(KaLoc, 0.4f, 0.4f, 0.4f, 1.0f);
    glUniform4f(KdLoc, 0.9f, 0.9f, 0.9f, 1.0f);
    glUniform4f(KsLoc, 0.1f, 0.1f, 0.1f, 1.0f);
    glUniform1f(NsLoc, 1.0f);

    glUniform1i(useTextureLocation, 0);
    mesh->bind(); mesh->draw();
}

void Snowball::drawOnlyObjects(GLuint modelMatrixLocation)
{
    mat4 modelMatrix = getModelMatrix();
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    mesh->bind(); mesh->draw();
}
