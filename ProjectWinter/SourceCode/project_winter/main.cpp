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
#include <common/light.h> 

// My src files
#include "src/terrain.h"

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

#define W_WIDTH 1280
#define W_HEIGHT 720
#define TITLE "Project Winter"

#define SHADOW_WIDTH 4096
#define SHADOW_HEIGHT 4096



// Creating a structure to store the material parameters of an object
struct Material
{
	vec4 Ka;
	vec4 Kd;
	vec4 Ks;
	float Ns;
};

// Global Variables
GLFWwindow* window;
Camera* camera;

Light* light1;
Light* light2;
int lightController = 1;         // 1 -> light & 2 -> light2
int previousLightController = 0; // Print ONCE the selected light...

GLuint ChampionOfLight;

GLuint shaderProgram, depthProgram;
Drawable* sphere; // Light model helper
GLuint depthFBO1, depthTexture1;
GLuint depthFBO2, depthTexture2;

// locations for shaderProgram
GLuint viewMatrixLocation;
GLuint projectionMatrixLocation;
GLuint modelMatrixLocation;
GLuint KaLocation, KdLocation, KsLocation, NsLocation;

GLuint LaLocation1, LdLocation1, LsLocation1, light1PositionLocation;

GLuint LaLocation2, LdLocation2, LsLocation2, light2PositionLocation;

GLuint lightPowerLocation;
GLuint diffuseColorSampler;
GLuint specularColorSampler;
GLuint useTextureLocation;

GLuint depthMapSampler1;
GLuint light1VPLocation;
GLuint depthMapSampler2;
GLuint light2VPLocation;

// locations for depthProgram
GLuint shadowViewProjectionLocation;
GLuint shadowModelLocation;

// Terrain system
TerrainRenderer* terrainSystem;

// Create sample materials
const Material polishedSilver
{
	vec4{ 0.23125,  0.23125,  0.23125,  1 },
	vec4{ 0.2775,   0.2775,   0.2775,   1 },
	vec4{ 0.773911, 0.773911, 0.773911, 1 },
	89.6f
};

const Material gold
{
	vec4{ 0.24725,  0.1995,   0.0745,   1 },
	vec4{ 0.75164,  0.60648,  0.22648,  1 },
	vec4{ 0.628281, 0.555802, 0.366065, 1 },
	51.2f
};

const Material ruby
{
	vec4{ 0.1745,   0.01175,  0.01175,  0.55 },
	vec4{ 0.61424,  0.04136,  0.04136,  0.55 },
	vec4{ 0.727811, 0.626959, 0.626959, 0.55 },
	76.8
};



// NOTE: Since the Light and Material struct are used in the shader programs as well 
//		 it is recommended to create a function that will update all the parameters 
//       of an object.
// 
// Creating a function to upload (make uniform) the light parameters to the shader program
void uploadLight(const Light& light,
	GLuint LaLocation, GLuint LdLocation, GLuint LsLocation,
	GLuint lightPositionLocation)
{
	glUniform4f(LaLocation, light.La.r, light.La.g, light.La.b, light.La.a);
	glUniform4f(LdLocation, light.Ld.r, light.Ld.g, light.Ld.b, light.Ld.a);
	glUniform4f(LsLocation, light.Ls.r, light.Ls.g, light.Ls.b, light.Ls.a);
	glUniform3f(
		lightPositionLocation,
		light.lightPosition_worldspace.x,
		light.lightPosition_worldspace.y,
		light.lightPosition_worldspace.z
	);
}



// Creating a function to upload the material parameters of a model to the shader program
void uploadMaterial(const Material& mtl)
{
	glUniform4f(KaLocation, mtl.Ka.r, mtl.Ka.g, mtl.Ka.b, mtl.Ka.a);
	glUniform4f(KdLocation, mtl.Kd.r, mtl.Kd.g, mtl.Kd.b, mtl.Kd.a);
	glUniform4f(KsLocation, mtl.Ks.r, mtl.Ks.g, mtl.Ks.b, mtl.Ks.a);
	glUniform1f(NsLocation, mtl.Ns);
}



