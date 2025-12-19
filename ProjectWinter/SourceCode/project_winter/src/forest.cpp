#include "forest.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace std;

extern GLuint depthTextureSamplerLocation;
extern GLuint depthUseTransparentTexLocation;
extern GLuint depthWindPowerLocation;

Forest::Forest(GLuint shaderProgram)
{
    // Initialize Position
    position      = vec3(0.0f, 0.0f, 0.0f);
    rotationAngle = 0.0f;
    scaleFactor   = 4.5f;
    updateModelMatrix();

    // Load Models
    trunkMesh  = new Drawable("assets/vegetation/forest_wood.obj");
    leavesMesh = new Drawable("assets/vegetation/forest_leaves.obj");

    // Load Textures
    trunkTexture  = loadBMP("assets/vegetation/wood.bmp");
    leavesTexture = loadBMP("assets/vegetation/leaf.bmp");

    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");
    useTransparentTex   = glGetUniformLocation(shaderProgram, "useTransparentTex");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");
    windPowerLocation   = glGetUniformLocation(shaderProgram, "windPower");
}

Forest::~Forest()
{
    delete trunkMesh;
    delete leavesMesh;

    glDeleteTextures(1, &trunkTexture);
    glDeleteTextures(1, &leavesTexture);
}

void Forest::setPosition(vec3 pos)
{
    position = pos;
    updateModelMatrix();
}

void Forest::setRotation(float angle)
{
	rotationAngle = angle;
	updateModelMatrix();
}

void Forest::updateModelMatrix()
{
    modelMatrix = translate(mat4(), position)
                * rotate(mat4(), rotationAngle, vec3(0, 1, 0))
                * scale(mat4(), vec3(scaleFactor));
}

void Forest::draw(int windPower)
{
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    // ================< Wood >================ //

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, trunkTexture);
    glUniform1i(diffuseColorSampler, 0);
    glUniform1i(useTextureLocation,  1); // We want to use texture (tell the shader)!

    trunkMesh->bind(); trunkMesh->draw();
    
    // ================< Leaves >================ //

    glDisable(GL_CULL_FACE); // Disable Culling: We want to see the back of the leaves!

    glUniform1i(windPowerLocation, windPower); // Enable wind animation

    // Bind Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, leavesTexture);
    glUniform1i(diffuseColorSampler, 0);
    glUniform1i(useTransparentTex, 1);

    leavesMesh->bind(); leavesMesh->draw();

    // Restore...
    glEnable(GL_CULL_FACE);
    glUniform1i(useTransparentTex, 0);
    glUniform1i(windPowerLocation, 0); // Disable wind animation
}

void Forest::drawOnlyObjects(GLuint shadowModelLocation, int windPower)
{
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    // Draw Trunk
    trunkMesh->bind(); trunkMesh->draw();

    // Draw Leaves
    // IMPORTANT: Disable culling for shadows too, otherwise 
    // light hitting the back of a leaf won't cast a shadow!
    glDisable(GL_CULL_FACE);

    glUniform1i(depthWindPowerLocation, windPower); // Enable wind animation
    
    // Bind Texture - leaves have transparency!
    // So the shadow casted should consider that!
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, leavesTexture);
    glUniform1i(depthTextureSamplerLocation, 0);
    glUniform1i(depthUseTransparentTexLocation, 1);

    leavesMesh->bind(); leavesMesh->draw();
    
    // Restore...
    glEnable(GL_CULL_FACE);
    glUniform1i(depthUseTransparentTexLocation, 0);
    glUniform1i(depthWindPowerLocation, 0); // Disable wind animation
}
