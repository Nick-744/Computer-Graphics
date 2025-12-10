#include "snowfall.h"

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <common/shader.h>
#include <glm/gtc/matrix_transform.hpp>

static const float SNOWFLAKE_POINT_SIZE = 4.0f;

float rand01() { return static_cast<float> (rand()) / static_cast<float> (RAND_MAX); }

Snowfall::Snowfall(
    int   maxParticles,
    float minSpeed,
    float maxSpeed
)
    : maxParticles(maxParticles),
    minSpeed(minSpeed),
    maxSpeed(maxSpeed),
    active(false),
    firstFrame(true)
{
    particles.resize(maxParticles);

    // Initial values (will be overwritten by firstFrame scatter...)
    for (int i = 0; i < maxParticles; ++i)
    {
        particles[i].pos = vec3(0.0f);
        resetParticle(particles[i]);
    }

    // GL Setup
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    glBindVertexArray(0);

    shader     = loadShaders("shaders/Snowfall.vertexshader", "shaders/Snowfall.fragmentshader");
    viewMatrix = glGetUniformLocation(shader, "V");
    projMatrix = glGetUniformLocation(shader, "P");
    pointSize  = glGetUniformLocation(shader, "pointSize");
}

Snowfall::~Snowfall()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);
}

void Snowfall::resetParticle(Particle& p)
{
    p.life      = 2.0f + rand01() * 3.0f;
    float speed = minSpeed + rand01() * (maxSpeed - minSpeed);
    p.vel       = vec3(0.0f, -speed, 0.0f);
    p.wobblePhase = rand01() * 6.28f; // Random starting angle for wobble
}

void Snowfall::update(float deltaTime, const mat4& view, const mat4& proj)
{
    if (!active) return;

    mat4 invView    = inverse(view);
    float boxRadius = 70.0f; // VERY IMPORTANT FOR THE ILLUSION OF INFINITE SNOWFALL!
    
    // Define the "Snow Globe" bounds in View Space
    vec3 minBounds = vec3(-boxRadius, -boxRadius, -boxRadius);
    vec3 maxBounds = vec3( boxRadius,  boxRadius,  boxRadius);
    vec3 boxSize   = maxBounds - minBounds;

    // --- First Frame Scatter --- //
    // If this is the first update, scatter particles RANDOMLY inside the box!
    if (firstFrame)
    {
        for (int i = 0; i < maxParticles; ++i)
        {
            Particle& p = particles[i];

            // Random position in View Space within the Snow Globe
            float rx = rand01();
            float ry = rand01();
            float rz = rand01();

            vec3 randomViewPos = minBounds + vec3(rx, ry, rz) * boxSize;

            // Transform back to World Space
            p.pos = vec3(invView * vec4(randomViewPos, 1.0f));
        }
        firstFrame = false;
    }

    // --- Update Physics & Wrapping --- //
    for (int i = 0; i < maxParticles; ++i)
    {
        Particle& p = particles[i];

        // [Wobble Effect] Add sine wave motion to X and Z
        p.wobblePhase += deltaTime * 2.0f; // Speed of flutter
        vec3 flutter = vec3(
            cos(p.wobblePhase) * 1.3f, // X flutter strength
            0.0f,
            sin(p.wobblePhase) * 1.3f  // Z flutter strength
        );

        // Apply Gravity + Flutter
        p.pos += (p.vel + flutter) * deltaTime;

        // --- Wrap Logic (View Space) --- //
        // Convert particle to View Space to check against the "Snow Globe" bounds
        vec4 pView4 = view * vec4(p.pos, 1.0f);
        vec3 pView  = vec3(pView4);

        bool wrapped = false;
        vec3 wrapOffsetView(0.0f);

        // X Axis
        if      (pView.x < minBounds.x) { wrapOffsetView.x += boxSize.x; wrapped = true; }
        else if (pView.x > maxBounds.x) { wrapOffsetView.x -= boxSize.x; wrapped = true; }

        // Y Axis (Top/Bottom)
        if      (pView.y < minBounds.y) { wrapOffsetView.y += boxSize.y; wrapped = true; }
        else if (pView.y > maxBounds.y) { wrapOffsetView.y -= boxSize.y; wrapped = true; }

        // Z Axis (Near/Far)
        if      (pView.z < minBounds.z) { wrapOffsetView.z += boxSize.z; wrapped = true; }
        else if (pView.z > maxBounds.z) { wrapOffsetView.z -= boxSize.z; wrapped = true; }

        if (wrapped)
        {
            // Transform the wrapping offset from View Space -> World Space
            // Note: Direction vectors use w = 0.0f...
            vec3 wrapOffsetWorld = vec3(invView * vec4(wrapOffsetView, 0.0f));
            p.pos += wrapOffsetWorld;

            // Randomize X/Z slightly to break patterns
            if (wrapOffsetView.y != 0.0f)
            {
                p.pos.x += (rand01() - 0.5f) * 5.0f;
                p.pos.z += (rand01() - 0.5f) * 5.0f;
            }
        }

        // Reset particle data occasionally
        p.life -= deltaTime;
        if (p.life <= 0.0f) resetParticle(p);
    }
}

void Snowfall::draw(const mat4& view, const mat4& proj)
{
    if (!active) return;

    // Upload positions to GPU
    vector<vec3> positions;
    positions.reserve(maxParticles);
    for (int i = 0; i < maxParticles; ++i) positions.push_back(particles[i].pos);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(vec3), positions.data());

    // Render points
    glUseProgram(shader);
    glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projMatrix, 1, GL_FALSE, &proj[0][0]);
    glUniform1f(pointSize, SNOWFLAKE_POINT_SIZE);
    
    glEnable(GL_PROGRAM_POINT_SIZE); // Enable point size control in shader...

    // Keep depth test ON but disable depth write so snow doesn't punch holes
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glDepthMask(GL_FALSE);

    // Enable blending for soft snowflake edges
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, maxParticles);
    glBindVertexArray(0);

    // Restore state
    glDisable(GL_BLEND);
    glDepthMask(depthMask);
    glDisable(GL_PROGRAM_POINT_SIZE);
}
