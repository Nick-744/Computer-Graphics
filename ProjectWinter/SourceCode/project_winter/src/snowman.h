#ifndef SNOWMAN_H
#define SNOWMAN_H

#include <glm/glm.hpp>
#include <common/model.h>

class Snowman
{
public:
    Snowman(Drawable* sphereMesh);
    ~Snowman();

    glm::vec3 position;
    float radius;
    bool active = false;
    bool held   = false;

    // Updates physics, collision with player, and growth
    void update(float deltaTime, glm::vec3 currentPlayerPos, glm::vec3 prevPlayerPos);
    void spawn(glm::vec3 pos);

    void draw(
        GLuint modelMatrixLocation,
        GLuint useTextureLocation,
        GLuint KaLoc, GLuint KdLoc, GLuint KsLoc, GLuint NsLoc
    );

    // Depth/Shadow pass
    void drawOnlyObjects(GLuint modelMatrixLocation);

    glm::vec3 getPosition() { return position; }
    float getRadius()       { return radius;   }

private:
    Drawable* mesh;

    glm::vec3 velocity;
    float mass;

    // Constants
    const float MAX_RADIUS    = 0.36f;
    const float GROWTH_RATE   = 0.03f;
    const float FRICTION      = 0.9f;
    const float PLAYER_RADIUS = 1.2f;
};

#endif
