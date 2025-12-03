#include "meadow.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

// Random helper for size variation
float rnd() { return (float)rand() / (float)RAND_MAX; }

Meadow::Meadow(GLuint shaderProgram)
{
    this->shaderProgram = shaderProgram;

    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation = glGetUniformLocation(shaderProgram, "useTexture");
    useTransparentTexLocation = glGetUniformLocation(shaderProgram, "useTransparentTex");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");

    grassTexture = loadBMP("assets/vegetation/grass.bmp");

    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // LOAD FROM FILE
    loadAndGenerateMesh("assets/grass_positions.txt");
}

void Meadow::loadAndGenerateMesh(const char* filepath)
{
    vector<GrassVertex> vertices;
    ifstream file(filepath);

    if (!file.is_open()) {
        printf("ERROR: Could not open grass positions: %s\n", filepath);
        printf("Did you run the Python script bake_grass.py?\n");
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        float x, y, z;
        if (!(ss >> x >> y >> z)) continue;

        vec3 center = vec3(x, y, z);
        float scale = 1.0f + rnd() * 0.5f;

        // --- Build "X" Geometry ---
        float w = 0.5f * scale;
        float h = 1.0f * scale;

        vec3 n1 = vec3(0, 0, 1);
        vec3 v1 = center + vec3(-w, 0, 0); vec3 v2 = center + vec3(w, 0, 0);
        vec3 v3 = center + vec3(w, h, 0); vec3 v4 = center + vec3(-w, h, 0);

        vertices.push_back({ v1, vec2(0, 0), n1 }); vertices.push_back({ v2, vec2(1, 0), n1 });
        vertices.push_back({ v3, vec2(1, 1), n1 }); vertices.push_back({ v1, vec2(0, 0), n1 });
        vertices.push_back({ v3, vec2(1, 1), n1 }); vertices.push_back({ v4, vec2(0, 1), n1 });

        vec3 n2 = vec3(1, 0, 0);
        vec3 v5 = center + vec3(0, 0, -w); vec3 v6 = center + vec3(0, 0, w);
        vec3 v7 = center + vec3(0, h, w); vec3 v8 = center + vec3(0, h, -w);

        vertices.push_back({ v5, vec2(0, 0), n2 }); vertices.push_back({ v6, vec2(1, 0), n2 });
        vertices.push_back({ v7, vec2(1, 1), n2 }); vertices.push_back({ v5, vec2(0, 0), n2 });
        vertices.push_back({ v7, vec2(1, 1), n2 }); vertices.push_back({ v8, vec2(0, 1), n2 });
    }

    file.close();

    vertexCount = (GLsizei)vertices.size();
    printf("Loaded %d grass tufts from file.\n", vertexCount / 12);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GrassVertex), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex), (void*)offsetof(GrassVertex, normal));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GrassVertex), (void*)offsetof(GrassVertex, uv));
    glBindVertexArray(0);
}

Meadow::~Meadow() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &grassTexture);
}

void Meadow::draw(const mat4& view, const mat4& proj)
{
    if (vertexCount == 0) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    mat4 model = mat4(1.0f);
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &model[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glUniform1i(diffuseColorSampler, 0);

    glUniform1i(useTextureLocation, 1);
    glUniform1i(useTransparentTexLocation, 1);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);

    glUniform1i(useTransparentTexLocation, 0);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}
