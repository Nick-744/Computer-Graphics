#include "snowman.h"
#include <algorithm>

using namespace glm;

Snowman::Snowman(Drawable* sphereMesh, GLuint shaderProgram)
    : mesh(sphereMesh), programID(shaderProgram) {}

Snowman::~Snowman()
{
    // Clean up all allocated snowballs
    for (auto ball : balls) delete ball;
    balls.clear();
}

void Snowman::update(float deltaTime, vec3 playerPos, vec3 prevPlayerPos, vec3 cameraDir)
{
    // Update all balls
    for (auto& ball : balls)
    {
        ball->update(deltaTime, playerPos, prevPlayerPos);

		// AUTO-PICKUP LOGIC - 2nd snowball only!
        if (!heldBall && ball->isMaxSize && !ball->stacked && !ball->held)
        {
            if (ball->getMaxRadius() < 0.30f)
            {
                heldBall       = ball;
                heldBall->held = true;

                // If we were looking at it, clear the targeted pointer
                if (targetedBall == ball) targetedBall = nullptr;
            }
        }
    }

    // If holding a ball, position it in front of the camera!
    if (heldBall)
    {
        // Normalize camera direction and keep it horizontal
        vec3 forwardDir = normalize(cameraDir);
        forwardDir.y    = 0; // Flatten to horizontal plane

        // Fallback if camera is looking straight up/down!
        if (length(forwardDir) < 0.01f) forwardDir = vec3(0.0f, 0.0f, -1.0f);
        else                            forwardDir = normalize(forwardDir);

        // Position ball in front of player at chest height
        heldBall->position = playerPos + vec3(0.0f, 0.3f, 0.0f) + forwardDir * 1.2f;
    }
}

void Snowman::handleInteraction(vec3 playerPos, vec3 lookDir, float snowAmount)
{
    if (heldBall)
    {
        // Try to stack on another ball
        bool stacked = false;
        for (auto ball : balls)
        {
            if (ball == heldBall || !ball->active || ball->held || ball->stacked) continue;

            float stackDistance = (heldBall->radius + ball->radius) * 1.8f;

            if (distance(heldBall->position, ball->position) < stackDistance)
            {
                stackBall(heldBall, ball);
                heldBall = nullptr;
                stacked  = true;
                break;
            }
        }

        // If not stacked, throw the ball
        if (!stacked)
        {
			heldBall->held = false;
            heldBall       = nullptr;
        }
    }
    else
    {
        if (targetedBall) // Use the ball stored by the RayMarch!
        {
            heldBall       = targetedBall;
            heldBall->held = true;
            targetedBall   = nullptr; // Clear it since we are now holding it!
        }
        else if (snowAmount > 0.8f && balls.size() < MAX_BALLS)
        {
            // Create new snowball with appropriate max radius
            float maxRadius   = (balls.size() == 0) ? 0.36f : 0.18f;
            Snowball* newBall = new Snowball(mesh, programID, maxRadius);
            newBall->spawn(playerPos + lookDir * 1.5f);
            balls.push_back(newBall);
        }
    }
}

bool Snowman::isLookingAtAnyBall(glm::vec3 testPoint)
{
    for (auto& ball : balls)
    {
        if (!ball->active || ball->isMaxSize) continue;

        // Check if this specific ray point is inside the ball's selection volume...
        if (distance(testPoint, ball->position) < ball->radius * 1.8f)
        {
            targetedBall = ball; // Store the reference!
            return true;
        }
    }
    return false;
}

bool Snowman::checkCollision(glm::vec3 playerPos, float playerRadius)
{
    for (auto& ball : balls)
    {
        if (!ball->active || !ball->isMaxSize) continue; // Only collide with max-sized balls

        float distanceToPlayer  = distance(playerPos, ball->position);
        float collisionDistance = ball->radius * 1.6f + playerRadius; // Slight buffer

        if (distanceToPlayer < collisionDistance) return true;
    }
    return false;
}

void Snowman::stackBall(Snowball* top, Snowball* bottom)
{
    top->stacked   = true;
    top->held      = false;
    top->stackedOn = bottom; // Track the base ball
    top->position  = bottom->position + vec3(0, (bottom->radius + top->radius) * 1.4f, 0);
}

void Snowman::draw()
{
    for (auto& ball : balls) ball->draw();
}

void Snowman::drawOnlyObjects(GLuint shadowModelLocation)
{
    for (auto& ball : balls) ball->drawOnlyObjects(shadowModelLocation);
}
