#include "clouds.h"

#include <common/shader.h>
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include <cstdlib> // Random

using namespace std;
using namespace glm;

// ========================================= //
//               CONFIGURATION               //
// ========================================= //
const float AREA_WIDTH  = 1000.0f; // Total width (X axis)
const float AREA_LENGTH = 1000.0f; // Total length (Z axis) for looping
const float MIN_Y       = 250.0f;  // Lowest cloud height
const float MAX_Y       = 350.0f;  // Highest cloud height
const float CLOUD_SPEED = 50.0f;   // How fast they move!
const int   AVG_COUNT   = 30;      // Target number of clouds
// ==========================================

struct CloudVertex
{
    vec3 centerOffset; // The center of the specific particle relative to cloud center
    vec2 uv;           // UV coordinates (also used to determine corner direction)
};

// Helper for random float between -1 and 1...
float randomFloat() { return (float) rand() / (float) RAND_MAX * 2.0f - 1.0f; }
float random01()    { return (float) rand() / (float) RAND_MAX; } // [0.0, 1.0]

CloudRenderer::CloudRenderer()
{
    shaderProgram = loadShaders("shaders/clouds.vertexshader", "shaders/clouds.fragmentshader");

    // Get Uniform Locations
    vpLocation          = glGetUniformLocation(shaderProgram, "VP");
    cameraRightLocation = glGetUniformLocation(shaderProgram, "CameraRight");
    cameraUpLocation    = glGetUniformLocation(shaderProgram, "CameraUp");
    cloudBaseLocation   = glGetUniformLocation(shaderProgram, "cloudBase");
    cloudDetailLocation = glGetUniformLocation(shaderProgram, "cloudDetail");
    
    // For the fog effect...
    fogDensityLocation = glGetUniformLocation(shaderProgram, "fogDensity");
    fogColorLocation   = glGetUniformLocation(shaderProgram, "fogColor");

    // Build a cluster of 50 billboards (puffs)
    buildCloudMesh(50, 15.0f);

    // --- GENERATE RANDOM CLOUDS --- //
    // Variation: +/- 5 clouds (so 15 to 25 clouds)
    int count = AVG_COUNT + (rand() % 10 - 5);

    for (int i = 0; i < count; i++)
    {
        CloudInstance c;

        // Random Position in the Area
        // X: -Width/2 to Width/2
        // Y: MinY to MaxY
        // Z: -Length/2 to Length/2
        float x = (random01() - 0.5f) * AREA_WIDTH;
        float y = MIN_Y + random01() * (MAX_Y - MIN_Y);
        float z = (random01() - 0.5f) * AREA_LENGTH;

        c.startPosition = vec3(x, y, z);

        // Random Scale (Flatter and wider)
        float s = 1.0f + random01(); // [1.0, 2.0]
        c.scale = vec3(s * 1.5f, s * 0.8f, s * 1.5f);

        clouds.push_back(c);
    }

    // Initialize Instancing Buffer
    instanceMatrices.resize(clouds.size()); // Pre-allocate CPU buffer

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // Pre-allocate space for all cloud matrices
    glBufferData(GL_ARRAY_BUFFER, clouds.size() * sizeof(mat4), nullptr, GL_DYNAMIC_DRAW);

    // Bind instance attributes to VAO
    glBindVertexArray(vao);
    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*) (sizeof(vec4) * i));
        glVertexAttribDivisor(2 + i, 1); // IMPORTANT: Advance once per INSTANCE!
    }
    glBindVertexArray(0);

    cloudBaseTexture   = loadBMP("assets/clouds_textures/cloud_base.bmp");
    cloudDetailTexture = loadBMP("assets/clouds_textures/cloud_detail.bmp");

    // Better texture wrapping for noise
    glBindTexture(GL_TEXTURE_2D, cloudBaseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glBindTexture(GL_TEXTURE_2D, cloudDetailTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

void CloudRenderer::buildCloudMesh(int numParticles, float radius)
{
    vector<CloudVertex> vertices;
    vector<GLuint>      indices;

    for (int i = 0; i < numParticles; ++i)
    {
        // Generate random positions inside a flattened sphere (Ellipsoid)
        vec3 offset = vec3(
            randomFloat() * radius * 1.5f, // Wider in X
            randomFloat() * radius * 0.6f, // Thinner in height
            randomFloat() * radius * 1.2f  // Wide in Z
        );

        // Push 4 vertices for one quad (billboard)
        // Note: position is the SAME for all 4 vertices (the center of the puff)
        // The vertex shader will expand them using the UVs.

        GLuint base = (GLuint)vertices.size();

        vertices.push_back({ offset, vec2(0.0f, 0.0f) }); // 0: Bottom-Left
        vertices.push_back({ offset, vec2(1.0f, 0.0f) }); // 1: Bottom-Right
        vertices.push_back({ offset, vec2(1.0f, 1.0f) }); // 2: Top-Right
        vertices.push_back({ offset, vec2(0.0f, 1.0f) }); // 3: Top-Left

        // 2 triangles
        indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);

        indices.push_back(base + 2); indices.push_back(base + 3); indices.push_back(base + 0);
    }

    indexCount = (GLsizei) indices.size();

	// --- UPLOAD TO GPU --- //
    glGenVertexArrays(1, &vao);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER,         vbo);
    glBufferData(GL_ARRAY_BUFFER,        vertices.size() * sizeof(CloudVertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),      indices.data(),  GL_STATIC_DRAW);

    // Center Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CloudVertex), (void*) 0);

    // UV / Corner info
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CloudVertex), (void*) offsetof(CloudVertex, uv));

    glBindVertexArray(0);
}

