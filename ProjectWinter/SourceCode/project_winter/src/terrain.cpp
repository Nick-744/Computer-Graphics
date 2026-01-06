#include "terrain.h"
#include <common/shader.h>
#include <common/texture.h>
#include <iostream>

extern float snowInflate;

TerrainRenderer::TerrainRenderer(GLuint shaderProgram_) : shaderProgram(shaderProgram_)
{
    // Special flag to indicate terrain rendering (ShadowMapping.fragmentshader)
    terrainType = glGetUniformLocation(shaderProgram, "terrainType");

    // Get Uniform Locations
    vpLocation   = glGetUniformLocation(shaderProgram, "VP");
    mLocation    = glGetUniformLocation(shaderProgram, "M");
    timeLocation = glGetUniformLocation(shaderProgram, "time");

    // Texture sampler locations in the shader
    textureSamplerWorld = glGetUniformLocation(shaderProgram, "textureSamplerWorld");

    textureSamplerSlope = glGetUniformLocation(shaderProgram, "textureSamplerSlope");

    textureSamplerRock  = glGetUniformLocation(shaderProgram, "textureSamplerRock");
    textureSamplerGrass = glGetUniformLocation(shaderProgram, "textureSamplerGrass");
    textureSamplerSand  = glGetUniformLocation(shaderProgram, "textureSamplerSand");

    textureSamplerWater           = glGetUniformLocation(shaderProgram, "textureSamplerWater");
    textureSamplerDisplacement    = glGetUniformLocation(shaderProgram, "displacementTextureSampler");
    textureSamplerRiversDirection = glGetUniformLocation(shaderProgram, "textureSamplerRiversDirection");

    // Load Textures
    textureWorld = loadBMP("assets/worldmap_gaea/worldmap_texture_NO-BLUE.bmp");

    textureSlope = loadBMP("assets/worldmap_gaea/slope_texture.bmp");

    textureRock  = loadBMP("assets/world_textures/rock_face_03_diff_4k.bmp");
    textureGrass = loadBMP("assets/world_textures/brown_mud_leaves_01_diff_4k.bmp");
    textureSand  = loadBMP("assets/world_textures/damp_sand_diff_4k.bmp");

    textureWater           = loadBMP("assets/world_textures/water.bmp");
    textureDisplacement    = loadBMP("assets/world_textures/gray.bmp");
    textureRiversDirection = loadBMP("assets/worldmap_gaea/rivers_direction.bmp");

    // Configure Texture Parameters (Filtering)
    glBindTexture(GL_TEXTURE_2D, textureSlope);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load Land/Lake Chunks from info file...
    std::ifstream infoFile("assets/worldmap_gaea/chunks/chunks_info.txt");
    std::string line;
    while (std::getline(infoFile, line))
    {
        std::stringstream ss(line);
        std::string filename;
        vec3 cMin, cMax;
        if (!(ss >> filename >> cMin.x >> cMin.y >> cMin.z >> cMax.x >> cMax.y >> cMax.z))
            continue;

        Chunk c;
        c.mesh = new Drawable("assets/worldmap_gaea/chunks/" + filename);
        c.min  = cMin;
        c.max  = cMax;
        
        if      (filename.find("land_") == 0) landChunks.push_back(c);
        else if (filename.find("lake_") == 0) lakeChunks.push_back(c);
    }

    // Load Mesh
    river = new Drawable("assets/worldmap_gaea/terrain_river.obj");
    
    // Load snow wall mesh!
    snowWall = new Drawable("assets/worldmap_gaea/terrain_wall.obj");

    // Dedicated collision mesh!
    lakeWall     = new Drawable("assets/collision_walls/lake_invisible_wall.obj");
    worldWall    = new Drawable("assets/collision_walls/world_invisible_wall.obj");
    lakeBoatWall = new Drawable("assets/collision_walls/lake_boat_inv_wall.obj");
}

TerrainRenderer::~TerrainRenderer()
{
    // Cleanup
    glDeleteTextures(1, &textureWorld);
    
    glDeleteTextures(1, &textureSlope);

    glDeleteTextures(1, &textureRock);
    glDeleteTextures(1, &textureGrass);
    glDeleteTextures(1, &textureSand);
    
    glDeleteTextures(1, &textureWater);
    glDeleteTextures(1, &textureDisplacement);
    glDeleteTextures(1, &textureRiversDirection);

    // Clean up Chunks
    for (auto& chunk : landChunks) delete chunk.mesh;
    landChunks.clear(); // Clear the vector of invalid pointers
    for (auto& chunk : lakeChunks) delete chunk.mesh;
    lakeChunks.clear();

    delete river;
    
    delete snowWall;

    delete lakeWall;
    delete worldWall;
    delete lakeBoatWall;
}

