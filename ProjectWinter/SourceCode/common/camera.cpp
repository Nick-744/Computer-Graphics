#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include <fstream>
#include <iostream>

using namespace glm;

Camera::Camera(GLFWwindow* window) : window(window) {
    position        = vec3(0, 60, 0);
    horizontalAngle = 3.14f;
    verticalAngle   = 0.0f;
    FoV             = 45.0f;
    speed           = 4.0f;
    mouseSpeed      = 0.001f;
    fovSpeed        = 100.0f;

    // For camera's height management...
    loadTerrain("assets/terrain_triangles_geometry_final.txt");
}

void Camera::update()
{
    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();
    // Static variable to track walking time for head bobbing...
    static float walkTimer = 0.0f;

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime    = float(currentTime - lastTime);



    // Get mouse position
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Reset mouse position for next frame
    glfwSetCursorPos(window, width / 2, height / 2);



    // Task 5.3: Compute new horizontal and vertical angles, given windows size
    //*/
    // and cursor position
    horizontalAngle += mouseSpeed * float(width / 2 - xPos);
    verticalAngle   += mouseSpeed * float(height / 2 - yPos);

    // Don't flip over...
    float temp = 1.2f;
    if (verticalAngle >  temp) verticalAngle =  temp;
    if (verticalAngle < -temp) verticalAngle = -temp;



    // Task 5.4: right and up vectors of the camera coordinate system
    // use spherical coordinates
    vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );

    // Right vector
    vec3 right(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );

    // Up vector
    vec3 up = cross(right, direction);



    // Task 5.5: update camera position using the direction/right vectors
	bool isMoving = false; // For head bobbing!
    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        position += direction * deltaTime * speed;
		isMoving  = true;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        position -= direction * deltaTime * speed;
        isMoving  = true;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        position += right * deltaTime * speed;
        isMoving  = true;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        position -= right * deltaTime * speed;
        isMoving  = true;
    }
    
    
    
    // ===< FLYING MODE TOGGLE >=== //
    static int lastToggleState = GLFW_RELEASE; // Remembered between frames
    int currentToggleState     = glfwGetKey(window, GLFW_KEY_F);

    if (currentToggleState == GLFW_PRESS && lastToggleState == GLFW_RELEASE)
    {
        flyingMode = !flyingMode;
        speed      = flyingMode ? 20.0f : 4.0f;
    }
    lastToggleState = currentToggleState; // Didn't want to overcomplicate things with pollKeyboard...

    vec3 eyePosition = position;
    if (!flyingMode) // Realistic camera movement...
    {
        // ===< TERRAIN HEIGHT ADJUSTMENT >=== //
        {
            // Calculate height of the ground at current (x, z)
            float groundHeight = getTerrainHeight(position.x, position.z);
            position.y         = groundHeight + 1.8f;
        }

        // ===< HEAD BOBBING >=== //
        if (isMoving)
        {
            // Increment timer based on movement speed
            walkTimer += deltaTime * speed * 2.5f;

            // Vertical Bob (Up/Down)
            float bobOffset = sin(walkTimer) * 0.1f;
            eyePosition.y  += bobOffset;

            // Horizontal Sway (Left/Right) - Simulates weight shifting...
            float swayOffset = cos(walkTimer * 0.5f) * 0.08f;
            eyePosition     += right * swayOffset;
        }
        else walkTimer = 0.0f; // Reset timer to 0!
    }



    // Task 5.6: handle ZOOM EFFECT!
    float targetFoV = 45.0f; // Default Base FoV
    
    // If SPACE is held, ZOOOOOOOOM IN!
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) targetFoV = 20.0f;

    // Smoothly move current FoV towards the Target...
    if (FoV < targetFoV)
    {
        FoV += fovSpeed * deltaTime;
        if (FoV > targetFoV) FoV = targetFoV; // Clamp
    }
    else if (FoV > targetFoV)
    {
        FoV -= fovSpeed * deltaTime;
        if (FoV < targetFoV) FoV = targetFoV; // Clamp
    }



    // Task 5.7: construct projection and view matrices
    projectionMatrix = perspective(radians(FoV), 4.0f / 3.0f, 0.1f, 660.0f);
    viewMatrix       = lookAt(
        eyePosition,
        eyePosition + direction,
        up
    );

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}



// ===< Terrain Height Management Functions >=== //
void Camera::loadTerrain(const char* filePath)
{
    ifstream file(filePath);
    if (!file.is_open())
    {
        cerr << "ERROR: Could not open terrain geometry file: " << filePath << endl;
        return;
    }

    int count;
    file >> count; // Read the number of triangles
    terrainTriangles.reserve(count);

    float x1, y1, z1, x2, y2, z2, x3, y3, z3;

    // Loop through all lines in the file
    for (int i = 0; i < count; i++)
    {
        file >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3;

        TerrainTriangle t;
        t.v1 = vec3(x1, y1, z1);
        t.v2 = vec3(x2, y2, z2);
        t.v3 = vec3(x3, y3, z3);

        terrainTriangles.push_back(t);
    }
    cout << "Camera loaded " << terrainTriangles.size() << " terrain triangles." << endl;
}

float Camera::getTerrainHeight(float x, float z)
{
    // Iterate over all triangles to find which one we are standing on...
    for (const auto& tri : terrainTriangles)
    {
        // Barycentric Coordinates Logic
        // Project the 3D triangle onto the 2D (X, Z) plane...
        float det = (tri.v2.z - tri.v3.z) * (tri.v1.x - tri.v3.x) + (tri.v3.x - tri.v2.x) * (tri.v1.z - tri.v3.z);

        if (abs(det) < 0.00001f) continue; // Degenerate triangle

        float l1 = ((tri.v2.z - tri.v3.z) * (x - tri.v3.x) + (tri.v3.x - tri.v2.x) * (z - tri.v3.z)) / det;
        float l2 = ((tri.v3.z - tri.v1.z) * (x - tri.v3.x) + (tri.v1.x - tri.v3.x) * (z - tri.v3.z)) / det;
        float l3 = 1.0f - l1 - l2;

        // Check if the point (x,z) lies inside this triangle...
        if (l1 >= 0.0f && l1 <= 1.0f &&
            l2 >= 0.0f && l2 <= 1.0f &&
            l3 >= 0.0f && l3 <= 1.0f) return l1 * tri.v1.y + l2 * tri.v2.y + l3 * tri.v3.y; // Interpolate the Y value.
    }
}
