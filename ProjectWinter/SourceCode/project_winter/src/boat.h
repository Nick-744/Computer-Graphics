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

    void steer(float deltaTime);
    float getRotationAngle() { return rotationAngle; }

    // Setters for positioning the boat in the world
    void setPosition(vec3  pos)   { position      = pos;   updateModelMatrix(); }
    void setRotation(float angle) { rotationAngle = angle; updateModelMatrix(); }

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

    void updateModelMatrix();
};

#endif
