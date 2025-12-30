#include "cabin.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace std;

Cabin::Cabin(GLuint shaderProgram)
{
    // Initialize Transform
    position      = vec3(7.0f, 59.2f, -0.5f);
    rotationAngle = 3.0f;
    scaleFactor   = 1.0f;
    updateModelMatrix();

    // Load Uniforms
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");
    
    // Material Uniforms (WE NEED GLASS MATERIAL FOR WINDOWS!!!)
    KaLocation = glGetUniformLocation(shaderProgram, "mtl.Ka");
    KdLocation = glGetUniformLocation(shaderProgram, "mtl.Kd");
    KsLocation = glGetUniformLocation(shaderProgram, "mtl.Ks");
    NsLocation = glGetUniformLocation(shaderProgram, "mtl.Ns");

    // Load Models
    baseMesh        = new Drawable("assets/cabin/model/Base.obj");
    doorMesh        = new Drawable("assets/cabin/model/Door.obj");
    doorWinMesh     = new Drawable("assets/cabin/model/DoorWin.obj");
    floorMesh       = new Drawable("assets/cabin/model/Floor.obj");
    roofMesh        = new Drawable("assets/cabin/model/Roof.obj");
    supportStrMesh  = new Drawable("assets/cabin/model/SupportStr.obj");
    wallsFrBackMesh = new Drawable("assets/cabin/model/Walls_FrBack.obj");
    wallsSideMesh   = new Drawable("assets/cabin/model/Walls_Side.obj");

    // Load Textures
    baseTexture        = loadBMP("assets/cabin/textures/BuildingBase_BaseColor_10.bmp");
    doorWinTexture     = loadBMP("assets/cabin/textures/DoorAndWindows_BaseColor_1.bmp");
    floorTexture       = loadBMP("assets/cabin/textures/Floor_BaseColor_4.bmp");
    roofTexture        = loadBMP("assets/cabin/textures/Roof_BaseColor_19.bmp");
    supportStrTexture  = loadBMP("assets/cabin/textures/WoodenSupportStruct_BaseColor_7.bmp");
    wallsFrBackTexture = loadBMP("assets/cabin/textures/WallsFrontandBack_BaseColor_13.bmp");
    wallsSideTexture   = loadBMP("assets/cabin/textures/WallsSides_BaseColor_16.bmp");
    
    // Door initial parameters
    doorOpenAngle     = 0.0f;
    targetDoorAngle   = 0.0f; // Angle to animate towards!
    doorHingePosition = vec3(-0.373f, 0.0f, 2.834f); // AFTER TESTING...
}

Cabin::~Cabin()
{
    // Cleanup Memory
    delete baseMesh;
    delete doorMesh;
    delete doorWinMesh;
    delete floorMesh;
    delete roofMesh;
    delete supportStrMesh;
    delete wallsFrBackMesh;
    delete wallsSideMesh;

    // Cleanup Textures
    glDeleteTextures(1, &baseTexture);
    glDeleteTextures(1, &doorWinTexture);
    glDeleteTextures(1, &floorTexture);
    glDeleteTextures(1, &roofTexture);
    glDeleteTextures(1, &supportStrTexture);
    glDeleteTextures(1, &wallsFrBackTexture);
    glDeleteTextures(1, &wallsSideTexture);
}

void Cabin::updateModelMatrix()
{
    modelMatrix = translate(mat4(), position)
                * rotate(mat4(), rotationAngle, vec3(0, 1, 0))
                * scale(mat4(), vec3(scaleFactor));
}

void Cabin::draw()
{
	// Send the model matrix ONCE, as all parts share the same origin (for my sanity...)
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    // Tell shader to use texture
    glUniform1i(useTextureLocation, 1);

    // For each part: Bind Texture -> Bind Mesh -> Draw

    // Base
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, baseTexture);
    glUniform1i(diffuseColorSampler, 0);
    baseMesh->bind(); baseMesh->draw();

    // Floor
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, floorTexture);
    glUniform1i(diffuseColorSampler, 0);
    floorMesh->bind(); floorMesh->draw();

    // Walls Front/Back
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, wallsFrBackTexture);
    glUniform1i(diffuseColorSampler, 0);
    wallsFrBackMesh->bind(); wallsFrBackMesh->draw();

    // Walls Side
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, wallsSideTexture);
    glUniform1i(diffuseColorSampler, 0);
    wallsSideMesh->bind(); wallsSideMesh->draw();

    // Roof
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, roofTexture);
    glUniform1i(diffuseColorSampler, 0);
    roofMesh->bind(); roofMesh->draw();

    // Support Structure
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, supportStrTexture);
    glUniform1i(diffuseColorSampler, 0);
    supportStrMesh->bind(); supportStrMesh->draw();

    // Door/Windows Frame
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, doorWinTexture);
    glUniform1i(diffuseColorSampler, 0);
    doorWinMesh->bind(); doorWinMesh->draw();



    // --- DRAW DYNAMIC DOOR --- //
    mat4 doorMatrix = getDoorMatrix();
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &doorMatrix[0][0]);
    
    doorMesh->bind(); doorMesh->draw(); // The texture is already bound - doorWinTexture!

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Reset model matrix...
}

void Cabin::drawOnlyObjects(GLuint shadowModelLocation)
{
    // Used for Shadow Mapping pass (Depth Buffer)
    // No textures needed, just geometry!

    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    baseMesh->bind();        baseMesh->draw();
    floorMesh->bind();       floorMesh->draw();
    wallsFrBackMesh->bind(); wallsFrBackMesh->draw();
    wallsSideMesh->bind();   wallsSideMesh->draw();
    roofMesh->bind();        roofMesh->draw();
    supportStrMesh->bind();  supportStrMesh->draw();
    doorWinMesh->bind();     doorWinMesh->draw();

    // --- DRAW DOOR SHADOWS --- //
    mat4 doorMatrix = getDoorMatrix();
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &doorMatrix[0][0]);
    doorMesh->bind(); doorMesh->draw();
}



// --- Door --- //
mat4 Cabin::getDoorMatrix()
{
    return modelMatrix
        * translate(mat4(), doorHingePosition)
        * rotate(mat4(), radians(doorOpenAngle), vec3(0, 1, 0))
        * translate(mat4(), -doorHingePosition);
}

void Cabin::update(float deltaTime)
{
    float doorSpeed = 120.0f; // Speed: Degrees per second

    // Smoothly interpolate current angle towards target angle!
    if (doorOpenAngle < targetDoorAngle)
    {
        doorOpenAngle += doorSpeed * deltaTime;
        if (doorOpenAngle > targetDoorAngle) doorOpenAngle = targetDoorAngle;
    }
    else if (doorOpenAngle > targetDoorAngle)
    {
        doorOpenAngle -= doorSpeed * deltaTime;
        if (doorOpenAngle < targetDoorAngle) doorOpenAngle = targetDoorAngle;
    }
}

void Cabin::toggleDoor()
{
    // If target is 0, switch to 90. If 90, switch to 0.
    if (targetDoorAngle < 45.0f) targetDoorAngle = 90.0f;
    else targetDoorAngle = 0.0f;
}

bool Cabin::checkCollision(const vec3& position, float radius)
{
    if (wallsSideMesh->checkCollision(position, radius, modelMatrix)) return true;
    if (doorMesh->checkCollision(position, radius, getDoorMatrix()))  return true;

    // We do NOT check floor/roof here (handled by gravity/terrain logic usually)
    return false;
}
