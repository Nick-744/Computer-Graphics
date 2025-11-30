#include "terrain.h"
#include <common/shader.h>
#include <common/texture.h>
#include <iostream>

using namespace glm;

TerrainRenderer::TerrainRenderer(GLuint shaderProgram_) : shaderProgram(shaderProgram_)
{
	// Special flag to indicate terrain rendering (ShadowMapping.fragmentshader)
	isTerrain = glGetUniformLocation(shaderProgram, "isTerrain");

    // Get Uniform Locations
    vpLocation   = glGetUniformLocation(shaderProgram, "VP");
    mLocation    = glGetUniformLocation(shaderProgram, "M");
    timeLocation = glGetUniformLocation(shaderProgram, "time");

    // Texture sampler locations in the shader
    textureSamplerWorld = glGetUniformLocation(shaderProgram, "textureSamplerWorld");

    textureSamplerSlope  = glGetUniformLocation(shaderProgram, "textureSamplerSlope");
    textureSamplerSoil   = glGetUniformLocation(shaderProgram, "textureSamplerSoil");
    textureSamplerPeaks  = glGetUniformLocation(shaderProgram, "textureSamplerPeaks");
    textureSamplerLake   = glGetUniformLocation(shaderProgram, "textureSamplerLake");
    textureSamplerRivers = glGetUniformLocation(shaderProgram, "textureSamplerRivers");

    textureSamplerRock  = glGetUniformLocation(shaderProgram, "textureSamplerRock");
    textureSamplerGrass = glGetUniformLocation(shaderProgram, "textureSamplerGrass");
    textureSamplerDirt  = glGetUniformLocation(shaderProgram, "textureSamplerDirt");
    textureSamplerSand  = glGetUniformLocation(shaderProgram, "textureSamplerSand");

    textureSamplerWater           = glGetUniformLocation(shaderProgram, "textureSamplerWater");
    textureSamplerDisplacement    = glGetUniformLocation(shaderProgram, "displacementTextureSampler");
    textureSamplerRiversDirection = glGetUniformLocation(shaderProgram, "textureSamplerRiversDirection");

    // Load Textures
    textureWorld = loadBMP("assets/worldmap_gaea/worldmap_texture_NO-BLUE.bmp");

    textureSlope  = loadBMP("assets/worldmap_gaea/slope_texture.bmp");
	textureSoil   = loadBMP("assets/worldmap_gaea/soil_texture.bmp");
	texturePeaks  = loadBMP("assets/worldmap_gaea/peaks_texture.bmp");
	textureLake   = loadBMP("assets/worldmap_gaea/lake_texture.bmp");
	textureRivers = loadBMP("assets/worldmap_gaea/rivers_texture.bmp");

    textureRock  = loadBMP("assets/world_textures/rock_face_03_diff_4k.bmp");
    textureGrass = loadBMP("assets/world_textures/brown_mud_leaves_01_diff_4k.bmp");
    textureDirt  = loadBMP("assets/world_textures/dirt_diff_4k.bmp");
    textureSand  = loadBMP("assets/world_textures/damp_sand_diff_4k.bmp");

    textureWater           = loadBMP("assets/world_textures/water.bmp");
    textureDisplacement    = loadBMP("assets/world_textures/gray.bmp");
    textureRiversDirection = loadBMP("assets/worldmap_gaea/rivers_direction.bmp");

    // Configure Texture Parameters (Filtering)
    glBindTexture(GL_TEXTURE_2D, textureSlope);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, textureSoil);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, texturePeaks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, textureLake);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, textureRivers);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load Mesh
    terrain = new Drawable("assets/worldmap_gaea/super_low_poly_worldmap.obj");
}

TerrainRenderer::~TerrainRenderer()
{
    // Cleanup
    glDeleteTextures(1, &textureWorld);
    
	glDeleteTextures(1, &textureSlope);
	glDeleteTextures(1, &textureSoil);
	glDeleteTextures(1, &texturePeaks);
	glDeleteTextures(1, &textureLake);
	glDeleteTextures(1, &textureRivers);

	glDeleteTextures(1, &textureRock);
	glDeleteTextures(1, &textureGrass);
	glDeleteTextures(1, &textureDirt);
	glDeleteTextures(1, &textureSand);

	glDeleteTextures(1, &textureWater);
	glDeleteTextures(1, &textureDisplacement);
	glDeleteTextures(1, &textureRiversDirection);

    delete terrain;
}

void TerrainRenderer::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, float time)
{
	glUseProgram(shaderProgram); // Just to be sure...

    glUniform1i(isTerrain, 1); // ShadowMapping bs...

    // Bind Textures to Units
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textureWorld);
    glUniform1i(textureSamplerWorld, 0);

    // Bind terrain attribute textures
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, textureSlope);
    glUniform1i(textureSamplerSlope, 1);

    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, textureSoil);
    glUniform1i(textureSamplerSoil, 2);

	glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, texturePeaks);
	glUniform1i(textureSamplerPeaks, 3);

	glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, textureLake);
	glUniform1i(textureSamplerLake, 4);

	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, textureRivers);
	glUniform1i(textureSamplerRivers, 5);

    // Bind detailed terrain textures
	glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, textureRock);
	glUniform1i(textureSamplerRock, 6);

	glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, textureGrass);
	glUniform1i(textureSamplerGrass, 7);

	glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, textureDirt);
	glUniform1i(textureSamplerDirt, 8);

	glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_2D, textureSand);
	glUniform1i(textureSamplerSand, 9);

	// Bind water and displacement textures
	glActiveTexture(GL_TEXTURE10); glBindTexture(GL_TEXTURE_2D, textureWater);
	glUniform1i(textureSamplerWater, 10);

    glActiveTexture(GL_TEXTURE11); glBindTexture(GL_TEXTURE_2D, textureDisplacement);
    glUniform1i(textureSamplerDisplacement, 11);

	glActiveTexture(GL_TEXTURE12); glBindTexture(GL_TEXTURE_2D, textureRiversDirection);
	glUniform1i(textureSamplerRiversDirection, 12);

    // Set Uniforms
    glUniform1f(timeLocation, time);

	mat4 modelMatrix = getTerrainModelMatrix();
    mat4 vp          = projectionMatrix * viewMatrix;

    glUniformMatrix4fv(vpLocation, 1, GL_FALSE, &vp[0][0]);
    glUniformMatrix4fv(mLocation,  1, GL_FALSE, &modelMatrix[0][0]);

    // Draw
    terrain->bind();
    terrain->draw();

	glUniform1i(isTerrain, 0); // ShadowMapping bs...
}
