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

    // Load Models
    bodyMesh = new Drawable("assets/car/car_body.obj");
    doorMesh = new Drawable("assets/car/car_door.obj");

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
    delete doorMesh;

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
}

void Car::drawOnlyObjects(GLuint shadowModelLocation)
{
    // Body Shadows
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    bodyMesh->bind(); bodyMesh->draw();

    // Door Shadows
    mat4 doorMatrix = getDoorMatrix();
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &doorMatrix[0][0]);
    doorMesh->bind(); doorMesh->draw();
}

mat4 Car::getDoorMatrix()
{
    // Rotates the door around the specified hinge position
    return modelMatrix
        * translate(mat4(), doorHingePosition)
        * rotate(mat4(), radians(doorOpenAngle), vec3(0, 1, 0))
        * translate(mat4(), -doorHingePosition);
}

void Car::update(float deltaTime)
{
    float doorSpeed = 120.0f; // Degrees per second

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

bool Car::toggleDoor()
{
    if (!targetDoorAngle > -30.0f) targetDoorAngle = 0.0f;

    return (targetDoorAngle > doorOpenAngle);
}

bool Car::checkCollision(const vec3& position, float radius)
{
    if (bodyMesh->checkCollision(position, radius, modelMatrix)) return true;

    return false;
}
