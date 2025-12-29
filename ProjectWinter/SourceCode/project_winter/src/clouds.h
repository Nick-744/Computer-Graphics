#ifndef CLOUDS_H
#define CLOUDS_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

class CloudRenderer
{
public:
    CloudRenderer();
    ~CloudRenderer();

    void draw(const mat4& view, const mat4& proj, float time, vec3 fogColor, float fogDensity);

private:
    struct CloudInstance
    {
        vec3 startPosition;
        vec3 scale;
    };

    // Instancing...
    GLuint instanceVBO; // Buffer for model matrices
    std::vector<mat4> instanceMatrices; // Fixed-size buffer for updates

    GLuint shaderProgram;
    GLuint vao, vbo, ebo;
    GLsizei indexCount;

    GLuint cloudBaseTexture;
    GLuint cloudDetailTexture;

    // Uniform Locations
    GLint vpLocation;
    GLint cameraRightLocation; // NEEDED: To make clouds face camera
    GLint cameraUpLocation;    // NEEDED: To make clouds face camera
    GLint cloudBaseLocation;
    GLint cloudDetailLocation;

    // Fog
    GLint fogDensityLocation;
    GLint fogColorLocation;

    std::vector<CloudInstance> clouds;

    void buildCloudMesh(int numParticles, float radius);
};

#endif