void createContext()
{
	// Create and compile our GLSL program from the shader
	shaderProgram = loadShaders("shaders/ShadowMapping.vertexshader", "shaders/ShadowMapping.fragmentshader");

	// Task 3.1 
	// Create and load the shader program for the depth buffer construction
	// You need to load and use the Depth.vertexshader, Depth.fragmentshader
	depthProgram = loadShaders("shaders/Depth.vertexshader", "shaders/Depth.fragmentshader");

	// NOTE: Don't forget to delete the shader programs on the free() function



	// Get pointers to uniforms
	// --- shaderProgram ---
	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "P");
	viewMatrixLocation       = glGetUniformLocation(shaderProgram, "V");
	modelMatrixLocation      = glGetUniformLocation(shaderProgram, "M");
	// for phong lighting
	KaLocation = glGetUniformLocation(shaderProgram, "mtl.Ka");
	KdLocation = glGetUniformLocation(shaderProgram, "mtl.Kd");
	KsLocation = glGetUniformLocation(shaderProgram, "mtl.Ks");
	NsLocation = glGetUniformLocation(shaderProgram, "mtl.Ns");

	LaLocation1 = glGetUniformLocation(shaderProgram, "light1.La");
	LdLocation1 = glGetUniformLocation(shaderProgram, "light1.Ld");
	LsLocation1 = glGetUniformLocation(shaderProgram, "light1.Ls");
	light1PositionLocation = glGetUniformLocation(shaderProgram, "light1.lightPosition_worldspace");

	LaLocation2 = glGetUniformLocation(shaderProgram, "light2.La");
	LdLocation2 = glGetUniformLocation(shaderProgram, "light2.Ld");
	LsLocation2 = glGetUniformLocation(shaderProgram, "light2.Ls");
	light2PositionLocation = glGetUniformLocation(shaderProgram, "light2.lightPosition_worldspace");

	ChampionOfLight = glGetUniformLocation(shaderProgram, "ChampionOfLight");

	diffuseColorSampler  = glGetUniformLocation(shaderProgram, "diffuseColorSampler");
	specularColorSampler = glGetUniformLocation(shaderProgram, "specularColorSampler");

	// Task 1.4
	useTextureLocation = glGetUniformLocation(shaderProgram, "useTexture");

	// locations for shadow rendering
	depthMapSampler1 = glGetUniformLocation(shaderProgram, "shadowMapSampler1");
	light1VPLocation = glGetUniformLocation(shaderProgram, "light1VP");

	// ===< HOMEWORK 2 >=== //
	depthMapSampler2 = glGetUniformLocation(shaderProgram, "shadowMapSampler2");
	light2VPLocation = glGetUniformLocation(shaderProgram, "light2VP");

	// --- depthProgram ---
	shadowViewProjectionLocation = glGetUniformLocation(depthProgram, "VP");
	shadowModelLocation          = glGetUniformLocation(depthProgram, "M");



	// Initialize the terrain system
	terrainSystem = new TerrainRenderer(shaderProgram);

	// Loading a model

	// Task 1.2 Load earth.obj using drawable 
	sphere = new Drawable("assets/earth.obj");



	// ---------------------------------------------------------------------------- //
	// -  Task 3.2 Create a depth framebuffer and a texture to store the depthmap - //
	// ---------------------------------------------------------------------------- //
	// Tell opengl to generate a framebuffer
	glGenFramebuffers(1, &depthFBO1);
	// Binding the framebuffer, all changes bellow will affect the binded framebuffer
	// **Don't forget to bind the default framebuffer at the end of initialization
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO1);



	// We need a texture to store the depth image
	glGenTextures(1, &depthTexture1);
	glBindTexture(GL_TEXTURE_2D, depthTexture1);
	// Telling opengl the required information about the texture
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		NULL // Δεν έχουμε εικόνα ακόμα, θα δημιουργηθεί αργότερα!
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Task 4.5 Don't shadow area out of light's viewport
	// Step 1 : (Don't forget to comment out the respective lines above
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Set color to set out of border 
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// Next go to fragment shader and add an iff statement, so if the distance in the z-buffer is equal to 1, 
	// meaning that the fragment is out of the texture border (or further than the far clip plane) 
	// then the shadow value is 0.

	// Task 3.2 Continue
	// Attaching the texture to the framebuffer, so that it will monitor the depth component
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture1, 0);

	// Since the depth buffer is only for the generation of the depth texture, 
	// there is no need to have a color output
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);



	// Framebuffer for light2
	glGenFramebuffers(1, &depthFBO2);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO2);

	glGenTextures(1, &depthTexture2);
	glBindTexture(GL_TEXTURE_2D, depthTexture2);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		SHADOW_WIDTH,
		SHADOW_HEIGHT,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		NULL
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_BORDER);

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// Αν δεν εκτελεστεί, οι περιοχές εκτός του shadow map εμφανίζονται σκοτεινές (σκιά) για το light2!

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture2, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);



	// Finally, we have to always check that our frame buffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		glfwTerminate();
		throw runtime_error("Frame buffer not initialized correctly");
	}

	// Binding the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void free()
{
	// Delete Shader Programs
	glDeleteProgram(shaderProgram);
	glDeleteProgram(depthProgram);

	terrainSystem->~TerrainRenderer();

	glfwTerminate();
}



