#ifndef SNOWBALL_H
#define SNOWBALL_H

#include <glm/glm.hpp>
#include <common/model.h>

class Snowball
{
public:
    Snowball(Drawable* sphereMesh, GLuint shaderProgram, float maxRadiusOverride = -1.0f);
    ~Snowball();

    // Updates physics, collision with player, and growth
    void update(float deltaTime, glm::vec3 currentPlayerPos, glm::vec3 prevPlayerPos);
    void spawn(glm::vec3 pos);

    glm::mat4 getModelMatrix();

    void draw();
    void drawOnlyObjects(GLuint shadowModelLocation); // Shadow pass

    glm::vec3 position;
    float radius;
    bool active    = false;
    bool held      = false;
    bool stacked   = false;
    bool isMaxSize = false;
    float getMaxRadius() { return MAX_RADIUS; }

    // Track which ball this is stacked on
    Snowball* stackedOn = nullptr;

private:
    Drawable* mesh;

    glm::vec3 velocity;
    float mass;

    // Constants
    float MAX_RADIUS;
    const float GROWTH_RATE   = 0.03f;
    const float FRICTION      = 0.95f;
    const float PLAYER_RADIUS = 1.0f;

    // --- UNIFORM LOCATIONS --- //
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;

    // Material Uniform Locations (Ka, Kd, Ks, Ns)
    GLuint KaLocation, KdLocation, KsLocation, NsLocation;
};

#endif
