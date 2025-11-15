// Include C++ headers
#include <iostream>
#include <string>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/model.h>
#include <common/texture.h>

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#define W_WIDTH 1280
#define W_HEIGHT 720
#define TITLE "Project Winter"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram;
GLuint VPLocation, MLocation;
GLuint modelVAO, modelVerticiesVBO, terrainVAO, terrainVerticiesVBO;
std::vector<vec3> modelVertices, modelNormals;
std::vector<vec2> modelUVs;

// Terrain texture samplers
GLuint textureSamplerWorld;

GLuint textureSamplerSlope;
GLuint textureSamplerSoil;
GLuint textureSamplerCurvature;
GLuint textureSamplerLake;

GLuint textureSamplerRock;
GLuint textureSamplerGrass;
GLuint textureSamplerDirt;
GLuint textureSamplerSand;

// Moving water texture
GLuint timeUniform;
GLuint textureSamplerWater;

// Terrain textures
GLuint textureWorld, textureSlope, textureSoil, textureCurvature, textureLake;
GLuint textureRock, textureGrass, textureDirt, textureSand, textureWater;

// Terrain shader variables
GLuint terrainShaderProgram, terrainMLocation, terrainVPLocation;

// Drawables
Drawable* terrain;
Drawable* model;

void createContext()
{
    // Draw wire frame triangles or fill: GL_LINE, or GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    { // Nothing yet...

        // Create and compile our GLSL program from the shaders
        shaderProgram = loadShaders("Shader.vertexshader", "Shader.fragmentshader");

        // Get a pointer location to model matrix in the vertex shader
        VPLocation = glGetUniformLocation(shaderProgram, "VP");
        MLocation  = glGetUniformLocation(shaderProgram, "M");

    }

	{ // Terrain setup

        // Create a new shader program to render the terrain!
        terrainShaderProgram = loadShaders("terrain.vertexshader", "terrain.fragmentshader");

        terrainVPLocation = glGetUniformLocation(terrainShaderProgram, "VP");
        terrainMLocation  = glGetUniformLocation(terrainShaderProgram, "M");

		// Texture sampler locations in the shader
        textureSamplerWorld = glGetUniformLocation(terrainShaderProgram, "textureSamplerWorld");

        textureSamplerSlope     = glGetUniformLocation(terrainShaderProgram, "textureSamplerSlope");
        textureSamplerSoil      = glGetUniformLocation(terrainShaderProgram, "textureSamplerSoil");
		textureSamplerCurvature = glGetUniformLocation(terrainShaderProgram, "textureSamplerCurvature");
		textureSamplerLake      = glGetUniformLocation(terrainShaderProgram, "textureSamplerLake");

		textureSamplerRock  = glGetUniformLocation(terrainShaderProgram, "textureSamplerRock");
		textureSamplerGrass = glGetUniformLocation(terrainShaderProgram, "textureSamplerGrass");
		textureSamplerDirt  = glGetUniformLocation(terrainShaderProgram, "textureSamplerDirt");
		textureSamplerSand  = glGetUniformLocation(terrainShaderProgram, "textureSamplerSand");

		// Moving water texture
		textureSamplerWater = glGetUniformLocation(terrainShaderProgram, "textureSamplerWater");
        timeUniform         = glGetUniformLocation(terrainShaderProgram, "time");

        // load BMP textures
		textureWorld = loadBMP("assets/worldmap_gaea/worldmap_texture_NO-BLUE.bmp"); // General terrain texture

		// Load terrain attribute textures
		textureSlope     = loadBMP("assets/worldmap_gaea/slope_texture.bmp");
		textureSoil      = loadBMP("assets/worldmap_gaea/soil_texture.bmp");
		textureCurvature = loadBMP("assets/worldmap_gaea/curvature_texture.bmp");
		textureLake      = loadBMP("assets/worldmap_gaea/lake_texture_4k.bmp");

        glBindTexture(GL_TEXTURE_2D, textureSlope);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, textureSoil);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, textureCurvature);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, textureLake);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// For detailed terrain texturing!
		textureRock  = loadBMP("assets/world_textures/rock_face_03_diff_4k.bmp");
		textureGrass = loadBMP("assets/world_textures/brown_mud_leaves_01_diff_4k.bmp");
		textureDirt  = loadBMP("assets/world_textures/dirt_diff_4k.bmp");
		textureSand  = loadBMP("assets/world_textures/damp_sand_diff_4k.bmp");
		textureWater = loadBMP("assets/world_textures/water.bmp");

        // Load the terrain as a Drawable
        terrain = new Drawable("assets/worldmap_gaea/low_poly_worldmap.obj");

    }
}
void free()
{
    glDeleteBuffers(1, &modelVerticiesVBO);
    glDeleteVertexArrays(1, &modelVAO);

    glDeleteBuffers(1, &terrainVerticiesVBO);
    glDeleteVertexArrays(1, &terrainVAO);

    glDeleteProgram(shaderProgram);
	glDeleteProgram(terrainShaderProgram);

    glfwTerminate();
}

void mainLoop()
{
    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //camera
        camera->update();
        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix       = camera->viewMatrix;
        glUseProgram(shaderProgram);

        { // Terrain

			glUseProgram(terrainShaderProgram);

            // Bind textures
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textureWorld);
            glUniform1i(textureSamplerWorld, 0);

			// Bind terrain attribute textures
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, textureSlope);
			glUniform1i(textureSamplerSlope, 1);

			glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, textureSoil);
			glUniform1i(textureSamplerSoil, 2);

			glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, textureCurvature);
			glUniform1i(textureSamplerCurvature, 3);

			glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, textureLake);
            glUniform1i(textureSamplerLake, 4);

			// Bind detailed terrain textures
			glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, textureRock);
			glUniform1i(textureSamplerRock, 5);

			glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, textureGrass);
			glUniform1i(textureSamplerGrass, 6);

			glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, textureDirt);
			glUniform1i(textureSamplerDirt, 7);

			glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, textureSand);
			glUniform1i(textureSamplerSand, 8);

			glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_2D, textureWater);
			glUniform1i(textureSamplerWater, 9);

            // Pass time to shader
            glUniform1f(timeUniform, (float) glfwGetTime() / 20.0);

            terrain->bind();
            mat4 terrainTranslation = mat4();
            mat4 terrainRotation    = mat4();

            mat4 terrainModelMatrix = terrainTranslation * terrainRotation;
            mat4 terrainVP          = projectionMatrix * viewMatrix;
            glUniformMatrix4fv(terrainVPLocation, 1, GL_FALSE, &terrainVP[0][0]);
            glUniformMatrix4fv(terrainMLocation,  1, GL_FALSE, &terrainModelMatrix[0][0]);
            terrain->draw();

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Toggle polygon mode
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode[0]);

        // if GL_LINE, if GL_FILL check with polygonMode[0]
         if (polygonMode[0] == GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         if (polygonMode[0] == GL_FILL) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

void initialize()
{
    // Initialize GLFW
    if (!glfwInit()) throw runtime_error("Failed to initialize GLFW\n");

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
                            " If you have an Intel GPU, they are not 3.3 compatible." +
                            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    glfwSetKeyCallback(window, pollKeyboard);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Task 3.3: blend must be enabled
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);
}

int main(void)
{
    try
    {
        initialize();
        createContext();
        mainLoop();
        free();
    }
    catch (exception& ex)
    {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