void depth_pass(mat4 viewMatrix, mat4 projectionMatrix, GLuint fbo)
{
	// Task 3.3

	// Setting viewport to shadow map size
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	// Binding the depth framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Cleaning the framebuffer depth information (stored from the last render)
	glClear(GL_DEPTH_BUFFER_BIT);

	// Selecting the new shader program that will output the depth component
	glUseProgram(depthProgram);

	// sending the view and projection matrix to the shader
	mat4 view_projection = projectionMatrix * viewMatrix;
	glUniformMatrix4fv(shadowViewProjectionLocation, 1, GL_FALSE, &view_projection[0][0]);



	// ---- rendering the scene ---- //

	// For sphere
	mat4 sphereModelMatrix = translate(mat4(), vec3(0.0f, 7.0f, 0.0f)) * scale(mat4(), vec3(0.5f));
	glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &sphereModelMatrix[0][0]);
	sphere->bind();
	sphere->draw();

	// Terrain
	mat4 terrainModelMatrix = terrainSystem->getTerrainModelMatrix();
	glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &terrainModelMatrix[0][0]);
	terrainSystem->getTerrainMesh()->bind();
	terrainSystem->getTerrainMesh()->draw();



	// binding the default framebuffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void lighting_pass(mat4 viewMatrix, mat4 projectionMatrix)
{
	// Step 1: Binding a frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, W_WIDTH, W_HEIGHT);

	// Step 2: Clearing color and depth info
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Step 3: Selecting shader program
	glUseProgram(shaderProgram);

	// Making view and projection matrices uniform to the shader program
	glUniformMatrix4fv(viewMatrixLocation,       1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	glUniform1i(ChampionOfLight, false); // Κανονικά αντικείμενα - Κάνε υπολογισμούς Phong!



	// uploading the light parameters to the shader program
	uploadLight(*light1, LaLocation1, LdLocation1, LsLocation1, light1PositionLocation);

	// Task 4.1 Display shadows on the plane
	// Sending the shadow texture to the shaderProgram
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, depthTexture1);
	glUniform1i(depthMapSampler1, 23);

	// Sending the light View-Projection matrix to the shader program
	mat4 light1VP = light1->lightVP();
	glUniformMatrix4fv(light1VPLocation, 1, GL_FALSE, &light1VP[0][0]);



	uploadLight(*light2, LaLocation2, LdLocation2, LsLocation2, light2PositionLocation);

	glActiveTexture(GL_TEXTURE24);
	glBindTexture(GL_TEXTURE_2D, depthTexture2);
	glUniform1i(depthMapSampler2, 24);

	mat4 light2VP = light2->lightVP();
	glUniformMatrix4fv(light2VPLocation, 1, GL_FALSE, &light2VP[0][0]);



	// ----------------------------------------------------------------- //
	// --------------------- Drawing scene objects --------------------- //	
	// ----------------------------------------------------------------- //

	// Draw terrain!
	float currentTime = (float) glfwGetTime() / 20.0f;
	terrainSystem->draw(viewMatrix, projectionMatrix, currentTime);



	// Remove the texture from model2 and use material instead
	// ** Use bool variable to tell the shader not to use a texture
	// ** Look at if statement in the fragment shader
	uploadMaterial(polishedSilver);
	glUniform1i(useTextureLocation, 0); // Tell shader not to use texture!

	// Task 1.2 - Draw the sphere on the scene
	// Use a scaling of 0.5 across all dimensions and translate it to (-3, 1, -3)
	mat4 sphereModelMatrix = translate(mat4(), vec3(0.0f, 7.0f, 0.0f)) * scale(mat4(), vec3(0.5f));
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphereModelMatrix[0][0]);

	sphere->bind();
	sphere->draw();



	// Light sphere model (visualization helper)
	glUniform1i(ChampionOfLight, 1); // Render with only ambient component!

	mat4 temp = scale(mat4(), vec3(0.1f));

	// Light sphere 1
	mat4 light1SphereModel = translate(mat4(), light1->lightPosition_worldspace) * temp;
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &light1SphereModel[0][0]);

	uploadMaterial(gold);
	glUniform1i(useTextureLocation, 0);

	sphere->bind(); sphere->draw();

	// Light sphere 2
	mat4 light2SphereModel = translate(mat4(), light2->lightPosition_worldspace) * temp;
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &light2SphereModel[0][0]);

	uploadMaterial(ruby);
	glUniform1i(useTextureLocation, 0);

	sphere->bind(); sphere->draw();

	glUniform1d(ChampionOfLight, 0);
}



