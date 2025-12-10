#include "lakeReflection.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>

LakeReflection::LakeReflection(int bufferSize)
    : textureBufferSize(bufferSize),
    reflectionFBO(0),
    reflectionTexture(0),
    reflectionDepthBuffer(0) {}

LakeReflection::~LakeReflection()
{
    glDeleteFramebuffers(1, &reflectionFBO);
    glDeleteTextures(1, &reflectionTexture);
    glDeleteRenderbuffers(1, &reflectionDepthBuffer);

	reflectionFBO         = 0;
	reflectionTexture     = 0;
	reflectionDepthBuffer = 0;
}

void LakeReflection::initialize()
{
    // Generate framebuffer for reflection
    glGenFramebuffers(1, &reflectionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);

    // Create reflection texture (color attachment)
    glGenTextures(1, &reflectionTexture);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
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
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0);

    // Create depth buffer for reflection
    glGenRenderbuffers(1, &reflectionDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, reflectionDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureBufferSize, textureBufferSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflectionDepthBuffer);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw runtime_error("Reflection framebuffer not complete!");

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LakeReflection::beginReflectionPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
    glViewport(0, 0, textureBufferSize, textureBufferSize);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LakeReflection::endReflectionPass() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

mat4 LakeReflection::getMirroredViewMatrix(const mat4& viewMatrix, float waterHeight)
{
    // Create reflection matrix for mirroring across XZ plane (Y = waterHeight)
    mat4 reflectionMatrix  = mat4(1.0f);
    reflectionMatrix[1][1] = -1.0f; // Flip Y axis
    reflectionMatrix[3][1] = 2.0f * waterHeight; // Translate to water height
    
    // Apply reflection transformation to the inverse view matrix!
    // This mirrors the camera position and orientation correctly...
    mat4 inverseView         = inverse(viewMatrix);
    mat4 mirroredInverseView = reflectionMatrix * inverseView;
    
    return inverse(mirroredInverseView);
}

// Clipping plane - Above water surface (for reflection, clip below water)!
vec4 LakeReflection::getClipPlane(float waterHeight) { return vec4(0.0f, 1.0f, 0.0f, -waterHeight); }
// Plane equation: 0x + 1y + 0z - waterHeight = 0
