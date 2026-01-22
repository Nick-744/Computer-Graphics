#ifndef MIRROR_REFLECTION_H
#define MIRROR_REFLECTION_H

#include <GL/glew.h>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

class MirrorReflection
{
public:
    MirrorReflection(int bufferSize);
    ~MirrorReflection();

    void initialize(); // Initialize framebuffer and texture for reflection

    void beginReflectionPass(); // Begin rendering to reflection texture
    void endReflectionPass();   // End reflection pass and restore default framebuffer

    // Get reflection texture for binding in main shader
    GLuint getMirrorTexture() const { return mirrorTexture; }

    // Calculate mirrored view matrix for reflection camera
    mat4 getReflectionMatrix(vec3 mirrorPos, vec3 mirrorNormal);
    mat4 getMirroredViewMatrix(const mat4& viewMatrix, vec3 mirrorPos, vec3 mirrorNormal);

    // Get clipping plane for reflection rendering (to avoid rendering behind the mirror...)
    vec4 getClipPlane(vec3 mirrorPos, vec3 mirrorNormal);

private:
    GLuint mirrorFBO;
    GLuint mirrorTexture;
    GLuint mirrorDepthBuffer;

    int textureBufferSize;
};

#endif
