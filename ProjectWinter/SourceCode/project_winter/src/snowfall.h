#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class Snowfall
{
public:
    Snowfall(
        int   maxParticles,
        float minSpeed,
        float maxSpeed
    );

    ~Snowfall();

    // Call every frame (fit the camera's view box!)
    void update(float deltaTime, const mat4& view, const mat4& proj, float windPower);

    // Call every frame!
    void draw(const mat4& view, const mat4& proj);

    void setActive(bool value) { active = value; }

private:
    struct Particle
    {
        vec3 pos;
        vec3 vel;
        float life; // seconds
        float wobblePhase;
    };

    // Helper to randomize a particle's properties
    // (velocity/life) without moving it...
    void resetParticle(Particle& p);

    vector<Particle> particles;

    int   maxParticles;
    float minSpeed;
    float maxSpeed;

    bool active;
    bool firstFrame;

    // GL
    GLuint vao;
    GLuint vbo;
    GLuint shader; // Particle Shader

    // uniforms
	GLint viewMatrix;
	GLint projMatrix;
	GLint pointSize;
};
