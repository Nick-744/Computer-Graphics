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

    void draw(const mat4& view, const mat4& proj, float time);

private:
    struct CloudInstance
    {
        vec3 startPosition;
        vec3 scale;
    };

    GLuint shaderProgram;
    GLuint vao, vbo, ebo;
    GLsizei indexCount;

    GLuint cloudBaseTexture;
    GLuint cloudDetailTexture;

    // Uniform Locations
    GLint vpLocation;
    GLint modelLocation;
    GLint cameraRightLocation; // NEEDED: To make clouds face camera
    GLint cameraUpLocation;    // NEEDED: To make clouds face camera
    GLint timeLocation;
    GLint cloudBaseLocation;
    GLint cloudDetailLocation;

    std::vector<CloudInstance> clouds;

    void buildCloudMesh(int numParticles, float radius);
};

#endif
