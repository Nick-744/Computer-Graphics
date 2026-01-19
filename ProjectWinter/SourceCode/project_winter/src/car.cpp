#include "car.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

Car::Car(GLuint shaderProgram)
{
    // Initialize Transform
    position       = vec3(-5.7f, 59.13f, 33.9f);
    rotationAngleY =  2.06f;
    rotationAngleX =  0.04f;
    rotationAngleZ = -0.09f;
    scaleFactor    = 0.66f;
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
    bodyMesh   = new Drawable("assets/car/car_body.obj");
    bodyWindow = new Drawable("assets/car/car_body_windows.obj");
    doorMesh   = new Drawable("assets/car/car_door.obj");
    doorWindow = new Drawable("assets/car/car_door_window.obj");

    // Load Textures
    carTexture = loadBMP("assets/car/car.bmp");

    // Door initial parameters
    doorOpenAngle     = -90.0f;
    targetDoorAngle   = -90.0f;
    doorHingePosition = vec3(1.15f, 0.5f, 1.34f);
}

Car::~Car()
{
    delete bodyMesh;
    delete bodyWindow;
    delete doorMesh;
    delete doorWindow;

    glDeleteTextures(1, &carTexture);
}

void Car::updateModelMatrix()
{
    modelMatrix = translate(mat4(), position)
                * rotate(mat4(), rotationAngleY, vec3(0, 1, 0))
                * rotate(mat4(), rotationAngleX, vec3(1, 0, 0))
                * rotate(mat4(), rotationAngleZ, vec3(0, 0, 1))
                * scale(mat4(), vec3(scaleFactor));
}

void Car::draw()
{
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(useTextureLocation, 1);

    // Bind the shared car texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, carTexture);
    glUniform1i(diffuseColorSampler, 0);

    // Draw Main Body
    bodyMesh->bind(); bodyMesh->draw();

    // Draw Animated Door
    mat4 doorMatrix = getDoorMatrix();
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &doorMatrix[0][0]);
    doorMesh->bind(); doorMesh->draw();

    // --- TRANSPARENT WINDOWS --- //
    glUniform1i(useTextureLocation, 0); // Use material for glass!
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Enable blending for transparency

    glUniform4f(KaLocation, 0.05f, 0.05f, 0.05f, 0.15f);
    glUniform4f(KdLocation, 0.0f,  0.0f,  0.0f,  0.15f);
    glUniform4f(KsLocation, 1.0f,  1.0f,  1.0f,  1.0f);
    glUniform1f(NsLocation, 256.0f);
    doorWindow->bind(); doorWindow->draw();

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    bodyWindow->bind(); bodyWindow->draw();

    glDisable(GL_BLEND); // Disable blending after drawing windows
}

void Car::drawOnlyObjects(GLuint shadowModelLocation, bool isSnowBuffer)
{
    // Body Shadows
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    bodyMesh->bind(); bodyMesh->draw();

    if (isSnowBuffer) // Don't snow the inside of the car!
    {
        bodyWindow->bind();
        bodyWindow->draw();
    }

    // Door Shadows
    mat4 doorMatrix = getDoorMatrix();
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &doorMatrix[0][0]);
    doorMesh->bind(); doorMesh->draw();

    if (isSnowBuffer)
    {
        doorWindow->bind();
        doorWindow->draw();
    }
}

mat4 Car::getDoorMatrix()
{
    // Rotates the door around the specified hinge position
    return modelMatrix
        * translate(mat4(), doorHingePosition)
        * rotate(mat4(), radians(doorOpenAngle), vec3(0, 1, 0))
        * translate(mat4(), -doorHingePosition);
}

bool Car::update(float deltaTime)
{
    if (doorOpenAngle == 0.0f) return true; // Trigger the sound...

    if (doorOpenAngle < targetDoorAngle)
    {
        float doorSpeed = 120.0f;
        doorOpenAngle  += doorSpeed * deltaTime;
        if (doorOpenAngle > targetDoorAngle) doorOpenAngle = targetDoorAngle;
    }

    return false;
}

void Car::toggleDoor() { targetDoorAngle = 0.0f; } // Only closing is needed...

bool Car::checkCollision(const vec3& position, float radius)
{
    if (bodyMesh->checkCollision(position, radius, modelMatrix)) return true;

    return false;
}
