#ifndef MARINA_H
#define MARINA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <common/model.h>

using namespace glm;

class Marina
{
public:
    Marina(GLuint shaderProgram, vec3 pos, float rotation);
    ~Marina();

    void draw();
    void drawOnlyObjects(GLuint shadowModelLocation);

    bool checkCollision(const vec3& position, float radius);

private:
    // Meshes
    Drawable* logsMesh;
    Drawable* planksMesh;

    // Textures
    GLuint logsTexture;
    GLuint planksTexture;

    // Transforms
    mat4 modelMatrix;

    // Shader Uniforms
    GLuint modelMatrixLocation;
    GLuint useTextureLocation;
    GLuint diffuseColorSampler;
};

#endif
