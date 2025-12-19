#ifndef MEADOW_H
#define MEADOW_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

struct GrassVertex
{
    vec3 position;
    vec2 uv;
    vec3 normal;
};

class Meadow
{
public:
    Meadow(GLuint shaderProgram);
    ~Meadow();

    void draw(int windPower);
    void drawOnlyObjects(GLuint shadowModelLocation, int windPower);
    // Passing the enableWindLocation is better than passing the entire DEPTH shader program...

private:
    GLuint shaderProgram;

    // Uniform Locations
    GLint modelMatrixLocation;
    GLint useTransparentTexLocation;
    GLint diffuseColorSampler;
    GLint windPowerLocation;

    // Grass Data
    GLuint grassTexture;
    GLuint vao;
    GLuint vbo;
    GLsizei vertexCount;

    // Tree Data
    GLuint treeTexture;
    GLuint treeVAO;
    GLuint treeVBO;
    GLsizei treeVertexCount;

    // Helper to load baked positions
    void loadAndGenerateMesh(const char* filepath);
    void loadAndGenerateTrees(const char* filepath);
};

#endif
