#include "clouds.h"

#include <common/shader.h>
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstdlib> // Random

using namespace std;
using namespace glm;

struct CloudVertex
{
    vec3 centerOffset; // The center of the specific particle relative to cloud center
    vec2 uv;           // UV coordinates (also used to determine corner direction)
};

// Helper for random float between -1 and 1...
float randomFloat() { return (float)rand() / (float)RAND_MAX * 2.0f - 1.0f; }

CloudRenderer::CloudRenderer()
{
    shaderProgram = loadShaders("shaders/clouds.vertexshader", "shaders/clouds.fragmentshader");

    // Get Uniform Locations
    vpLocation          = glGetUniformLocation(shaderProgram, "VP");
    modelLocation       = glGetUniformLocation(shaderProgram, "M");
    cameraRightLocation = glGetUniformLocation(shaderProgram, "CameraRight");
    cameraUpLocation    = glGetUniformLocation(shaderProgram, "CameraUp");
    timeLocation        = glGetUniformLocation(shaderProgram, "time");
    lightPosLocation    = glGetUniformLocation(shaderProgram, "lightPos");
    cloudBaseLocation   = glGetUniformLocation(shaderProgram, "cloudBase");
    cloudDetailLocation = glGetUniformLocation(shaderProgram, "cloudDetail");

    // Build a cluster of 50 billboards (puffs)
    buildCloudMesh(50, 15.0f);

    cloudBaseTex   = loadBMP("assets/clouds_textures/cloud_base.bmp");
    cloudDetailTex = loadBMP("assets/clouds_textures/cloud_detail.bmp");

    // Better texture wrapping for noise
    glBindTexture(GL_TEXTURE_2D, cloudBaseTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glBindTexture(GL_TEXTURE_2D, cloudDetailTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Initial Positions
    clouds.push_back({ vec3(-80.0f, 100.0f, -40.0f), vec3(1.5f, 1.0f, 1.5f) });
    clouds.push_back({ vec3(200.0f, 110.0f,   0.0f), vec3(2.0f, 1.2f, 2.0f) });
    clouds.push_back({ vec3( 90.0f, 150.0f,  40.0f), vec3(1.2f, 0.9f, 1.2f) });
    clouds.push_back({ vec3(-50.0f, 130.0f,  50.0f), vec3(1.8f, 1.1f, 1.8f) });
}

void CloudRenderer::buildCloudMesh(int numParticles, float radius)
{
    vector<CloudVertex> vertices;
    vector<GLuint> indices;

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
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 2);
        indices.push_back(base + 3);
        indices.push_back(base + 0);
    }

    indexCount = (GLsizei)indices.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CloudVertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

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

    glDeleteTextures(1, &cloudBaseTex);
    glDeleteTextures(1, &cloudDetailTex);
}

void CloudRenderer::draw(const mat4& view, const mat4& proj, float time, const vec3& lightPos)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth writing so inner particles don't occlude outer ones weirdly
    // But keep depth testing ON so they hide behind buildings/mountains
    glDepthMask(GL_FALSE);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudBaseTex);
    glUniform1i(cloudBaseLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudDetailTex);
    glUniform1i(cloudDetailLocation, 1);

    glUniform1f(timeLocation, time);
    glUniform3fv(lightPosLocation, 1, &lightPos[0]);

    // Calculate Camera Right and Up vectors from View Matrix
    // View matrix columns 0, 1, 2 are Right, Up, Forward in View Space.
    // We need them in World Space to orient the billboards.
    // The inverse (or transpose for rotation) of View gives World vectors.

    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp    = vec3(view[0][1], view[1][1], view[2][1]);

    glUniform3fv(cameraRightLocation, 1, &cameraRight[0]);
    glUniform3fv(cameraUpLocation,    1, &cameraUp[0]);

    mat4 vp = proj * view;
    glUniformMatrix4fv(vpLocation, 1, GL_FALSE, &vp[0][0]);

    for (const CloudInstance& c : clouds)
    {
        mat4 model = translate(mat4(1.0f), c.position) * scale(mat4(1.0f), c.scale);

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &model[0][0]);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    // Restore depth writing
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
