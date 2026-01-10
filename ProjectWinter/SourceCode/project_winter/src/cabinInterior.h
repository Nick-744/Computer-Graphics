#ifndef CABIN_INTERIOR_H
#define CABIN_INTERIOR_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glfw3.h>
#include <common/model.h>
#include <iostream>
#include <windows.h>

using namespace std;
using namespace glm;

class CabinInterior
{
public:
    CabinInterior(GLuint shaderProgram, GLFWwindow* window);
    ~CabinInterior();

    void draw(); // drawOnlyObjects is too much for the interior!
    void updateArcadeTexture();

    // Interaction logic
    void toggleInteraction();
    bool isInteracting()     const { return interacting; }
    mat4 getArcadeScreenViewMatrix();
    vec3 getArcadePosition() const { return arcadePosition; }

    bool checkCollision(const vec3& position, float radius);

private:
    GLFWwindow* window;
    bool interacting = false;
    HWND javaHwnd    = NULL; // Cache the Java window handle

    // --- Meshes & Textures --- //
    Drawable* arcadeMesh;
    GLuint arcadeMeshTexture;
    GLuint arcadeStaticText;

    Drawable* bedMesh;
    GLuint bedMeshTexture;
    


    // ===< Special screen - Render Java game >=== //
    Drawable* arcadeScreen;
    GLuint arcadeScreenTexture;
    unsigned char* specialScreenBuffer;
    void initArcade();
    void launchJavaGame();
    bool captureJavaWindow();



    // --- Matrices --- //
    // Arcade
    mat4 worldArcadeMatrix;
    vec3 arcadePosition;
    // Screen
    mat4 arcadeScreenModelMatrix;
    vec3 screenPosition;
    float screenRotY;
    float screenRotX;
    // Bed
    mat4 worldBedMatrix;

    // --- Uniform Locations --- //
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint diffuseColorSampler;
};

#endif
