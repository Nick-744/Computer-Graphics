#ifndef BOAT_H
#define BOAT_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glfw3.h>
#include <common/model.h>

using namespace glm;

class Boat
{
public:
    GLFWwindow* window; // To capture input - steer...

    Boat(GLuint shaderProgram, GLFWwindow* window);
    ~Boat();

    mat4 getViewMatrix();

    void draw();
    void drawOnlyObjects(GLuint shadowModelLocation); // For depth pass

    void update(float deltaTime);
    void steer(float deltaTime);

    // Setters for positioning the boat in the world
    void setPosition(vec3  pos)   { position      = pos;   updateModelMatrix(); }
    void setRotation(float angle) { rotationAngle = angle; updateModelMatrix(); }

    vec3  getWorldPosition() { return position;      }
    float getRotationAngle() { return rotationAngle; }
    
    void invertOnBoat() { onBoat = !onBoat; }
    bool isOnBoat()     { return onBoat;    }



    // ===< Boat porting >=== //
    const vec3  INITIAL_POSITION = vec3(-58.0f, 58.065f, 8.0f);
    const float INITIAL_ROTATION = 1.6f;

    void setToPort1()
    {
        setPosition(INITIAL_POSITION);
        setRotation(INITIAL_ROTATION);
    }

private:
    bool onBoat = false;

    // --- Meshes --- //
    Drawable* boatMesh;
    Drawable* paddleL;
    Drawable* paddleR;

    // --- Textures --- //
    GLuint boatTexture;

    // --- Transforms --- //
    vec3  position;
    float rotationAngle;
    float scaleFactor;
    mat4  modelMatrix;

    // --- Uniform Locations --- //
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint diffuseColorSampler;
    
    // --- Animation Timers --- //
    float totalTime;
    float leftPaddleTimer;
    float rightPaddleTimer;
    // Physics/Inertia state
    float currentSpeed     = 0.0f;
    float currentTurnSpeed = 0.0f;
    // Constants
    const float MAX_SPEED      = 4.0f;
    const float ACCELERATION   = 3.0f;
    const float FRICTION       = 1.5f;
    const float MAX_TURN_SPEED = 1.0f;
    const float TURN_ACCEL     = 2.0f;
    const float TURN_FRICTION  = 2.5f;

    void updateModelMatrix();
    mat4 getPaddleTransform(float timer, bool isLeft);
};

#endif
