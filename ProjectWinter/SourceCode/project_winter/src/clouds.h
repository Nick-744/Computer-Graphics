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

    void draw(const mat4& view, const mat4& proj, float time, const vec3& lightPos);

private:
    struct CloudInstance
    {
        vec3 position;
        vec3 scale;
    };

    GLuint shaderProgram;
    GLuint vao, vbo, ebo;
    GLsizei indexCount;

    GLuint cloudBaseTex;
    GLuint cloudDetailTex;

    // Uniform Locations
    GLint vpLocation;
    GLint modelLocation;
    GLint cameraRightLocation;
    GLint cameraUpLocation;
    GLint timeLocation;
    GLint lightPosLocation; // Volumetric lighting
    GLint cloudBaseLocation;
    GLint cloudDetailLocation;

    std::vector<CloudInstance> clouds;

    void buildCloudMesh(int numParticles, float radius);
};

#endif
