#ifndef FOREST_H
#define FOREST_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <common/model.h>

using namespace glm;

class Forest
{
public:
    Forest(GLuint shaderProgram);
    ~Forest();

    void draw(int windPower);
    void drawOnlyObjects(GLuint shadowModelLocation, int windPower); // For depth pass!

    void setPosition(vec3 pos);
    void setRotation(float angle);

    bool checkCollision(const vec3& position, float radius);

private:
    // Meshes
    Drawable* trunkMesh;
    Drawable* leavesMesh;

    // Textures
    GLuint trunkTexture;
    GLuint leavesTexture;

    // Position data
    vec3 position;
	float rotationAngle;
	float scaleFactor;
    mat4 modelMatrix;

    // Shader Uniform Locations (cached for performance)
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint useTransparentTex;
    GLuint diffuseColorSampler;
    GLuint windPowerLocation;

    void updateModelMatrix();
};

#endif
