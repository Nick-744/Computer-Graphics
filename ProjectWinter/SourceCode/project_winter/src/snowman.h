#ifndef SNOWMAN_H
#define SNOWMAN_H

#include <vector>
#include <glm/glm.hpp>
#include "snowball.h"

class Snowman
{
public:
    Snowman(Drawable* sphereMesh, GLuint shaderProgram, glm::vec3 lookAtTarget);
    ~Snowman();

    void update(float deltaTime, glm::vec3 playerPos, glm::vec3 prevPlayerPos, glm::vec3 cameraDir, const glm::vec4 planes[6]);
    void draw();
    void drawOnlyObjects(GLuint shadowModelLocation);

    void handleInteraction(glm::vec3 playerPos, glm::vec3 lookDir, float snowAmount);
    bool isLookingAtAnyBall(glm::vec3 testPoint);

    bool checkCollision(glm::vec3 playerPos, float playerRadius);

    bool active = true; // System-wide active flag

    // The snowball the player is currently looking at
    Snowball* targetedBall = nullptr;
    void clearTarget() { targetedBall = nullptr; }

    bool hasTransformed = false; // Tracks the mesh swap

private:
    std::vector<Snowball*> balls;
    Snowball* heldBall = nullptr;
    GLuint programID;
    Drawable* mesh; // The sphere mesh for snowballs

    void stackBall(Snowball* topBall, Snowball* bottomBall);

    const int MAX_BALLS = 2; // Only 2 balls allowed!



    // ===< Creepy snowman mesh >=== //
    Drawable* staticModel;
    GLuint staticTexture;
    glm::mat4 staticModelMatrix;
    glm::vec3 lookAtTarget; // Store the point to face

    // --- UNIFORM LOCATIONS --- //
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint diffuseColorSampler;
};

#endif
