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


#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Lab 04"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram;
GLuint VPLocation, MLocation, planeLocation, detachmentCoeffLocation;
GLuint modelVAO, modelVerticiesVBO, planeVAO, planeVerticiesVBO;
std::vector<vec3> modelVertices, modelNormals;
std::vector<vec2> modelUVs;

// Task 5: for the new shader program
GLuint plane2ShaderProgram, plane2MLocation, plane2VPLocation;

// Drawables
Drawable* plane;
Drawable* plane2;

Drawable* model;

// 1st plane parameters
float planeY          = 0.0f;
float planeAngle      = 0.0f;
float detachmentCoeff = 0.0f;

// 2nd plane parameters
float plane2Y          = 0.0f;
float plane2Angle      = 0.0f;
float detachmentCoeff2 = 0.0f;

GLuint plane2Location, detachmentCoeff2Location;

GLuint isPlaneLocation; // So the plane is not discarded by the shader when drawing the model!

void createContext() {
    // Create and compile our GLSL program from the shaders
    shaderProgram = loadShaders("Shader.vertexshader", "Shader.fragmentshader");

    // Draw wire frame triangles or fill: GL_LINE, or GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Get a pointer location to model matrix in the vertex shader
    VPLocation = glGetUniformLocation(shaderProgram, "VP");
    MLocation  = glGetUniformLocation(shaderProgram, "M");

    // Task 3.3d: get uniform location of the plane coefficients
    planeLocation = glGetUniformLocation(shaderProgram, "planeCoeffs");

    // Task 4.1b:
	detachmentCoeffLocation = glGetUniformLocation(shaderProgram, "detachmentDisplacement");
    // Αφού υπάρχει και στους 2 shaders, έχει 2 τιμές!

    // Task 5.2:
    // Create a new shader program to render the plane
    plane2ShaderProgram = loadShaders("plane.vertexshader", "plane.fragmentshader");

    plane2VPLocation = glGetUniformLocation(plane2ShaderProgram, "VP");
    plane2MLocation  = glGetUniformLocation(plane2ShaderProgram, "M");

	plane2Location           = glGetUniformLocation(shaderProgram, "plane2Coeffs");
	detachmentCoeff2Location = glGetUniformLocation(shaderProgram, "detachmentDisplacement2");

    // model
    // Task 1.3 Load a heart model as a Drawable
    model = new Drawable("heart.obj");
    
    // Task 1.1: Construct a plane (x-z) using Drawable
    float size = 2.0f;
    vector<vec3>planeVertices = { // Συντεταγμένες κορυφών του plain
        vec3(-size, planeY, -size),
        vec3(-size, planeY,  size),
        vec3( size, planeY,  size),

        // add a second triangle
        vec3( size, planeY,  size),
		vec3( size, planeY, -size),
		vec3(-size, planeY, -size)
    };
    
    plane = new Drawable(planeVertices);

    // Red plane
    float size2 = 1.5f;
    vector<vec3>plane2Vertices = { // Συντεταγμένες κορυφών του 2ου plain
        vec3(-size2, plane2Y, -size2),
        vec3(-size2, plane2Y,  size2),
        vec3( size2, plane2Y,  size2),

        vec3( size2, plane2Y,  size2),
        vec3( size2, plane2Y, -size2),
        vec3(-size2, plane2Y, -size2)
    };
    
    plane2 = new Drawable(plane2Vertices);

	isPlaneLocation = glGetUniformLocation(shaderProgram, "isPlane");
}
void free() {
    glDeleteBuffers(1, &modelVerticiesVBO);
    glDeleteVertexArrays(1, &modelVAO);

    glDeleteBuffers(1, &planeVerticiesVBO);
    glDeleteVertexArrays(1, &planeVAO);

    glDeleteProgram(shaderProgram);
    glDeleteProgram(plane2ShaderProgram);

    glfwTerminate();
}

