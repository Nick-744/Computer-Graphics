#include "meadow.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

// Get the global location variables from main.cpp
extern GLuint depthTextureSamplerLocation;
extern GLuint depthUseTransparentTexLocation;

vec3 normalCorrectLighting = vec3(1.0f, 1.0f, 1.0f); // SPECIAL NORMAL for lighting...

// Random helper: returns float between 0.0 and 1.0
float rnd() { return (float) rand() / (float) RAND_MAX; }

Meadow::Meadow(GLuint shaderProgram)
{
    this->shaderProgram = shaderProgram;

    modelMatrixLocation       = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation        = glGetUniformLocation(shaderProgram, "useTexture");
    useTransparentTexLocation = glGetUniformLocation(shaderProgram, "useTransparentTex");
    diffuseColorSampler       = glGetUniformLocation(shaderProgram, "diffuseColorSampler");

    // --- SETUP GRASS --- //
    grassTexture = loadBMP("assets/vegetation/grass.bmp");

    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    loadAndGenerateMesh("assets/grass_positions_final.txt");

    // --- SETUP TREES --- //
    treeTexture = loadBMP("assets/vegetation/tree.bmp");

    glBindTexture(GL_TEXTURE_2D, treeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    loadAndGenerateTrees("assets/trees_positions_final.txt");
}

// ---< Grass Loader >--- //
void Meadow::loadAndGenerateMesh(const char* filepath)
{
    vector<GrassVertex> vertices;
    ifstream file(filepath);

    if (!file.is_open())
    {
        printf("ERROR: Could not open grass positions: %s\n", filepath);
        return;
    }

    string line;
    const float PI = 3.14159265359f;

    while (getline(file, line))
    {
        stringstream ss(line);
        float x, y, z;
        if (!(ss >> x >> y >> z)) continue;

        vec3 center = vec3(x, y, z);

        // Random Size (Scale 0.7 to 1.5)
        float scale = 0.7f + rnd() * 0.8f;

        // Random Rotation on Y (0 to 360 degrees in radians)
        float randomRotation = rnd() * 2.0f * PI;

        float w = 0.5f * scale; // Half width relative to center
        float h = 1.0f * scale; // Height

        // Cross arrangement of 3 quads (60 degrees / PI/3 apart)
        for (int i = 0; i < 3; i++)
        {
            // Calculate angle for this specific blade (base rotation + 60 degree steps)
            float angle = randomRotation + (float)i * (PI / 3.0f);

            // Calculate offsets based on rotation
            float dx = cos(angle) * w;
            float dz = sin(angle) * w;

            // Define the 4 corners relative to the center
            vec3 vBottomLeft  = center + vec3(-dx, 0, -dz);
            vec3 vBottomRight = center + vec3(dx, 0, dz);
            vec3 vTopLeft     = center + vec3(-dx, h, -dz);
            vec3 vTopRight    = center + vec3(dx, h, dz);

            vec3 normal = normalCorrectLighting;

            // Push 2 Triangles (1 Quad)

            // Triangle 1
            vertices.push_back({ vBottomLeft,  vec2(0, 0), normal });
            vertices.push_back({ vBottomRight, vec2(1, 0), normal });
            vertices.push_back({ vTopRight,    vec2(1, 1), normal });

            // Triangle 2
            vertices.push_back({ vBottomLeft, vec2(0, 0), normal });
            vertices.push_back({ vTopRight,   vec2(1, 1), normal });
            vertices.push_back({ vTopLeft,    vec2(0, 1), normal });
        }
    }

    file.close();

    vertexCount = (GLsizei)vertices.size();
    // Adjusted printf division by 18 (3 quads * 2 tris * 3 verts)
    printf("Loaded %d grass tufts from file.\n", vertexCount / 18);

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

// ---< Tree Loader >--- //
void Meadow::loadAndGenerateTrees(const char* filepath)
{
    vector<GrassVertex> vertices;
    ifstream file(filepath);

    if (!file.is_open())
    {
        printf("ERROR: Could not open tree positions: %s\n", filepath);
        return;
    }

    string line;
    const float PI = 3.14159265359f;

    while (getline(file, line))
    {
        stringstream ss(line);
        float x, y, z;
        if (!(ss >> x >> y >> z)) continue;

        vec3 center = vec3(x, y, z);

        // Trees are larger than grass!
        float scale = 6.0f + rnd() * 2.0f;

        float randomRotation = rnd() * 2.0f * PI;

        float w = 0.5f * scale;
        float h = 1.0f * scale;

        // Cross arrangement (only 2 planes for trees...)
        for (int i = 0; i < 2; i++)
        {
            float angle = randomRotation + (float)i * (PI / 2.0f);

            float dx = cos(angle) * w;
            float dz = sin(angle) * w;

            vec3 vBottomLeft  = center + vec3(-dx, 0, -dz);
            vec3 vBottomRight = center + vec3(dx, 0, dz);
            vec3 vTopLeft     = center + vec3(-dx, h, -dz);
            vec3 vTopRight    = center + vec3(dx, h, dz);
            
            vec3 normal = normalCorrectLighting;

            vertices.push_back({ vBottomLeft,  vec2(0, 0), normal });
            vertices.push_back({ vBottomRight, vec2(1, 0), normal });
            vertices.push_back({ vTopRight,    vec2(1, 1), normal });

            vertices.push_back({ vBottomLeft, vec2(0, 0), normal });
            vertices.push_back({ vTopRight,   vec2(1, 1), normal });
            vertices.push_back({ vTopLeft,    vec2(0, 1), normal });
        }
    }

    file.close();

    treeVertexCount = (GLsizei)vertices.size();
    printf("Loaded %d trees.\n", treeVertexCount / 18);

    // Generate separate VAO/VBO for trees
    glGenVertexArrays(1, &treeVAO);
    glGenBuffers(1, &treeVBO);
    glBindVertexArray(treeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GrassVertex), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex), (void*)offsetof(GrassVertex, normal));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GrassVertex), (void*)offsetof(GrassVertex, uv));
    glBindVertexArray(0);
}

