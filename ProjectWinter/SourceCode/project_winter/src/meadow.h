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

    void draw(const mat4& view, const mat4& proj);

private:
    GLuint shaderProgram;

    // Uniform Locations
    GLint modelMatrixLocation;
    GLint useTextureLocation;
    GLint useTransparentTexLocation;
    GLint diffuseColorSampler;

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
