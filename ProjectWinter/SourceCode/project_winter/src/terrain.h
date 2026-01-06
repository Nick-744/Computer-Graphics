#ifndef TERRAIN_H
#define TERRAIN_H

// Include GL headers
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <common/model.h>
#include <fstream>
#include <sstream>

using namespace glm;

struct Chunk
{
    Drawable* mesh;
    vec3 min;
    vec3 max;
};

class TerrainRenderer
{
public:
    // Constructor: Loads shaders and textures
    TerrainRenderer(GLuint shaderProgram);

    // Destructor: Cleans up memory
    ~TerrainRenderer();

    // The main function to render the terrain
    void draw(const mat4& viewMatrix, const mat4& projectionMatrix, float time, bool renderWall = true);
    bool checkCollision(const vec3& position, float radius, bool isLakeFrozen);
    bool checkCollisionBoat(const vec3& position, float radius);

    void drawOnlyObjects(GLuint shadowModelLocation, const mat4& lightVP);
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

    // Terrain Chunks
    std::vector<Chunk> landChunks;
    std::vector<Chunk> lakeChunks;
    bool isBoxInFrustum(const vec3& min, const vec3& max, const vec4 planes[6]); // Frustum Culling

    // The 3D Meshes
    Drawable* land; // Only for shadow pass...
    Drawable* river;
    
    // Dedicated collision mesh!
    Drawable* lakeWall;
    Drawable* worldWall;
    Drawable* lakeBoatWall;

    Drawable* snowWall;
};

#endif
