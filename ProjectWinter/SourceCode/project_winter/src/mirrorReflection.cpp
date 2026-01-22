#include "MirrorReflection.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>

MirrorReflection::MirrorReflection(int bufferSize)
    : textureBufferSize(bufferSize),
    mirrorFBO(0),
    mirrorTexture(0),
    mirrorDepthBuffer(0) {}

MirrorReflection::~MirrorReflection()
{
    glDeleteFramebuffers(1, &mirrorFBO);
    glDeleteTextures(1, &mirrorTexture);
    glDeleteRenderbuffers(1, &mirrorDepthBuffer);

    mirrorFBO         = 0;
    mirrorTexture     = 0;
    mirrorDepthBuffer = 0;
}

void MirrorReflection::initialize()
{
    // Generate framebuffer for reflection
    glGenFramebuffers(1, &mirrorFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mirrorFBO);

    // Create reflection texture (color attachment)
    glGenTextures(1, &mirrorTexture);
    glBindTexture(GL_TEXTURE_2D, mirrorTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        textureBufferSize,
        textureBufferSize,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        NULL
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    // Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture, 0);

    // Create depth buffer for reflection
    glGenRenderbuffers(1, &mirrorDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mirrorDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureBufferSize, textureBufferSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mirrorDepthBuffer);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw runtime_error("Reflection framebuffer not complete!");

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MirrorReflection::beginReflectionPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mirrorFBO);
    glViewport(0, 0, textureBufferSize, textureBufferSize);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MirrorReflection::endReflectionPass() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

mat4 MirrorReflection::getReflectionMatrix(vec3 mirrorPos, vec3 mirrorNormal)
{
    mirrorNormal = normalize(mirrorNormal);
    float d      = -dot(mirrorNormal, mirrorPos);

    // Create a general reflection matrix!
    // This matrix reflects a point across the plane: ax + by + cz + d = 0
    mat4 R  = mat4();
    R[0][0] = 1 - 2 * mirrorNormal.x * mirrorNormal.x;
    R[1][0] =    -2 * mirrorNormal.x * mirrorNormal.y;
    R[2][0] =    -2 * mirrorNormal.x * mirrorNormal.z;
    R[3][0] =    -2 * mirrorNormal.x * d;

    R[0][1] =    -2 * mirrorNormal.y * mirrorNormal.x;
    R[1][1] = 1 - 2 * mirrorNormal.y * mirrorNormal.y;
    R[2][1] =    -2 * mirrorNormal.y * mirrorNormal.z;
    R[3][1] =    -2 * mirrorNormal.y * d;

    R[0][2] =    -2 * mirrorNormal.z * mirrorNormal.x;
    R[1][2] =    -2 * mirrorNormal.z * mirrorNormal.y;
    R[2][2] = 1 - 2 * mirrorNormal.z * mirrorNormal.z;
    R[3][2] =    -2 * mirrorNormal.z * d;

    return R;
}

mat4 MirrorReflection::getMirroredViewMatrix(const mat4& viewMatrix, vec3 mirrorPos, vec3 mirrorNormal)
{
    mat4 reflectionMatrix = getReflectionMatrix(mirrorPos, mirrorNormal);

    // Apply reflection transformation to the inverse view matrix!
    // This mirrors the camera position and orientation correctly...
    mat4 inverseView         = inverse(viewMatrix);
    mat4 mirroredInverseView = reflectionMatrix * inverseView;

    return inverse(mirroredInverseView); // Inverse View -> Reflect -> Inverse back to View
}

// Clipping plane - Above water surface (for reflection, clip below water)!
vec4 MirrorReflection::getClipPlane(vec3 mirrorPos, vec3 mirrorNormal)
{
    // The plane equation is ax + by + cz + d = 0
    // (a,b,c) is the normal, d is the distance from origin
    float d = -dot(normalize(mirrorNormal), mirrorPos);

    return vec4(mirrorNormal.x, mirrorNormal.y, mirrorNormal.z, d);
}
