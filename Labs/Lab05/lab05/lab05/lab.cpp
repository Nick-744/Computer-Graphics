// Include C++ headers
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>

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

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Lab 05"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram;
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation;
GLuint lightLocation;
GLuint diffuseColorSampler, specularColorSampler;
GLuint diffuseTexture, specularTexture;
std::vector<vec3> objVertices, objNormals;
std::vector<vec2> objUVs;
Drawable* triangle;
Drawable* obj;

// ===< Homework 4 >=== //
GLuint materialKa, materialKd, materialKs, materialNs;

// Για αποδοτική διαχείριση των υλικών
struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

vector<Material> materials = {
    { // Chrome
        vec4(0.25f,     0.25f,     0.25f,     1.0f),
        vec4(0.4f,      0.4f,      0.4f,      1.0f),
        vec4(0.774597f, 0.774597f, 0.774597f, 1.0f),
        76.8f
    },
    { // Copper
        vec4(0.19125f,  0.0735f,   0.0225f,   1.0f),
        vec4(0.7038f,   0.27048f,  0.0828f,   1.0f),
        vec4(0.256777f, 0.137622f, 0.086014f, 1.0f),
        12.8f
    },
    { // Emerald
        vec4(0.0215f,  0.1745f,   0.0215f,  0.55f),
        vec4(0.07568f, 0.61424f,  0.07568f, 0.55f),
        vec4(0.633f,   0.727811f, 0.633f,   0.55f),
        76.8f
    }
};

// ===< Homework 5 >=== //
GLuint lightColorLocation;
GLuint light_powerLocation;

vec3 lightPos    = glm::vec3(2.0f, 1.0f, 0.0f);
vec3 lightColor  = glm::vec3(1.0f, 1.0f, 1.0f);
float lightPower = 40.0f;

// ===< Homework 6A >=== //
struct ShaderInfo
{
    GLuint programID;

    GLuint P_Location;
    GLuint V_Location;
    GLuint M_Location;

    GLuint lightPosLocation;

    GLuint diffuseSamplerLocation;
    GLuint specularSamplerLocation;
};

vector<ShaderInfo> shaderList;

ShaderInfo setupShader(const char* vertexPath, const char* fragmentPath) {
    ShaderInfo s;
    s.programID = loadShaders(vertexPath, fragmentPath);

    // Get locations specific to THIS program
    s.P_Location = glGetUniformLocation(s.programID, "P");
    s.V_Location = glGetUniformLocation(s.programID, "V");
    s.M_Location = glGetUniformLocation(s.programID, "M");

    s.lightPosLocation = glGetUniformLocation(s.programID, "light_position_worldspace");

    s.diffuseSamplerLocation  = glGetUniformLocation(s.programID, "diffuseColorSampler");
    s.specularSamplerLocation = glGetUniformLocation(s.programID, "specularColorSampler");

    return s;
}

//#define RENDER_TRIANGLE // Με αυτό μπορούμε εύκολα να επιλέξουμε το τι ζωγραφίζουμε!

