#ifndef SNOWBALL_H
#define SNOWBALL_H

#include <glm/glm.hpp>
#include <common/model.h>

class Snowball
{
public:
    Snowball(Drawable* sphereMesh, float maxRadiusOverride = -1.0f);
    ~Snowball();

    // Updates physics, collision with player, and growth
    void update(float deltaTime, glm::vec3 currentPlayerPos, glm::vec3 prevPlayerPos);
    void spawn(glm::vec3 pos);

    glm::mat4 getModelMatrix();

    void draw(
        GLuint modelMatrixLocation,
        GLuint useTextureLocation,
        GLuint KaLoc, GLuint KdLoc, GLuint KsLoc, GLuint NsLoc
    );

    // Depth/Shadow pass
    void drawOnlyObjects(GLuint modelMatrixLocation);

    glm::vec3 position;
    float radius;
    bool active    = false;
    bool held      = false;
    bool stacked   = false;
    bool isMaxSize = false;

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
    const float PLAYER_RADIUS = 0.3f;
};

#endif
