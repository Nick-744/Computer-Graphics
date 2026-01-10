#include "cabinInterior.h"
#include <common/texture.h>
#include <glm/gtc/matrix_transform.hpp>

// For Windows handling...
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

CabinInterior::CabinInterior(GLuint shaderProgram, GLFWwindow* window) : window(window)
{
    // Cabin - Parent
    globalPosition    = vec3(7.0f, 59.2f, -0.5f); // rotationAngle = 3.0
    globalModelMatrix = translate(mat4(), globalPosition);
    
    // Arcade - Child
    arcadePosition      = vec3(-2.1f, 0.13f, -1.8f);
    arcadeRotationAngle = radians(-8.7f);
    mat4 arcadeLocalMatrix   = translate(mat4(), arcadePosition)
                             * rotate(mat4(), arcadeRotationAngle, vec3(0, 1, 0))
                             * scale(mat4(), vec3(0.012f));
    worldArcadeMatrix        = globalModelMatrix * arcadeLocalMatrix;
    // Screen - IMPORTANT: ABSOLUTE WORLD POSITIONING!!!
    screenPosition = vec3(5.52f, 60.88f, -2.16f); // World position
    screenRotY     =  1.42f;
    screenRotX     = -0.26f;
    arcadeScreenModelMatrix = translate(mat4(), screenPosition)
                            * rotate(mat4(), screenRotY, vec3(0, 1, 0))
                            * rotate(mat4(), screenRotX, vec3(1, 0, 0));

    // Link shader uniforms
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    useTextureLocation  = glGetUniformLocation(shaderProgram, "useTexture");
    diffuseColorSampler = glGetUniformLocation(shaderProgram, "diffuseColorSampler");

    // Load Assets
    arcadeMesh        = new Drawable("assets/cabin_interior/arcade.obj");
    arcadeMeshTexture = loadBMP("assets/cabin_interior/arcade.bmp");
    arcadeStaticText  = loadBMP("assets/cabin_interior/arcade_static.bmp");
    
    initArcade();
}

CabinInterior::~CabinInterior()
{
    delete arcadeMesh;
    delete arcadeScreen;
    delete[] specialScreenBuffer;

    glDeleteTextures(1, &arcadeMeshTexture);
    glDeleteTextures(1, &arcadeScreenTexture);
    glDeleteTextures(1, &arcadeStaticText);
}

void CabinInterior::draw()
{
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

    arcadeScreen        = new Drawable(vertices, uvs);
    specialScreenBuffer = new unsigned char[1280 * 720 * 3];

    // Create an empty texture
    glGenTextures(1, &arcadeScreenTexture);
    glBindTexture(GL_TEXTURE_2D, arcadeScreenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void CabinInterior::launchJavaGame()
{
    // Get the full path of the running .exe
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);

    std::string path(buffer);

    // Manually move up 6 levels...
    for (int i = 0; i < 6; ++i)
    {
        size_t lastSlash = path.find_last_of("\\/");
        if (lastSlash != std::string::npos) path = path.substr(0, lastSlash);
    }

    // Append the Java project folder
    std::string workingDir = path + "\\TheForbiddenSpaceship"; // CWD: GitHub root...
    const char* parameters = "/c start /min java -cp bin mainPacket.MainClass";

    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = "open";
    sei.lpFile = "cmd.exe";
    sei.lpParameters = parameters;
    sei.lpDirectory  = workingDir.c_str();
    sei.nShow        = SW_HIDE;

    if (!ShellExecuteExA(&sei))
        std::cout << "[ERROR] Failed to launch Java game." << std::endl;
}

bool CabinInterior::captureJavaWindow()
{
    // Find the Java window
    if (!javaHwnd) javaHwnd = FindWindowA(NULL, "The Forbidden Spaceship");
    if (!javaHwnd) return false; // Game not running!

    // Get the device context
    HDC hdcWindow   = GetDC(javaHwnd);
    HDC hdcMem      = CreateCompatibleDC(hdcWindow);
    HBITMAP hbitmap = CreateCompatibleBitmap(hdcWindow, 1280, 720);
    SelectObject(hdcMem, hbitmap);

    // BitBlt: Copy the window pixels
    BitBlt(hdcMem, 0, 0, 1280, 720, hdcWindow, 0, 0, SRCCOPY);

    // Get the raw bits
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), 1280, -720, 1, 24, BI_RGB };
    GetDIBits(hdcMem, hbitmap, 0, 720, specialScreenBuffer, (BITMAPINFO*)& bi, DIB_RGB_COLORS);

    // Update Texture
    glBindTexture(GL_TEXTURE_2D, arcadeScreenTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1280, 720, GL_BGR, GL_UNSIGNED_BYTE, specialScreenBuffer);

    // Cleanup
    DeleteObject(hbitmap);
    DeleteDC(hdcMem);
    ReleaseDC(javaHwnd, hdcWindow);

    return true;
}

void CabinInterior::updateArcadeTexture()
{
    bool windowIsRunning = captureJavaWindow(); // Update the texture every frame
    
    // Try to launch the Java game ONLY ONCE when interacting...
    static bool launchTriggered = false;
    if (!windowIsRunning && !launchTriggered && interacting)
    {
        launchJavaGame();
        launchTriggered = true;
    }

    // Draw the arcade screen!
    glActiveTexture(GL_TEXTURE0);
    if (windowIsRunning)
        glBindTexture(GL_TEXTURE_2D, arcadeScreenTexture);
    else
        glBindTexture(GL_TEXTURE_2D, arcadeStaticText);
    glUniform1i(diffuseColorSampler, 0);

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &arcadeScreenModelMatrix[0][0]);

    arcadeScreen->bind(); arcadeScreen->draw();
}

void CabinInterior::toggleInteraction() // Like the boat... See: getArcadeScreenViewMatrix()
{
    interacting  = !interacting;
    HWND cppHwnd = glfwGetWin32Window(window);

    if (interacting)
    {
        // Force OpenGL/C++ window to stay on top!!!
        SetWindowPos(cppHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        if (javaHwnd) SetForegroundWindow(javaHwnd);
    }
    else
    {
        // Return C++ window to normal behavior...
        SetWindowPos(cppHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetForegroundWindow(cppHwnd);
    }

    // Reset Java input states when unmounting
    if (!interacting && javaHwnd)
    {
        UINT keys[] = { 'W', 'A', 'S', 'D', VK_LEFT, VK_RIGHT, VK_SPACE };
        for (UINT k : keys)
        {
            UINT scan = MapVirtualKey(k, MAPVK_VK_TO_VSC);
            PostMessageA(javaHwnd, WM_KEYUP, k, 0xC0000001 | (scan << 16));
        }
    }
}

mat4 CabinInterior::getArcadeScreenViewMatrix()
{
    // The screen's "forward" normal
    vec3 normal = vec3(
        sin(screenRotY),
        -sin(screenRotX),
        cos(screenRotY) * cos(screenRotX)
    );

    float distance = 1.1f; // Distance from screen to eye
    vec3 eye       = screenPosition + normalize(normal) * distance;

    return lookAt(eye, screenPosition, vec3(0, 1, 0));
}