Meadow::~Meadow()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &grassTexture);

    glDeleteVertexArrays(1, &treeVAO);
    glDeleteBuffers(1, &treeVBO);
    glDeleteTextures(1, &treeTexture);
}

void Meadow::draw()
{
    // Disable Culling so the flat quads are visible from both sides
    glDisable(GL_CULL_FACE);

    mat4 model = mat4();
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &model[0][0]);

    glUniform1i(useTextureLocation, 1);
    glUniform1i(useTransparentTexLocation, 1);

    // --- DRAW GRASS --- //
    if (vertexCount > 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glUniform1i(diffuseColorSampler, 0);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    // --- DRAW TREES --- //
    if (treeVertexCount > 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, treeTexture);
        glUniform1i(diffuseColorSampler, 0);

        glBindVertexArray(treeVAO);
        glDrawArrays(GL_TRIANGLES, 0, treeVertexCount);
    }

    // Reset / Cleanup
    glBindVertexArray(0);
    glUniform1i(useTransparentTexLocation, 0);
    glEnable(GL_CULL_FACE);
}

void Meadow::drawOnlyObjects(GLuint shadowModelLocation)
{
    // The positions of grass/trees are already "baked" into the VBOs in world space!
    mat4 model = mat4();
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &model[0][0]);

    // Disable Culling - Shadows must be cast from both sides of the 2D quad!
    glDisable(GL_CULL_FACE);

    // Enable Transparency Check in Depth Shader
    glUniform1i(depthUseTransparentTexLocation, 1);

    // --- DRAW GRASS --- //
    if (vertexCount > 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);

        // Use the global location for the Depth Shader sampler
        glUniform1i(depthTextureSamplerLocation, 0);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    // --- DRAW TREES --- //
    if (treeVertexCount > 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, treeTexture);

        // Texture unit 0 is still active
        glUniform1i(depthTextureSamplerLocation, 0);

        glBindVertexArray(treeVAO);
        glDrawArrays(GL_TRIANGLES, 0, treeVertexCount);
    }

    // Cleanup
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);

    // Turn off transparency check...
    glUniform1i(depthUseTransparentTexLocation, 0);
}
