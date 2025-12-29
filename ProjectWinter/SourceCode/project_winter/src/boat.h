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

    void draw();
    void drawOnlyObjects(GLuint shadowModelLocation); // For depth pass

    mat4 getViewMatrix();

    void update(float deltaTime);
    void steer(float deltaTime);

    // Setters for positioning the boat in the world
    void setPosition(vec3  pos)   { position      = pos;   updateModelMatrix(); }
    void setRotation(float angle) { rotationAngle = angle; updateModelMatrix(); }

    vec3  getWorldPosition() { return position;      }
    float getRotationAngle() { return rotationAngle; }
    


    // ===< Boat porting >=== //
    const vec3  INITIAL_POSITION = vec3(-58.0f, 58.065f, 8.0f);
    const float INITIAL_ROTATION = 1.6f;

    void setToPort1()
    {
        setPosition(INITIAL_POSITION);
        setRotation(INITIAL_ROTATION);
    }

private:
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

    void updateModelMatrix();
};

#endif
