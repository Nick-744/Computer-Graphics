#ifndef CABIN_H
#define CABIN_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <common/model.h>

using namespace glm;

class Cabin
{
public:
    Cabin(GLuint shaderProgram);
    ~Cabin();

    void draw();
    void drawOnlyObjects(GLuint shadowModelLocation); // For shadow pass

    // For the door animation!
    void update(float deltaTime);
    void toggleDoor();

private:
    // --- MESHS --- //
    Drawable* baseMesh;
    Drawable* doorMesh;
    Drawable* doorWinMesh;
    Drawable* floorMesh;
    Drawable* roofMesh;
    Drawable* supportStrMesh;
    Drawable* wallsFrBackMesh;
    Drawable* wallsSideMesh;
    Drawable* windowsMesh;

    // --- TEXTURES --- //
    GLuint baseTexture;
    GLuint doorTexture;
    GLuint doorWinTexture;
    GLuint floorTexture;
    GLuint roofTexture;
    GLuint supportStrTexture;
    GLuint wallsFrBackTexture;
    GLuint wallsSideTexture;
    GLuint windowsTexture;

    // --- TRANSFORMS --- //
    vec3 position;
    float rotationAngle;
    float scaleFactor;
    mat4 modelMatrix;

    // --- UNIFORM LOCATIONS --- //
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint diffuseColorSampler;

    // Material Uniform Locations (Ka, Kd, Ks, Ns)
    GLuint KaLocation, KdLocation, KsLocation, NsLocation;

    // Door
    float doorOpenAngle;    // Current rotation of the door
    vec3 doorHingePosition; // Offset from center to the door hinge
    float targetDoorAngle;  // For animation!
    mat4 getDoorMatrix();

    void updateModelMatrix();
};

#endif
