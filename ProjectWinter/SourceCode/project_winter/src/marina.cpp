#include "marina.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>

Marina::Marina(GLuint shaderProgram, vec3 pos, float rotation)
{
    modelMatrix = translate(mat4(), pos)
                * rotate(mat4(), rotation, vec3(0, 1, 0))
                * scale(mat4(), vec3(0.5f));

    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");

    // Load Assets
    logsMesh   = new Drawable("assets/marina/logs_port.obj");
    planksMesh = new Drawable("assets/marina/plank_port.obj");

    logsTexture   = loadBMP("assets/marina/logs_port.bmp");
    planksTexture = loadBMP("assets/marina/plank_port.bmp");
}

Marina::~Marina()
{
    delete logsMesh;
    delete planksMesh;

    glDeleteTextures(1, &logsTexture);
    glDeleteTextures(1, &planksTexture);
}

void Marina::draw()
{
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniform1i(useTextureLocation, 1);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(diffuseColorSampler, 0);

    // Draw Logs
    glBindTexture(GL_TEXTURE_2D, logsTexture);
    logsMesh->bind(); logsMesh->draw();

    // Draw Planks
    glBindTexture(GL_TEXTURE_2D, planksTexture);
    planksMesh->bind(); planksMesh->draw();
}

void Marina::drawOnlyObjects(GLuint shadowModelLocation)
{
    glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

    logsMesh->bind();   logsMesh->draw();
    planksMesh->bind(); planksMesh->draw();
}

bool Marina::checkCollision(const vec3& position, float radius, bool onlyLogsMesh)
{
    if (logsMesh->checkCollision(position, radius, modelMatrix))
        return true;
    if (planksMesh->checkCollision(position, radius, modelMatrix) && !onlyLogsMesh)
        return true;

    return false;
}
