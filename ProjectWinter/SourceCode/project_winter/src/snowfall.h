// snowfall.h
#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Snowfall
{
public:
    Snowfall(
        int   maxParticles,
        float areaRadius,     // horizontal radius around camera
        float spawnHeight,    // how much above camera to spawn
        float minSpeed,
        float maxSpeed,
        GLuint snowTexture    // 0 if you want plain white discs
    );

    ~Snowfall();

    // Call every frame
    void update(float deltaTime, const glm::vec3& cameraPos);

    // Call every frame after normal 3D scene (with blending enabled here)
    void draw(const glm::mat4& view, const glm::mat4& proj);

    void setActive(bool value) { active = value; }
    bool isActive() const { return active; }

private:
    struct Particle
    {
        glm::vec3 pos;
        glm::vec3 vel;
        float     life; // seconds
    };

    void respawnParticle(Particle& p, const glm::vec3& cameraPos);

    std::vector<Particle> particles;

    int   maxParticles;
    float areaRadius;
    float spawnHeight;
    float minSpeed;
    float maxSpeed;

    bool   active;
    bool   useTexture;
    GLuint snowTexture;

    // GL
    GLuint vao;
    GLuint vbo;
    GLuint shader;   // particle shader

    // uniforms
    GLint  uView;
    GLint  uProj;
    GLint  uPointSize;
    GLint  uUseTexture;
    GLint  uTexture;

    // internal helper
    float rand01() const;
};