void createContext()
{
    // Create and compile our GLSL program from the shaders
    shaderProgram = loadShaders("PhongShading.vertexshader", "PhongShading.fragmentshader");

    // Homework 2: implement Gouraud shading.
    //shaderProgram = loadShaders("GouraudShading.vertexshader", "GouraudShading.fragmentshader");

    // Homework 3: implement flat shading.
    //shaderProgram = loadShaders("FlatShading.vertexshader", "FlatShading.fragmentshader");
    
    // Task 6.2: load diffuse and specular texture maps
    diffuseTexture  = loadSOIL("suzanne_diffuse.bmp");
	specularTexture = loadSOIL("suzanne_specular.bmp"); // Έτσι γυαλίζουν μόνο τα μάτια!

    // Task 6.3: get a pointer to the texture samplers (diffuseColorSampler, specularColorSampler)
	diffuseColorSampler  = glGetUniformLocation(shaderProgram, "diffuseColorSampler");
	specularColorSampler = glGetUniformLocation(shaderProgram, "specularColorSampler");

    // get pointers to the uniform variables
    projectionMatrixLocation = glGetUniformLocation(shaderProgram, "P");
    viewMatrixLocation       = glGetUniformLocation(shaderProgram, "V");
    modelMatrixLocation      = glGetUniformLocation(shaderProgram, "M");
    lightLocation            = glGetUniformLocation(shaderProgram, "light_position_worldspace");


    
    // ===< Homework 4 >=== // Material Uniform Variables Locations
    materialKa = glGetUniformLocation(shaderProgram, "materialKa");
    materialKd = glGetUniformLocation(shaderProgram, "materialKd");
    materialKs = glGetUniformLocation(shaderProgram, "materialKs");
    materialNs = glGetUniformLocation(shaderProgram, "materialNs");



	// ===< Homework 5 >=== //
	lightColorLocation  = glGetUniformLocation(shaderProgram, "lightColor");
    light_powerLocation = glGetUniformLocation(shaderProgram, "light_power");



	// ===< Homework 6A >=== //
    shaderList.push_back(setupShader("FlatShading.vertexshader",    "FlatShading.fragmentshader"));
    shaderList.push_back(setupShader("GouraudShading.vertexshader", "GouraudShading.fragmentshader"));
    shaderList.push_back(setupShader("PhongShading.vertexshader",   "PhongShading.fragmentshader"));



    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // triangle
    // Task 1: visualize a triangle facing towards the +z direction.
    // Task 1.1: define the triangle’s vertex positions
    vector<glm::vec3> triangleVertices = {
        vec3(-1.5f, -1.5f, 0.0f),
        vec3( 0.0f,  1.5f, 0.0f),
        vec3( 1.5f, -1.5f, 0.0f)
    };
    // Add other vertices!
	// triangleVertices.push_back( vec3(...) );

    // Task 1.2: define the triangle’s vertex normals
    vector<vec3> triangleNormals = {
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 0.0f, 1.0f)
	};

    // Task 1.3: construct the triangle as a Drawable
	triangle = new Drawable(triangleVertices, VEC_VEC2_DEFAULT_VALUE, triangleNormals);

    // obj
    // Task 1.4: construct the object (Suzanne) as a Drawable
	obj = new Drawable("suzanne.obj");

    // Homework 7: Load the obj's vertices and UVs using loadOBJWithTiny.
    // Ignore the normals returned by loadOBJWithTiny and try to compute them yourself.
    // loadOBJWithTiny("suzanne.obj", objVertices, objUVs, objNormals);
    // loadOBJWithTiny("earth.obj", objVertices, objUVs, objNormals);
}

