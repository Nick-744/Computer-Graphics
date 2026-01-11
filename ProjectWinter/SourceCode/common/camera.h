#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <vector>

using namespace std;

// Store terrain triangle...
struct TerrainTriangle
{
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;
};

class Camera
{
public:
    GLFWwindow* window;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    glm::vec3 position;    // Initial position         : on +Z
    float horizontalAngle; // Initial horizontal angle : toward -Z
    float verticalAngle;   // Initial vertical angle   : none

    // Field of View
    float FoV;
    float speed; // units / second
    float mouseSpeed;
    float fovSpeed;
    
    bool flyingMode = false; // If true, no terrain height adjustment!
    
    // Terrain data - Camera height management!
    vector<TerrainTriangle> terrainTriangles;
    void loadTerrain(const char* filePath);
    float getTerrainHeight(float x, float z);

    Camera(GLFWwindow* window);
    bool update(float deltaTime);
};

#endif
