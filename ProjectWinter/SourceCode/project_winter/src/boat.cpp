#include "boat.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>

Boat::Boat(GLuint shaderProgram, GLFWwindow* window) : window(window)
{
    // Initial position
    position      = vec3(-58.0f, 58.04f, 8.0f);
    rotationAngle = 1.6f;
    scaleFactor   = 2.0f;
    updateModelMatrix();

    // Link shader uniforms
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");

    // Load Meshes
    boatMesh = new Drawable("assets/boat/boat.obj");
    paddleL  = new Drawable("assets/boat/paddle_left.obj");
    paddleR  = new Drawable("assets/boat/paddle_right.obj");

    // Load Texture
    boatTexture = loadBMP("assets/boat/wood_BaseColor.bmp");
}

Boat::~Boat()
{
    delete boatMesh;
    delete paddleL;
    delete paddleR;

    glDeleteTextures(1, &boatTexture);
}

void Boat::updateModelMatrix()
{
    modelMatrix = translate(mat4(), position)
                * rotate(mat4(), rotationAngle, vec3(0, 1, 0))
                * scale(mat4(), vec3(scaleFactor));;
}

void Boat::draw()
{
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(useTextureLocation, 1);

    // Bind and Draw Boat
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, boatTexture);
    glUniform1i(diffuseColorSampler, 0);

    boatMesh->bind(); boatMesh->draw();

    // Bind and Draw Paddle (sharing the same model matrix for now...)
    paddleL->bind(); paddleL->draw();
    paddleR->bind(); paddleR->draw();
}

void Boat::drawOnlyObjects(GLuint shadowModelLocation)
{
    // Shadow mapping
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    boatMesh->bind(); boatMesh->draw();
    paddleL->bind();  paddleL->draw();
    paddleR->bind();  paddleR->draw();
}

mat4 Boat::getViewMatrix()
{
    // Calculate direction based on the boat's rotation angle!
    vec3 direction = vec3(sin(rotationAngle), 0, cos(rotationAngle));

    // Offset the player to be slightly above and inside the boat
    vec3 eye = position + vec3(0.0f, 1.5f, 0.0f) - direction * 2.5f;

    return lookAt(eye, eye + direction, vec3(0, 1, 0));
}

void Boat::steer(float deltaTime)
{
    float speed     = 2.0f; // Movement speed
    float turnSpeed = 0.1f; // Rotation speed

    // Handle Rotation
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) rotationAngle += turnSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) rotationAngle -= turnSpeed * deltaTime;

    // Calculate Direction Vector from current rotation
    // Note: Adjust sin/cos based on your model's initial orientation
    vec3 direction = vec3(sin(rotationAngle), 0, cos(rotationAngle));

    // Handle Forward/Backward (W/S or Up/Down)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += direction * speed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position -= direction * speed * deltaTime;

    // Finalize by updating the model matrix so the changes render
    updateModelMatrix();
}
