#ifndef SNOWMAN_H
#define SNOWMAN_H

#include <vector>
#include "snowball.h"

class Snowman
{
public:
    Snowman(Drawable* sphereMesh);
    ~Snowman();

    void update(float deltaTime, glm::vec3 playerPos, glm::vec3 prevPlayerPos, glm::vec3 cameraDir);
    void draw(GLuint modelLoc, GLuint texLoc, GLuint Ka, GLuint Kd, GLuint Ks, GLuint Ns);
    void drawOnlyObjects(GLuint modelLoc);

    void handleInteraction(glm::vec3 playerPos, glm::vec3 lookDir, float snowAmount);
    bool isLookingAtAnyBall(glm::vec3 testPoint);

    bool checkCollision(glm::vec3 playerPos, float playerRadius);

    bool active = true; // System-wide active flag

private:
    std::vector<Snowball*> balls;
    Snowball* heldBall = nullptr;
    Drawable* mesh;

    void stackBall(Snowball* topBall, Snowball* bottomBall);
    Snowball* findNearestBall(glm::vec3 position, float maxDistance);

    const int MAX_BALLS = 2; // Only 2 balls allowed!
};

#endif