CloudRenderer::~CloudRenderer()
{
    glDeleteProgram(shaderProgram);

    glDeleteVertexArrays(1, &vao);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glDeleteTextures(1, &cloudBaseTexture);
    glDeleteTextures(1, &cloudDetailTexture);
}

void CloudRenderer::draw(const mat4& view, const mat4& proj, float time, vec3 fogColor, float fogDensity)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth writing so inner particles don't occlude outer ones weirdly
    // But keep depth testing ON so they hide behind buildings/mountains
    glDepthMask(GL_FALSE);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudBaseTexture);
    glUniform1i(cloudBaseLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudDetailTexture);
    glUniform1i(cloudDetailLocation, 1);
    
    // --- FOG UNIFORMS --- //
    glUniform3fv(fogColorLocation, 1, &fogColor[0]);
    glUniform1f(fogDensityLocation, fogDensity);

    // --- BILLBOARDING MATH --- //
    // We need these so the clouds face the camera!
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp    = vec3(view[0][1], view[1][1], view[2][1]);
    glUniform3fv(cameraRightLocation, 1, &cameraRight[0]);
    glUniform3fv(cameraUpLocation,    1, &cameraUp[0]);

    mat4 vp = proj * view;
    glUniformMatrix4fv(vpLocation, 1, GL_FALSE, &vp[0][0]);

    // --- MOVEMENT CALCULATION --- //
    float zMin      = -AREA_LENGTH / 2.0f;
    float zMax      =  AREA_LENGTH / 2.0f;
    float mapLength = AREA_LENGTH;
    
    float globalZOffset = time * CLOUD_SPEED; // So movement is noticeable!

    // Update all matrices in the local buffer
    for (size_t i = 0; i < clouds.size(); i++)
    {
        const CloudInstance& c = clouds[i];

        // Calculate new Z
        // We subtract offset because we want to move along -Z
        float currentZ = c.startPosition.z - globalZOffset;

        // Loop Logic (Wrap around)
        // We use fmod to wrap the value within the map length
        // Shift Z so it's positive relative to a far point, mod it, then shift back
        float relativeZ = currentZ - zMax;
        float wrappedZ  = zMax + fmod(relativeZ, mapLength);

        // Handle negative result of fmod
        if (wrappedZ < zMin) wrappedZ += mapLength;

        vec3 currentPos     = vec3(c.startPosition.x, c.startPosition.y, wrappedZ);
        instanceMatrices[i] = translate(mat4(1.0f), currentPos) * scale(mat4(1.0f), c.scale);
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceMatrices.size() * sizeof(mat4), instanceMatrices.data());

    // Single Instanced Draw Call...
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, (GLsizei) clouds.size());

    // Restore
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
