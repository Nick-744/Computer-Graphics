#include "snowball.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

extern float getTerrainHeight(float x, float z);

Snowball::Snowball(Drawable* sphereMesh, GLuint shaderProgram, float maxRadiusOverride) : mesh(sphereMesh)
{
    velocity = vec3(0.0f);
    radius   = 0.1f;
    mass     = 1.0f;

    // Set max radius (default 0.36f or override for smaller ball)
    MAX_RADIUS = (maxRadiusOverride > 0.0f) ? maxRadiusOverride : 0.36f;

    // Load Uniforms
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");

    // Material Uniforms
    KaLocation = glGetUniformLocation(shaderProgram, "mtl.Ka");
    KdLocation = glGetUniformLocation(shaderProgram, "mtl.Kd");
    KsLocation = glGetUniformLocation(shaderProgram, "mtl.Ks");
    NsLocation = glGetUniformLocation(shaderProgram, "mtl.Ns");
}

Snowball::~Snowball() {}

void Snowball::update(float deltaTime, vec3 currentPlayerPos, vec3 prevPlayerPos)
{
    // Early exits for static states
    if (!active || stacked || held || isMaxSize)
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

        // Shift the wall center forward - FIX COLLISION TRAP!
        float pushOffset    = PLAYER_RADIUS * 0.6f;
        vec3 virtualWallPos = currentPlayerPos + (wallNormal * pushOffset);

        // Vector from player to snowball
        vec3 toBall = position - virtualWallPos;

        // Projected distances
        float distAlongNormal = dot(toBall, wallNormal);

        // How far the ball is to the side of the player center
        vec3 lateralVec   = toBall - (distAlongNormal * wallNormal);
        float distLateral = length(lateralVec);

        float wallThreshold = radius * 1.2f; // Slightly ahead for smoother contact

        // Collision Check
        if (distAlongNormal > 0.0f && distAlongNormal < wallThreshold && distLateral < PLAYER_RADIUS)
        {
            // Physics: Use the player's actual velocity for the push strength
            float pushStrength = moveDist / (deltaTime + 0.0001f);
            float acceleration = (pushStrength * 10.0f) / mass;

            // Smooth the velocity change
            vec3 targetVelocity = velocity + wallNormal * acceleration * deltaTime;
            velocity            = mix(velocity, targetVelocity, 0.4f); // Smooth interpolation

            float penetration = wallThreshold - distAlongNormal;

            // Only correct position if there's significant penetration
            if (penetration > 0.001f)
            {
                // Smooth Static Resolution with interpolation
                vec3 targetPos = virtualWallPos + wallNormal * wallThreshold + lateralVec;

                // Gradual correction instead of instant snap
                float correctionFactor = min(penetration / radius, 1.0f);
                position               = mix(position, targetPos, correctionFactor * 0.5f);
            }
        }
    }

    // --- Movement & Growth --- //
    float speed = length(velocity);
    if (speed > 0.01f)
    {
        float moveStep = speed * deltaTime;
        position      += velocity * deltaTime;
        velocity      *= FRICTION;

        // Growth Logic
        if (radius < MAX_RADIUS)
        {
            radius    = min(radius + moveStep * GROWTH_RATE, MAX_RADIUS);
            isMaxSize = radius >= MAX_RADIUS;

            // Update mass based on volume
            mass = 1.0f + (pow(radius, 3.0f) * 100.0f);
        }
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

void Snowball::draw()
{
    mat4 modelMatrix = getModelMatrix();
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    // Material: White Snow
    glUniform4f(KaLocation, 0.4f, 0.4f, 0.4f, 1.0f);
    glUniform4f(KdLocation, 0.9f, 0.9f, 0.9f, 1.0f);
    glUniform4f(KsLocation, 0.1f, 0.1f, 0.1f, 1.0f);
    glUniform1f(NsLocation, 1.0f);

    glUniform1i(useTextureLocation, 0);
    mesh->bind(); mesh->draw();
}

void Snowball::drawOnlyObjects(GLuint shadowModelLocation)
{
    mat4 modelMatrix = getModelMatrix();
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    mesh->bind(); mesh->draw();
}
