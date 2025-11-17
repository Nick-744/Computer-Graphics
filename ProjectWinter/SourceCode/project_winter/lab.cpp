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
GLuint VPLocation, MLocation;
GLuint modelVAO, modelVerticiesVBO, terrainVAO, terrainVerticiesVBO;
std::vector<vec3> modelVertices, modelNormals;
std::vector<vec2> modelUVs;

// ===< Terrain >=== //

// Terrain texture samplers
GLuint textureSamplerWorld;

GLuint textureSamplerSlope;
GLuint textureSamplerSoil;
GLuint textureSamplerPeaks;
GLuint textureSamplerLake;
GLuint textureSamplerRivers;

GLuint textureSamplerRock;
GLuint textureSamplerGrass;
GLuint textureSamplerDirt;
GLuint textureSamplerSand;

// Moving water texture + displacement
GLuint timeUniform;
GLuint textureSamplerWater;

GLuint displacementTexture;
GLuint displacementTextureSampler;

// Terrain textures
GLuint textureWorld, textureSlope, textureSoil, texturePeaks, textureLake, textureRivers;
GLuint textureRock, textureGrass, textureDirt, textureSand, textureWater;

// Terrain shader variables
GLuint terrainShaderProgram, terrainMLocation, terrainVPLocation;

// ===< Clouds >=== //

// Quad for rendering clouds
GLuint quadVAO;
GLuint quadPosVBO, quadUVVBO;

// Clouds textures
GLuint cloudBaseTex, cloudDetailTex;

// Clouds shader variables
GLuint cloudsShaderProgram;

// Drawables
Drawable* terrain;
Drawable* model;

void createContext()
{
    // Draw wire frame triangles or fill: GL_LINE, or GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	{ // Terrain setup

        // Create a new shader program to render the terrain!
        terrainShaderProgram = loadShaders("terrain.vertexshader", "terrain.fragmentshader");

        terrainVPLocation = glGetUniformLocation(terrainShaderProgram, "VP");
        terrainMLocation  = glGetUniformLocation(terrainShaderProgram, "M");

		// Texture sampler locations in the shader
        textureSamplerWorld = glGetUniformLocation(terrainShaderProgram, "textureSamplerWorld");

        textureSamplerSlope  = glGetUniformLocation(terrainShaderProgram, "textureSamplerSlope");
        textureSamplerSoil   = glGetUniformLocation(terrainShaderProgram, "textureSamplerSoil");
        textureSamplerPeaks  = glGetUniformLocation(terrainShaderProgram, "textureSamplerPeaks");
		textureSamplerLake   = glGetUniformLocation(terrainShaderProgram, "textureSamplerLake");
		textureSamplerRivers = glGetUniformLocation(terrainShaderProgram, "textureSamplerRivers");

		textureSamplerRock  = glGetUniformLocation(terrainShaderProgram, "textureSamplerRock");
		textureSamplerGrass = glGetUniformLocation(terrainShaderProgram, "textureSamplerGrass");
		textureSamplerDirt  = glGetUniformLocation(terrainShaderProgram, "textureSamplerDirt");
		textureSamplerSand  = glGetUniformLocation(terrainShaderProgram, "textureSamplerSand");

		// Moving water texture + displacement
		textureSamplerWater = glGetUniformLocation(terrainShaderProgram, "textureSamplerWater");
        timeUniform         = glGetUniformLocation(terrainShaderProgram, "time");

        displacementTexture        = loadBMP("assets/world_textures/gray.bmp");
        displacementTextureSampler = glGetUniformLocation(terrainShaderProgram, "displacementTextureSampler");

        // load BMP textures
		textureWorld = loadBMP("assets/worldmap_gaea/worldmap_texture_NO-BLUE.bmp"); // General terrain texture

		// Load terrain attribute textures
		textureSlope  = loadBMP("assets/worldmap_gaea/slope_texture.bmp");
		textureSoil   = loadBMP("assets/worldmap_gaea/soil_texture.bmp");
		texturePeaks  = loadBMP("assets/worldmap_gaea/peaks_texture.bmp");
		textureLake   = loadBMP("assets/worldmap_gaea/lake_texture.bmp");
		textureRivers = loadBMP("assets/worldmap_gaea/rivers_texture.bmp");

		// They are low resolution textures, so use GL_LINEAR for filtering!
        glBindTexture(GL_TEXTURE_2D, textureSlope);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, textureSoil);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, texturePeaks);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, textureLake);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, textureRivers);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// For detailed terrain texturing!
		textureRock  = loadBMP("assets/world_textures/rock_face_03_diff_4k.bmp");
		textureGrass = loadBMP("assets/world_textures/brown_mud_leaves_01_diff_4k.bmp");
		textureDirt  = loadBMP("assets/world_textures/dirt_diff_4k.bmp");
		textureSand  = loadBMP("assets/world_textures/damp_sand_diff_4k.bmp");
		textureWater = loadBMP("assets/world_textures/water.bmp");

        // Load the terrain as a Drawable
        terrain = new Drawable("assets/worldmap_gaea/super_low_poly_worldmap.obj");

    }

    { // Clouds setup

        cloudsShaderProgram = loadShaders("clouds.vertexshader", "clouds.fragmentshader");

		cloudBaseTex   = loadBMP("assets/clouds_textures/cloud_base.bmp");
		cloudDetailTex = loadBMP("assets/clouds_textures/cloud_detail.bmp");

        glGenVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);

		// Quad data (Rectangle made out of triangles)
        static const GLfloat quadPositions[] = {
            1.0f,  10.0f, -1.0f,
            -1.0f, 10.0f, -1.0f,
            -1.0f, 10.0f,  1.0f,
            
            1.0f,  10.0f, -1.0f,
            -1.0f, 10.0f,  1.0f,
            1.0f,  10.0f,  1.0f
        };

        static const GLfloat quadUVs[] = {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,

            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 1.0f
        };

        // Position VBO
        glGenBuffers(1, &quadPosVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadPosVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadPositions), quadPositions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        // UV VBO
        glGenBuffers(1, &quadUVVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadUVVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadUVs), quadUVs, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

    }
}

