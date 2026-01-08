#include "cabinInterior.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>

CabinInterior::CabinInterior(GLuint shaderProgram)
{
    // Cabin - Parent
    vec3 globalPos      = vec3(7.0f, 59.2f, -0.5f);
    float rotationAngle = 3.0f;
    globalModelMatrix   = translate(mat4(), globalPos);
    
    // Arcade - Child
    arcadePosition      = vec3(-2.1f, 0.13f, -1.8f);
    arcadeRotationAngle = radians(-8.7f);

    updateMatrices();

    // Link shader uniforms
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");

    // Load Assets
    arcadeMesh        = new Drawable("assets/cabin_interior/arcade.obj");
    arcadeMeshTexture = loadBMP("assets/cabin_interior/arcade.bmp");
    arcadeErrorText   = loadBMP("assets/cabin_interior/arcade_error.bmp");
    
    initArcade();
}

CabinInterior::~CabinInterior()
{
    delete arcadeMesh;
    delete arcadeScreen;

    glDeleteTextures(1, &arcadeMeshTexture);
    glDeleteTextures(1, &arcadeScreenTexture);
    glDeleteTextures(1, &arcadeErrorText);
}

void CabinInterior::updateMatrices()
{
    arcadeLocalMatrix = translate(mat4(), arcadePosition)
                      * rotate(mat4(), arcadeRotationAngle, vec3(0, 1, 0))
                      * scale(mat4(), vec3(0.012f));
}

void CabinInterior::draw()
{
    mat4 worldArcadeMatrix = globalModelMatrix * arcadeLocalMatrix;

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &worldArcadeMatrix[0][0]);
    glUniform1i(useTextureLocation, 1);

    // Bind and Draw Arcade
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, arcadeMeshTexture);
    glUniform1i(diffuseColorSampler, 0);

    arcadeMesh->bind(); arcadeMesh->draw();
}

bool CabinInterior::checkCollision(const vec3& position, float radius)
{
    mat4 worldArcadeMatrix = globalModelMatrix * arcadeLocalMatrix;
    return arcadeMesh->checkCollision(position, radius, worldArcadeMatrix);
}



// ===< Special screen - Render Java game >=== //
void CabinInterior::initArcade()
{
    vector<vec3> vertices = {
        vec3(-0.25f, -0.15f, 0.0f), vec3( 0.25f, -0.15f, 0.0f), vec3( 0.25f,  0.15f, 0.0f),
        vec3( 0.25f,  0.15f, 0.0f), vec3(-0.25f,  0.15f, 0.0f), vec3(-0.25f, -0.15f, 0.0f)
    };
    vector<vec2> uvs = {
        vec2(0, 1), vec2(1, 1), vec2(1, 0),
        vec2(1, 0), vec2(0, 0), vec2(0, 1)
    };

    arcadeScreen = new Drawable(vertices, uvs);

    // Create an empty texture
    glGenTextures(1, &arcadeScreenTexture);
    glBindTexture(GL_TEXTURE_2D, arcadeScreenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

bool CabinInterior::captureJavaWindow()
{
    // Find the Java window
    HWND hwnd = FindWindowA(NULL, "The Forbidden Spaceship");
    if (!hwnd) return false; // Game not running!

    // Get the device context
    HDC hdcWindow   = GetDC(hwnd);
    HDC hdcMem      = CreateCompatibleDC(hdcWindow);
    HBITMAP hbitmap = CreateCompatibleBitmap(hdcWindow, 1280, 720);
    SelectObject(hdcMem, hbitmap);

    // BitBlt: Copy the window pixels
    BitBlt(hdcMem, 0, 0, 1264, 718, hdcWindow, 0, 0, SRCCOPY);

    // Get the raw bits
    BITMAPINFOHEADER bi   = { sizeof(BITMAPINFOHEADER), 1280, -720, 1, 24, BI_RGB };
    unsigned char* buffer = new unsigned char[1280 * 720 * 3];
    GetDIBits(hdcMem, hbitmap, 0, 720, buffer, (BITMAPINFO*)& bi, DIB_RGB_COLORS);

    // Update Texture
    glBindTexture(GL_TEXTURE_2D, arcadeScreenTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1280, 720, GL_BGR, GL_UNSIGNED_BYTE, buffer);

    // Cleanup
    delete[] buffer;
    DeleteObject(hbitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);

    return true;
}

void CabinInterior::updateArcadeTexture()
{
    bool windowIsRunning = captureJavaWindow(); // Update the texture every frame

    // Draw the arcade screen!
    glUniform1i(useTextureLocation, 1);

    glActiveTexture(GL_TEXTURE0);
    if (windowIsRunning)
        glBindTexture(GL_TEXTURE_2D, arcadeScreenTexture);
    else
        glBindTexture(GL_TEXTURE_2D, arcadeErrorText);
    glUniform1i(diffuseColorSampler, 0);

    mat4 arcadeM = translate(mat4(), vec3(5.52f, 60.88f, -2.16f))
                 * rotate(mat4(),  1.42f, vec3(0, 1, 0))
                 * rotate(mat4(), -0.26f, vec3(1, 0, 0));
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &arcadeM[0][0]);

    arcadeScreen->bind(); arcadeScreen->draw();
}
