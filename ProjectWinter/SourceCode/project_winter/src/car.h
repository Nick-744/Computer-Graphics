#ifndef CAR_H
#define CAR_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <common/model.h>

using namespace glm;

class Car
{
public:
    Car(GLuint shaderProgram);
    ~Car();

    void draw();
    void drawOnlyObjects(GLuint shadowModelLocation); // For shadow pass

    void update(float deltaTime);
    bool toggleDoor();
    bool checkCollision(const vec3& position, float radius);

private:
    // --- MESHES --- //
    Drawable* bodyMesh;
    Drawable* doorMesh;

    // --- TEXTURES --- //
    GLuint carTexture;

    // --- TRANSFORMS --- //
    vec3 position;
    float rotationAngleY;
    float rotationAngleX;
	float rotationAngleZ;
    float scaleFactor;
    mat4 modelMatrix;

    // --- UNIFORM LOCATIONS --- //
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint diffuseColorSampler;

    // Door Animation
    float doorOpenAngle;
    float targetDoorAngle;
    vec3 doorHingePosition;
    mat4 getDoorMatrix();

    void updateModelMatrix();
};

#endif
