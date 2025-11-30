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
    void draw(const mat4& viewMatrix, const mat4& projectionMatrix, float time);

	Drawable* getTerrainMesh() { return terrain; }

    mat4 getTerrainModelMatrix() { return scale(mat4(), vec3(10.0f)); }

private:
    // Shader Program
    GLuint shaderProgram;

	GLuint isTerrain; // ShadowMapping bs...

    // Uniform Locations
    GLuint vpLocation, mLocation, timeLocation;

    // Texture Sampler Locations
    GLuint textureSamplerWorld, textureSamplerSlope, textureSamplerSoil, textureSamplerPeaks, textureSamplerLake, textureSamplerRivers;
    GLuint textureSamplerRock, textureSamplerGrass, textureSamplerDirt, textureSamplerSand, textureSamplerWater, textureSamplerDisplacement, textureSamplerRiversDirection;

    // Actual Texture IDs
    GLuint textureWorld, textureSlope, textureSoil, texturePeaks, textureLake, textureRivers;
    GLuint textureRock, textureGrass, textureDirt, textureSand, textureWater, textureRiversDirection, textureDisplacement;

    // The 3D Mesh
    Drawable* terrain;
};

#endif
