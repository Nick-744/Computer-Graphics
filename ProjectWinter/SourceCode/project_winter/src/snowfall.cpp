// snowfall.cpp
#include "snowfall.h"

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <common/shader.h>  // your existing shader loader
#include <glm/gtc/matrix_transform.hpp>

static const float DEFAULT_POINT_SIZE = 10.0f; // screen-space pixels

Snowfall::Snowfall(
    int   maxParticles_,
    float areaRadius_,
    float spawnHeight_,
    float minSpeed_,
    float maxSpeed_,
    GLuint snowTexture_
)
    : maxParticles(maxParticles_),
    areaRadius(areaRadius_),
    spawnHeight(spawnHeight_),
    minSpeed(minSpeed_),
    maxSpeed(maxSpeed_),
    active(false),
    snowTexture(snowTexture_)
{
    if (snowTexture != 0) useTexture = true;
    else                  useTexture = false;

    // Seed RNG once
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    particles.resize(maxParticles);

    // Create VAO / VBO for point positions
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        maxParticles * sizeof(glm::vec3),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );

    glBindVertexArray(0);

    // Load particle shader
    shader = loadShaders(
        "shaders/Snowfall.vertexshader",
        "shaders/Snowfall.fragmentshader"
    );

    uView = glGetUniformLocation(shader, "V");
    uProj = glGetUniformLocation(shader, "P");
    uPointSize = glGetUniformLocation(shader, "pointSize");
    uUseTexture = glGetUniformLocation(shader, "useTexture");
    uTexture = glGetUniformLocation(shader, "snowTexture");

    // Initialize all particles somewhere (will be corrected on first update)
    for (int i = 0; i < maxParticles; ++i)
        respawnParticle(particles[i], glm::vec3(0.0f));
}

Snowfall::~Snowfall()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);
}

float Snowfall::rand01() const
{
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

void Snowfall::respawnParticle(Particle& p, const glm::vec3& cameraPos)
{
    // Horizontal spawn area around camera
    float xOffset = (rand01() * 2.0f - 1.0f) * areaRadius;
    float zOffset = (rand01() * 2.0f - 1.0f) * areaRadius;

    p.pos = glm::vec3(
        cameraPos.x + xOffset,
        cameraPos.y + spawnHeight,
        cameraPos.z + zOffset
    );

    float speed = minSpeed + rand01() * (maxSpeed - minSpeed);
    p.vel = glm::vec3(0.0f, -speed, 0.0f);

    // 2–5 seconds lifetime
    p.life = 2.0f + rand01() * 3.0f;
}

void Snowfall::update(float deltaTime, const glm::vec3& cameraPos)
{
    if (!active) {
        // Optional: still keep particles updated so when you enable again
        // they continue from where they were. Or just early return.
        return;
    }

    for (int i = 0; i < maxParticles; ++i)
    {
        Particle& p = particles[i];
        p.life -= deltaTime;

        if (p.life <= 0.0f || p.pos.y < cameraPos.y - spawnHeight)
        {
            respawnParticle(p, cameraPos);
        }
        else
        {
            p.pos += p.vel * deltaTime;
        }
    }
}

void Snowfall::draw(const glm::mat4& view, const glm::mat4& proj)
{
    if (!active) return;

    // Upload positions to GPU
    std::vector<glm::vec3> positions;
    positions.reserve(maxParticles);
    for (int i = 0; i < maxParticles; ++i)
        positions.push_back(particles[i].pos);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        positions.size() * sizeof(glm::vec3),
        positions.data()
    );

    // Render points
    glUseProgram(shader);

    glUniformMatrix4fv(uView, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(uProj, 1, GL_FALSE, &proj[0][0]);
    glUniform1f(uPointSize, DEFAULT_POINT_SIZE);
    glUniform1i(uUseTexture, useTexture ? 1 : 0);

    if (useTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, snowTexture);
        glUniform1i(uTexture, 0);
    }

    // Enable blending for soft particles
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    if (!blendEnabled) glEnable(GL_BLEND);

    GLint oldSrc, oldDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &oldSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &oldDst);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Keep depth test ON but disable depth write so snow doesn’t punch holes
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glDepthMask(GL_FALSE);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, maxParticles);
    glBindVertexArray(0);

    // Restore state
    glDepthMask(depthMask);
    glBlendFunc(oldSrc, oldDst);
    if (!blendEnabled) glDisable(GL_BLEND);
}
