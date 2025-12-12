#ifndef LAKE_REFLECTION_H
#define LAKE_REFLECTION_H

#include <GL/glew.h>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

class LakeReflection
{
public:
    LakeReflection(int bufferSize);
    ~LakeReflection();

    void initialize(); // Initialize framebuffer and texture for reflection

    void beginReflectionPass(); // Begin rendering to reflection texture
    void endReflectionPass();   // End reflection pass and restore default framebuffer

    // Get reflection texture for binding in main shader
    GLuint getReflectionTexture() const { return reflectionTexture; }

    // Calculate mirrored view matrix for reflection camera
    mat4 getMirroredViewMatrix(const mat4& viewMatrix, float waterHeight);

    // Get clipping plane for reflection rendering (to avoid rendering below water...)
    vec4 getClipPlane(float waterHeight);

private:
    GLuint reflectionFBO;
    GLuint reflectionTexture;
    GLuint reflectionDepthBuffer;

	int textureBufferSize;
};

#endif