void free()
{
    glDeleteBuffers(1, &modelVerticiesVBO);
    glDeleteVertexArrays(1, &modelVAO);

    glDeleteBuffers(1, &terrainVerticiesVBO);
    glDeleteVertexArrays(1, &terrainVAO);

    glDeleteProgram(cloudsShaderProgram);
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

        { // Sky

            glUseProgram(cloudsShaderProgram);

            mat4 cloudModel = mat4(1.0f); // positioned by vertex data
            mat4 cloudMVP = projectionMatrix * viewMatrix * cloudModel;

            glUniformMatrix4fv(glGetUniformLocation(cloudsShaderProgram, "MVP"),
                1, GL_FALSE, &cloudMVP[0][0]);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cloudBaseTex);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, cloudDetailTex);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

        }

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

			glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, texturePeaks);
			glUniform1i(textureSamplerPeaks, 3);

			glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, textureLake);
            glUniform1i(textureSamplerLake, 4);

			glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, textureRivers);
			glUniform1i(textureSamplerRivers, 5);

			// Bind detailed terrain textures
			glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, textureRock);
			glUniform1i(textureSamplerRock, 6);

			glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, textureGrass);
			glUniform1i(textureSamplerGrass, 7);

			glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, textureDirt);
			glUniform1i(textureSamplerDirt, 8);

			glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_2D, textureSand);
			glUniform1i(textureSamplerSand, 9);

			glActiveTexture(GL_TEXTURE10); glBindTexture(GL_TEXTURE_2D, textureWater);
			glUniform1i(textureSamplerWater, 10);

			// Bind displacement texture
			glActiveTexture(GL_TEXTURE11); glBindTexture(GL_TEXTURE_2D, displacementTexture);
			glUniform1i(displacementTextureSampler, 11);

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