void mainLoop()
{
	light1->update();
	mat4 light1_proj = light1->projectionMatrix;
	mat4 light1_view = light1->viewMatrix;
	depth_pass(light1_view, light1_proj, depthFBO1); // Create the depth buffer

	light2->update();
	mat4 light2_proj = light2->projectionMatrix;
	mat4 light2_view = light2->viewMatrix;
	depth_pass(light2_view, light2_proj, depthFBO2);

	do
	{
		if      (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) lightController = 1;
		else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) lightController = 2;

		// Print which light is selected (ONCE)!
		if (lightController != previousLightController)
		{
			if      (lightController == 1) printf("Light 1 selected\n");
			else if (lightController == 2) printf("Light 2 selected\n");

			previousLightController = lightController;
		}

		if (lightController == 1) light1->update();
		else                      light2->update();

		mat4 light1_proj = light1->projectionMatrix;
		mat4 light1_view = light1->viewMatrix;
		depth_pass(light1_view, light1_proj, depthFBO1); // Create the depth buffer

		mat4 light2_proj = light2->projectionMatrix;
		mat4 light2_view = light2->viewMatrix;
		depth_pass(light2_view, light2_proj, depthFBO2);

		// Getting camera information
		camera->update();
		mat4 projectionMatrix = camera->projectionMatrix;
		mat4 viewMatrix       = camera->viewMatrix;



		lighting_pass(viewMatrix, projectionMatrix); // Render the scene from camera's perspective



		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}



void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Toggle polygon mode
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
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

	// Sky color
	glClearColor(0.6f, 0.7f, 1.0f, 0.0f);

	glfwSetKeyCallback(window, pollKeyboard);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// enable texturing and bind the depth texture
	glEnable(GL_TEXTURE_2D);

	// Log
	logGLParameters();

	// Create camera
	camera = new Camera(window);

	// Creating a light source
	// Creating a custom light 
	light1 = new Light(
		window,
		vec4{ 1, 1, 1, 1 },
		vec4{ 1, 1, 1, 1 },
		vec4{ 1, 1, 1, 1 },
		vec3{ 0, 15, 0 }
	);

	light2 = new Light(
		window,
		vec4{ 1, 1, 1, 1 },
		vec4{ 1, 1, 1, 1 },
		vec4{ 1, 1, 1, 1 },
		vec3{ 0, 15, 0 }
	);
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
		char temp = getchar();
		free();
		return -1;
	}

	return 0;
}