void free()
{
    glDeleteTextures(1, &diffuseTexture);
    glDeleteTextures(1, &specularTexture);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void mainLoop()
{
    //glm::vec3 lightPos = glm::vec3(0, 0, 2);

    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // camera
        camera->update();

        // Task 1.5: bind
#ifdef RENDER_TRIANGLE
        // bind triangle
		triangle->bind();
#else
        // bind obj
		obj->bind();
#endif
        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix       = camera->viewMatrix;
        mat4 modelMatrix      = mat4(1.0);

        // transfer uniforms to GPU
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(viewMatrixLocation,       1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(modelMatrixLocation,      1, GL_FALSE, &modelMatrix[0][0]);
        //glUniform3f(lightLocation, camera->position.x, camera->position.y, camera->position.z); // light
		// Βάζοντας ως ορίσματα το position της κάμερας, το φως ακολουθεί την κάμερα, είναι σαν φακό!

		// ===< Homework 5 >=== //
        glUniform3fv(lightLocation,      1, &lightPos[0]);
        glUniform3fv(lightColorLocation, 1, &lightColor[0]);
        glUniform1f(light_powerLocation,    lightPower);

        // Task 6.4: bind textures and transmit the diffuse and specular maps to the GPU
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTexture);
		glUniform1i(diffuseColorSampler, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularTexture);
		glUniform1i(specularColorSampler, 1);

        // Task 1.5: draw
#ifdef RENDER_TRIANGLE
        // draw triangle
		triangle->draw();
#else
        // ===< Homework 4 >=== //
        /*for (int i = 0; i < materials.size(); i++)
        {
            float temp     = 3.0f;
            float x_offset = i * temp - temp;
			modelMatrix    = translate(mat4(1.0f), vec3(x_offset, 0.0f, -3.0f));
			glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

            glUniform4fv(materialKa, 1, &materials[i].ambient[0]);
            glUniform4fv(materialKd, 1, &materials[i].diffuse[0]);
            glUniform4fv(materialKs, 1, &materials[i].specular[0]);
            glUniform1f(materialNs,      materials[i].shininess);

            obj->draw();
        }*/

		// ===< Homework 6A >=== //
        for (int i = 0; i < materials.size(); i++)
        {
            int shaderIndex           = i % shaderList.size();
            ShaderInfo& currentShader = shaderList[shaderIndex];

			// Ενεργοποίηση του κατάλληλου shader
            glUseProgram(currentShader.programID);

            float temp     = 3.0f;
            float x_offset = i * temp - temp;
            mat4 modelMatrix = translate(mat4(1.0f), vec3(x_offset, 0.0f, -3.0f));

            // Send Uniforms!!!

            // Matrices
            glUniformMatrix4fv(currentShader.P_Location, 1, GL_FALSE, &camera->projectionMatrix[0][0]);
            glUniformMatrix4fv(currentShader.V_Location, 1, GL_FALSE, &camera->viewMatrix[0][0]);
            glUniformMatrix4fv(currentShader.M_Location, 1, GL_FALSE, &modelMatrix[0][0]);

            // Light Position
            glUniform3fv(currentShader.lightPosLocation, 1, &lightPos[0]);

            // Textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseTexture);
            glUniform1i(currentShader.diffuseSamplerLocation, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularTexture);
            glUniform1i(currentShader.specularSamplerLocation, 1);

            obj->draw();
        }

        // draw obj
		//obj->draw();
#endif

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT) return; // Register only presses

    const float posStep   = 0.2f;
    const float colorStep = 0.05f;
    const float powerStep = 1.0f;

    // Move light position!
    if (key == GLFW_KEY_U)      lightPos.y += posStep;
    else if (key == GLFW_KEY_O) lightPos.y -= posStep;

    else if (key == GLFW_KEY_J) lightPos.x -= posStep;
    else if (key == GLFW_KEY_L) lightPos.x += posStep;

    else if (key == GLFW_KEY_K) lightPos.z += posStep;
    else if (key == GLFW_KEY_I) lightPos.z -= posStep;

    // Light color
    else if (key == GLFW_KEY_R) lightColor.r = clamp(lightColor.r + colorStep, 0.0f, 1.0f);
    else if (key == GLFW_KEY_T) lightColor.r = clamp(lightColor.r - colorStep, 0.0f, 1.0f);

    else if (key == GLFW_KEY_G) lightColor.g = clamp(lightColor.g + colorStep, 0.0f, 1.0f);
    else if (key == GLFW_KEY_H) lightColor.g = clamp(lightColor.g - colorStep, 0.0f, 1.0f);

    else if (key == GLFW_KEY_B) lightColor.b = clamp(lightColor.b + colorStep, 0.0f, 1.0f);
    else if (key == GLFW_KEY_N) lightColor.b = clamp(lightColor.b - colorStep, 0.0f, 1.0f);

	// Light power
    else if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_EQUAL)
        lightPower += powerStep;
    else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS)
        lightPower = glm::max(0.0f, lightPower - powerStep);
	lightPower = clamp(lightPower, 0.0f, 50.0f);
}

void initialize()
{
    // Initialize GLFW
    if (!glfwInit())
        throw runtime_error("Failed to initialize GLFW\n");

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

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // enable textures
    glEnable(GL_TEXTURE_2D);

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