void TerrainRenderer::draw(const mat4& viewMatrix, const mat4& projectionMatrix, float time, bool renderWall)
{
    glUseProgram(shaderProgram); // Just to be sure...

    // Bind Textures to Units
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textureWorld);
    glUniform1i(textureSamplerWorld, 0);

    // Bind terrain attribute textures
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, textureSlope);
    glUniform1i(textureSamplerSlope, 1);

    // Bind detailed terrain textures
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, textureRock);
    glUniform1i(textureSamplerRock, 2);
    
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, textureGrass);
    glUniform1i(textureSamplerGrass, 3);
    
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, textureSand);
    glUniform1i(textureSamplerSand, 4);
    
    // Bind water and displacement textures
    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, textureWater);
    glUniform1i(textureSamplerWater, 5);

    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, textureDisplacement);
    glUniform1i(textureSamplerDisplacement, 6);
    
    glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, textureRiversDirection);
    glUniform1i(textureSamplerRiversDirection, 7);

    // Set Uniforms
    glUniform1f(timeLocation, time);

	mat4 modelMatrix = getTerrainModelMatrix();
    mat4 vp          = projectionMatrix * viewMatrix;

    glUniformMatrix4fv(vpLocation, 1, GL_FALSE, &vp[0][0]);
    glUniformMatrix4fv(mLocation,  1, GL_FALSE, &modelMatrix[0][0]);



    // Draw
    vec4 planes[6];
    mat4 m    = transpose(vp);
    planes[0] = m[3] + m[0]; planes[1] = m[3] - m[0];
    planes[2] = m[3] + m[1]; planes[3] = m[3] - m[1];
    planes[4] = m[3] + m[2]; planes[5] = m[3] - m[2];

    glUniform1i(terrainType, 1); // ShadowMapping bs...
    for (auto& chunk : landChunks)
        if (isBoxInFrustum(chunk.min, chunk.max, planes))
        {
            chunk.mesh->bind();
            chunk.mesh->draw();
        }
    


    if (snowInflate > 0.001f && renderWall) // Snow wall is special...
    {
        glDisable(GL_CULL_FACE);
        snowWall->bind(); snowWall->draw();
        glEnable(GL_CULL_FACE);
    }

    glUniform1i(terrainType, 2); // ShadowMapping bs...
    for (auto& chunk : lakeChunks)
        if (isBoxInFrustum(chunk.min, chunk.max, planes))
        {
            chunk.mesh->bind();
            chunk.mesh->draw();
        }

    glUniform1i(terrainType, 3); // ShadowMapping bs...
    river->bind(); river->draw();
    
    glUniform1i(terrainType, 0); // ShadowMapping bs...
}

void TerrainRenderer::drawOnlyObjects(GLuint shadowModelLocation, const mat4& lightVP)
{
    vec4 planes[6];
    mat4 m    = transpose(lightVP);
    planes[0] = m[3] + m[0]; planes[1] = m[3] - m[0];
    planes[2] = m[3] + m[1]; planes[3] = m[3] - m[1];
    planes[4] = m[3] + m[2]; planes[5] = m[3] - m[2];

    // Set the terrain model matrix
    mat4 modelMatrix = getTerrainModelMatrix();
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    for (auto& chunk : landChunks)
        if (isBoxInFrustum(chunk.min, chunk.max, planes))
        {
            chunk.mesh->bind();
            chunk.mesh->draw();
        }
}

bool TerrainRenderer::checkCollision(const vec3& position, float radius, bool isLakeFrozen)
{
    if (worldWall->checkCollision(position, radius, getTerrainModelMatrix()))
        return true;
    if (!isLakeFrozen && lakeWall->checkCollision(position, radius, getTerrainModelMatrix()))
        return true;
	
    return false;
}

bool TerrainRenderer::checkCollisionBoat(const vec3& position, float radius)
{
    if (lakeWall->checkCollision(position, radius, getTerrainModelMatrix()))     return true;
    if (lakeBoatWall->checkCollision(position, radius, getTerrainModelMatrix())) return true;

    return false;
}

bool TerrainRenderer::isBoxInFrustum(const vec3& min, const vec3& max, const vec4 planes[6])
{
    for (int i = 0; i < 6; i++)
    {
        int out = 0;
        if (dot(planes[i], vec4(min.x, min.y, min.z, 1.0f)) < 0.0) out++;
        if (dot(planes[i], vec4(max.x, min.y, min.z, 1.0f)) < 0.0) out++;
        if (dot(planes[i], vec4(min.x, max.y, min.z, 1.0f)) < 0.0) out++;
        if (dot(planes[i], vec4(max.x, max.y, min.z, 1.0f)) < 0.0) out++;
        if (dot(planes[i], vec4(min.x, min.y, max.z, 1.0f)) < 0.0) out++;
        if (dot(planes[i], vec4(max.x, min.y, max.z, 1.0f)) < 0.0) out++;
        if (dot(planes[i], vec4(min.x, max.y, max.z, 1.0f)) < 0.0) out++;
        if (dot(planes[i], vec4(max.x, max.y, max.z, 1.0f)) < 0.0) out++;

        if (out == 8) return false; // Entirely outside this plane
    }

    return true;
}
