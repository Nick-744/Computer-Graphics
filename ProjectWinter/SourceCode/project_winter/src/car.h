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
    void drawOnlyObjects(GLuint shadowModelLocation, bool isSnowBuffer = false); // For shadow pass

    bool update(float deltaTime);
    void toggleDoor();
    bool checkCollision(const vec3& position, float radius);

private:
    // --- MESHES --- //
    Drawable* bodyMesh;
    Drawable* bodyWindow;
    Drawable* doorMesh;
    Drawable* doorWindow;

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

    // Material Uniform Locations (Ka, Kd, Ks, Ns)
    GLuint KaLocation, KdLocation, KsLocation, NsLocation;

    // Door Animation
    float doorOpenAngle;
    float targetDoorAngle;
    vec3 doorHingePosition;
    mat4 getDoorMatrix();

    void updateModelMatrix();
};

#endif
