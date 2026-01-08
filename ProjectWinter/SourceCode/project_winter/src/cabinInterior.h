#ifndef CABIN_INTERIOR_H
#define CABIN_INTERIOR_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <common/model.h>
#include <iostream>
#include <windows.h>

using namespace std;
using namespace glm;

class CabinInterior
{
public:
    CabinInterior(GLuint shaderProgram);
    ~CabinInterior();
    
    void draw(); // drawOnlyObjects is too much for the interior!
    void updateArcadeTexture();

    bool checkCollision(const vec3& position, float radius);

private:
    // --- Meshes & Textures --- //
    Drawable* arcadeMesh;
    GLuint arcadeMeshTexture;
    GLuint arcadeErrorText;
    


    // ===< Special screen - Render Java game >=== //
    Drawable* arcadeScreen;
    GLuint arcadeScreenTexture;
    void initArcade();
    bool captureJavaWindow();



    // --- Matrices --- //
    mat4 globalModelMatrix; // Parent
    
    // Children transformation data
    vec3 arcadePosition;
    float arcadeRotationAngle;
    mat4 arcadeLocalMatrix;

    // --- Uniform Locations --- //
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint diffuseColorSampler;

    void updateMatrices();
};

#endif
