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

    void draw();
	void drawOnlyObjects(GLuint shadowModelLocation); // For depth pass!

    void setPosition(vec3 pos);

private:
    // Meshes
    Drawable* trunkMesh;
    Drawable* leavesMesh;

    // Textures
    GLuint trunkTexture;
    GLuint leavesTexture;

    // Position data
    vec3 position;
    mat4 modelMatrix;

    // Shader Uniform Locations (cached for performance)
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint useTransparentTex;
    GLuint diffuseColorSampler;

    void updateModelMatrix();
};

#endif