void mainLoop() {
    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //camera
        camera->update();
        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix       = camera->viewMatrix;

        // Homework 2 - Create a red second plane
        glUseProgram(plane2ShaderProgram);

		plane2->bind();
		vec3 plane2Position(0, plane2Y, 0);
		mat4 plane2Translation = translate(mat4(), plane2Position);
		mat4 plane2Rotation    = rotate(mat4(), plane2Angle, vec3(0.0f, 0.0f, 1.0f));

		mat4 plane2ModelMatrix = plane2Translation * plane2Rotation;
		mat4 plane2VP          = projectionMatrix * viewMatrix;
		glUniformMatrix4fv(plane2VPLocation, 1, GL_FALSE, &plane2VP[0][0]);
		glUniformMatrix4fv(plane2MLocation,  1, GL_FALSE, &plane2ModelMatrix[0][0]);
		plane2->draw(); // Task 5.3: Render the plane using the new shader program

        glUseProgram(shaderProgram); // Original shader used during the lab...

		vec3 plane2Normal(plane2Rotation * vec4(0, 1, 0, 0));
		float d2 = -dot(plane2Normal, plane2Position);
		vec4 plane2Coeffs(plane2Normal, d2);

		glUniform4f(
			plane2Location,
			plane2Coeffs.x,
			plane2Coeffs.y,
			plane2Coeffs.z,
			plane2Coeffs.w
		);

		vec3 detachmentVec2 = detachmentCoeff2 * plane2Normal;
		glUniform3fv(detachmentCoeff2Location, 1, &detachmentVec2[0]);

		// Task 1.2: Render the plane - Comment out, αλλιώς θα ζωγραφίζει 2 φορές το ίδιο πράγμα!
        /*plane->bind();
        mat4 planeModelMatrix = mat4(1);
        mat4 planeVP          = projectionMatrix * viewMatrix;
		// Στέλνουμε τους πίνακες στον shader! Έχουμε δηλώσει στην createContext το planeShaderProgram...
        glUniformMatrix4fv(VPLocation, 1, GL_FALSE, &planeVP[0][0]);
        glUniformMatrix4fv(MLocation , 1, GL_FALSE, &planeModelMatrix[0][0]);
        plane->draw();*/



        // Task 2: 
        // Task 2.1: translate the plane in + -y direction using the keyboard
        // Task 2.2: and rotate the plane around z-direction

		glUniform1i(isPlaneLocation, 1); // Do not discard the plane in the shader

        plane->bind();
        // Task 2.1: translation, use planeY as variable
        vec3 planePosition(0, planeY, 0);
		mat4 planeTranslation = translate(mat4(), planePosition);

        // Task 2.2: rotation, use planeAngle as variable
		mat4 planeRotation = rotate(mat4(), planeAngle, vec3(0.0f, 0.0f, 1.0f));

		mat4 planeModelMatrix = planeTranslation * planeRotation;
        mat4 planeVP          = projectionMatrix * viewMatrix;
        glUniformMatrix4fv(VPLocation, 1, GL_FALSE, &planeVP[0][0]);
        glUniformMatrix4fv(MLocation,  1, GL_FALSE, &planeModelMatrix[0][0]);
        plane->draw();

        // Task 3.1: calculate plane coefficients
        vec3 planeNormal(planeRotation * vec4(0, 1, 0, 0)); // Σε generalized συντεταγμένες -> w = 0!
        float d = -dot(planeNormal, planePosition);
        vec4 planeCoeffs(planeNormal, d);

        // Task 3.2d:
		glUniform4f(
            planeLocation,
            planeCoeffs.x,
            planeCoeffs.y,
            planeCoeffs.z,
            planeCoeffs.w
        );



        // Task 4.1b: calculate and transmit the detachment offset to the GPU
        vec3 detachmentVec = detachmentCoeff * planeNormal;
        glUniform3fv(detachmentCoeffLocation, 1, &detachmentVec[0]);



        // model
        // Task 1.4: Draw the heart model
        glUniform1i(isPlaneLocation, 0);
        model->bind();
        mat4 modelModelMatrix = mat4(1);
        mat4 modelVP          = projectionMatrix * viewMatrix;
        glUniformMatrix4fv(VPLocation, 1, GL_FALSE, &modelVP[0][0]);
        glUniformMatrix4fv(MLocation,  1, GL_FALSE, &modelModelMatrix[0][0]);
        model->draw();



        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Task 2.1:
    if (key == GLFW_KEY_I)
        planeY += 0.005f; 
    if (key == GLFW_KEY_K)
        planeY -= 0.005f;

    // Task 2.2: planeAngle J, L keys
    if (key == GLFW_KEY_J)
		planeAngle += 0.005f;
	if (key == GLFW_KEY_L)
		planeAngle -= 0.005f;



    // Task 3.4: toggle polygon mode
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode[0]);

        // if GL_LINE, if GL_FILL check with polygonMode[0]
         if (polygonMode[0] == GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         if (polygonMode[0] == GL_FILL) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }



    // Task 4.1a: change the detachment coefficient using U, O keys
    // Use variable: detachmentCoeff
    if (key == GLFW_KEY_U)
        detachmentCoeff += 0.01f;
    if (key == GLFW_KEY_O)
    {
        detachmentCoeff -= 0.01f;
        if (detachmentCoeff <= 0.0f) detachmentCoeff = 0.01f;
    }

	// Plane 2 controls
    if (key == GLFW_KEY_KP_8)
		plane2Y += 0.01f;
	if (key == GLFW_KEY_KP_5)
		plane2Y -= 0.01f;
	if (key == GLFW_KEY_KP_4)
		plane2Angle += 0.01f;
	if (key == GLFW_KEY_KP_6)
		plane2Angle -= 0.01f;

	if (key == GLFW_KEY_KP_7)
		detachmentCoeff2 += 0.01f;
    if (key == GLFW_KEY_KP_9)
    {
        detachmentCoeff2 -= 0.01f;
		if (detachmentCoeff2 <= 0.0f) detachmentCoeff2 = 0.01f;
    }

    // Combined μετατόπιση των τμημάτων!
    if (key == GLFW_KEY_KP_0)
    {
        detachmentCoeff2 += 0.01f;
        detachmentCoeff  += 0.01f;
    }
    if (key == GLFW_KEY_KP_1)
    {
        detachmentCoeff2 -= 0.01f;
        detachmentCoeff  -= 0.01f;
        if (detachmentCoeff2 <= 0.0f) detachmentCoeff2 = 0.01f;
        if (detachmentCoeff <= 0.0f)  detachmentCoeff  = 0.01f;
    }
}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
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
    if (glewInit() != GLEW_OK) {
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

int main(void) {
    try {
        initialize();
        createContext();
        mainLoop();
        free();
    } catch (exception& ex) {
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