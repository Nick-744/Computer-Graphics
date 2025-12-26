#ifndef TERRAIN_H
#define TERRAIN_H

// Include GL headers
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <common/model.h>

using namespace glm;

class TerrainRenderer
{
public:
    // Constructor: Loads shaders and textures
    TerrainRenderer(GLuint shaderProgram);

    // Destructor: Cleans up memory
    ~TerrainRenderer();

    // The main function to render the terrain
    void draw(const mat4& viewMatrix, const mat4& projectionMatrix, float time, bool renderWall = true);

    Drawable* getTerrainMesh() { return land; }

    mat4 getTerrainModelMatrix() { return scale(mat4(), vec3(200.0f)); }

private:
    // Shader Program
    GLuint shaderProgram;

    GLuint terrainType; // ShadowMapping bs...

    // Uniform Locations
    GLuint vpLocation, mLocation, timeLocation;

    // Texture Sampler Locations
    GLuint textureSamplerWorld, textureSamplerSlope;
    GLuint textureSamplerRock, textureSamplerGrass, textureSamplerSand, textureSamplerWater, textureSamplerRiversDirection, textureSamplerDisplacement;

    // Actual Texture IDs
    GLuint textureWorld, textureSlope;
    GLuint textureRock, textureGrass, textureSand, textureWater, textureRiversDirection, textureDisplacement;

    // The 3D Meshes
    Drawable* land;
    Drawable* lake;
    Drawable* river;

    Drawable* snowWall;
};

#endif
