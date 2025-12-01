#include "forest.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace std;

Forest::Forest(GLuint shaderProgram)
{
    // Initialize Position
    position = vec3(0.0f, 0.0f, 0.0f);
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

void Forest::updateModelMatrix()
{
    modelMatrix = translate(mat4(), position) * scale(mat4(), vec3(4.5));
}

void Forest::draw()
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

    // Bind Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, leavesTexture);
    glUniform1i(diffuseColorSampler, 0);
    glUniform1i(useTransparentTex,  1);

    leavesMesh->bind(); leavesMesh->draw();

    // Restore Culling!
    glEnable(GL_CULL_FACE);
    glUniform1i(useTransparentTex, 0);
}

void Forest::drawOnlyObjects(GLuint shadowModelLocation)
{
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    // Draw Trunk
    trunkMesh->bind(); trunkMesh->draw();

    // Draw Leaves
    // IMPORTANT: Disable culling for shadows too, otherwise 
    // light hitting the back of a leaf won't cast a shadow!
    glDisable(GL_CULL_FACE);

    leavesMesh->bind(); leavesMesh->draw();

    glEnable(GL_CULL_FACE);
}
