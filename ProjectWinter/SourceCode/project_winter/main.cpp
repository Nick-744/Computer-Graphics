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
#include "src/clouds.h"
#include "src/forest.h"
#include "src/meadow.h"
#include "src/cabin.h"
#include "src/boat.h"
#include "src/marina.h"
#include "src/snowSource.h"
#include "src/snowfall.h"
#include "src/lakeReflection.h"
#include "src/timer.h"
#include "src/menuGUI.h" // ImGui backbone
#include "miniaudio.h"   // Audio

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

#define TITLE "Project Winter"
int W_WIDTH  = 1280;
int W_HEIGHT = 720;

int currentShadowBufferSize     = 8192;
int currentSnowBufferSize       = 8192;
int currentReflectionBufferSize =  512;

#define FAR_PLANE_INITIAL 660.0f
#define PLAYER_RADIUS 0.3f // Approximate the player as a sphere!
#define BOAT_RADIUS 1.2f   // Approximate the boat as a bigger sphere!



// Creating a structure to store the material parameters of an object
struct Material
{
	vec4 Ka;
	vec4 Kd;
	vec4 Ks;
	float Ns;
};

struct MainShader // Shadow Mapping Shader...
{
	GLuint programID; // The shader's program ID

	GLuint timeLocation;

	// --- Matrices --- //
	GLuint viewMatrixLocation;
	GLuint projectionMatrixLocation;
	GLuint modelMatrixLocation;

	// --- Material --- //
	GLuint KaLocation, KdLocation, KsLocation, NsLocation;

	// --- Light1 --- //
	GLuint LaLocation1, LdLocation1, LsLocation1;
	GLuint light1PositionLocation;
	GLuint light1VPLocation;

	// --- Textures & Rendering Options --- //
	GLuint diffuseColorSampler;
	GLuint specularColorSampler;
	GLuint useTextureLocation;
	GLuint ChampionOfLight; // Render with only ambient component!

	// --- Shadow Mapping --- //
	GLuint depthMapSampler1;

	// --- Snow System --- //
	GLuint snowDepthMapSampler;
	GLuint snowPositionLocation;
	GLuint snowVPLocation;
	// Gaea snow mask!
	GLuint textureSamplerSnowMask;
	GLuint textureSnowMask;
	// Snow state
	GLuint snowAmountLocation;
	GLuint snowInflateLocation;
	GLuint skipSnowLocation;

	// --- Lake Reflection --- //
	GLuint reflectionTextureSamplerLocation;
	GLuint useReflectionLocation;

	// --- Fog --- //
	GLuint fogDensityLocation;
	GLuint fogColorLocation;

	void initialize()
	{
		programID = loadShaders("shaders/ShadowMapping.vertexshader", "shaders/ShadowMapping.fragmentshader");

		timeLocation = glGetUniformLocation(programID, "time");

		// Matrices
		viewMatrixLocation       = glGetUniformLocation(programID, "V");
		projectionMatrixLocation = glGetUniformLocation(programID, "P");
		modelMatrixLocation      = glGetUniformLocation(programID, "M");

		// --- For phong lighting --- //

		// Material
		KaLocation = glGetUniformLocation(programID, "mtl.Ka");
		KdLocation = glGetUniformLocation(programID, "mtl.Kd");
		KsLocation = glGetUniformLocation(programID, "mtl.Ks");
		NsLocation = glGetUniformLocation(programID, "mtl.Ns");

		// Light 1
		LaLocation1 = glGetUniformLocation(programID, "light1.La");
		LdLocation1 = glGetUniformLocation(programID, "light1.Ld");
		LsLocation1 = glGetUniformLocation(programID, "light1.Ls");
		light1PositionLocation = glGetUniformLocation(programID, "light1.lightPosition_worldspace");
		light1VPLocation       = glGetUniformLocation(programID, "light1VP"); // For shadow rendering

		ChampionOfLight      = glGetUniformLocation(programID, "ChampionOfLight");
		diffuseColorSampler  = glGetUniformLocation(programID, "diffuseColorSampler");
		specularColorSampler = glGetUniformLocation(programID, "specularColorSampler");
		useTextureLocation   = glGetUniformLocation(programID, "useTexture"); // Task 1.4

		// Locations for shadow rendering
		depthMapSampler1 = glGetUniformLocation(programID, "shadowMapSampler1");

		// ===< Snow >=== //
		snowDepthMapSampler  = glGetUniformLocation(programID, "snowDepthMapSampler");
		snowPositionLocation = glGetUniformLocation(programID, "snowPosition_worldspace");
		snowVPLocation       = glGetUniformLocation(programID, "snowVP");
		// Gaea snow mask!
		textureSamplerSnowMask = glGetUniformLocation(programID, "textureSamplerSnowMask");
		textureSnowMask        = loadBMP("assets/worldmap_gaea/snow_mask.bmp");
		// Snow state
		snowAmountLocation  = glGetUniformLocation(programID, "snowAmount");
		snowInflateLocation = glGetUniformLocation(programID, "snowInflate");
		skipSnowLocation    = glGetUniformLocation(programID, "skipSnow");

		// Lake reflection
		reflectionTextureSamplerLocation = glGetUniformLocation(programID, "reflectionTextureSampler");
		useReflectionLocation            = glGetUniformLocation(programID, "useReflection");

		// Fog
		fogDensityLocation = glGetUniformLocation(programID, "fogDensity");
		fogColorLocation   = glGetUniformLocation(programID, "fogColor");
	}

	void useProgram() { glUseProgram(programID); }
};

struct SkyShader
{
	GLuint programID;
	GLuint VPLocation;
	GLuint MLocation;
	GLuint hdriSampler;

	void initialize()
	{
		programID   = loadShaders("shaders/skydome.vertexshader", "shaders/skydome.fragmentshader");
		VPLocation  = glGetUniformLocation(programID, "VP");
		MLocation   = glGetUniformLocation(programID, "M");
		hdriSampler = glGetUniformLocation(programID, "hdriTexture");
	}
};

extern ma_decoding_backend_vtable g_ma_decoding_backend_vtable_stb_vorbis;
struct SoundManager
{
	ma_engine engine;
	ma_resource_manager resourceManager; // Keep the manager alive!
	bool initialized = false;

	void init()
	{
		// Define the backend list
		ma_decoding_backend_vtable* pBackends[] = { &g_ma_decoding_backend_vtable_stb_vorbis };

		// Initialize the Resource Manager first with OGG support...
		ma_resource_manager_config rmConfig     = ma_resource_manager_config_init();
		rmConfig.ppCustomDecodingBackendVTables = pBackends;
		rmConfig.customDecodingBackendCount     = 1;

		if (ma_resource_manager_init(&rmConfig, &resourceManager) != MA_SUCCESS)
		{
			cout << "Failed to init Resource Manager!" << endl;
			return;
		}

		// Initialize the Engine using the Resource Manager
		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pResourceManager = &resourceManager;

		if (ma_engine_init(&engineConfig, &engine) == MA_SUCCESS)
		{
			initialized = true;
			cout << "Audio Engine Initialized with OGG Support!" << endl;
		}
		else
		{
			cout << "Engine Init Failed!" << endl;
			ma_resource_manager_uninit(&resourceManager);
		}
	}

	void play(const char* filepath, bool loop = false)
	{
		if (!initialized) return;
		ma_engine_play_sound(&engine, filepath, NULL);
	}

	void cleanup()
	{
		if (initialized)
		{
			ma_engine_uninit(&engine);
			ma_resource_manager_uninit(&resourceManager); // Cleanup both
		}
	}
};

SoundManager soundSystem;



// Global Variables
GLFWwindow* window;
int windowedPosX, windowedPosY;
int windowedWidth  = W_WIDTH;
int windowedHeight = W_HEIGHT;

Camera* camera;
mat4 cameraProjectionMatrix;
mat4 cameraViewMatrix;
float cameraFarPlane = FAR_PLANE_INITIAL; // Optimization for shadow mapping...

// ---< Player interactions >--- //
bool isBoatClose;
bool isDoorClose;
MenuGUI myMenu;

float lastFrameTime = 0.0f;
float terrainTime   = 0.0f; // For terrain's animation control!
float cloudTime     = 0.0f; // For cloud movement!

float simulatedFrameTime = 0.0f;

// Light
Light* light1;
Drawable* sphere; // Light model helper

// Sky & fog colors
vec3 skyColor        = vec3(0.53f, 0.58f, 0.68f); // From HDRI texture...
vec3 currentSkyColor = skyColor; // Or currentFogColor!
vec3 snowFogColor    = vec3(0.8f, 0.85f, 0.9f);
float fogDensity     = 0.0f;
bool forceClearFog   = false;

SkyShader skyProgram;
GLuint skyTexture;

int windPower = 1;

// --- shaderProgram --- //
MainShader shaderProgram;

// --- depthProgram --- //
GLuint depthProgram;
GLuint depthFBO1 = 0, depthTexture1 = 0;
GLuint shadowViewProjectionLocation;
GLuint shadowModelLocation;
GLuint depthTextureSamplerLocation;
GLuint depthUseTransparentTexLocation;
// For meadow's shadow wind animation
GLuint depthTimeLocation;
GLuint depthWindPowerLocation;

// --- promptProgram --- //
GLuint promptProgram; // Shader program
GLuint quadTextureSamplerLocation;
GLuint promptTexture;
Drawable* quad;



// =====< Systems >===== //

// Terrain system
TerrainRenderer* terrainSystem;

// Cabin model
Cabin* cabinModel;

// Boat model
Boat* boatModel;

// Marine model
Marina* marinaModel;

// Forest system
Forest* forestSystem;
Forest* forestSystem2;

// Meadow system
Meadow* meadowSystem;

// Cloud system
CloudRenderer* cloudSystem;

// ===< Snow System >=== //
SnowSource* snowSource;
// Snow state (CPU)
bool  snowingActive = false;
bool  isLakeFrozen  = false;
float snowAmount    = 0.0f;
float snowInflate   = 0.0f;
Timer snowStartTimer;
// Snow particles
Snowfall* snowfallSystem;

// Lake reflection system
LakeReflection* lakeReflection;
const float WATER_HEIGHT = 58.1f; // Trial and error...



// Create sample materials
const Material gold
{
	vec4{ 0.24725,  0.1995,   0.0745,   1 },
	vec4{ 0.75164,  0.60648,  0.22648,  1 },
	vec4{ 0.628281, 0.555802, 0.366065, 1 },
	51.2f
};

// For testing new models positioning!
vec3 tempPosition       = vec3(0.0f, 59.0f, 0.0f);
float tempRotationAngle = 0.0f;
mat4 tempModelMatrix    = translate(mat4(), tempPosition)
                        * rotate(mat4(), tempRotationAngle, vec3(0, 1, 0));



void renderSky(mat4 viewMatrix, mat4 projectionMatrix)
{
	glDepthMask(GL_FALSE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glUseProgram(skyProgram.programID);

	// Remove translation
	mat4 skyView = mat4(mat3(viewMatrix));
	mat4 VP      = projectionMatrix * skyView;

	mat4 translationMatrix = translate(mat4(), vec3(0, 0.06, 0));
	mat4 rotationMatrixY   = rotate(mat4(), radians(-85.0f), vec3(0, 1, 0));
	mat4 rotationMatrixX   = rotate(mat4(), radians(180.0f), vec3(1, 0, 0));
	mat4 modelMatrix       = translationMatrix * rotationMatrixY * rotationMatrixX;

	glUniformMatrix4fv(skyProgram.VPLocation, 1, GL_FALSE, &VP[0][0]);
	glUniformMatrix4fv(skyProgram.MLocation, 1, GL_FALSE, &modelMatrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skyTexture);
	glUniform1i(skyProgram.hdriSampler, 0);

	sphere->bind(); sphere->draw();

	// Restore state
	glCullFace(GL_BACK);
	glDepthMask(GL_TRUE);
}



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
	glUniform4f(shaderProgram.KaLocation, mtl.Ka.r, mtl.Ka.g, mtl.Ka.b, mtl.Ka.a);
	glUniform4f(shaderProgram.KdLocation, mtl.Kd.r, mtl.Kd.g, mtl.Kd.b, mtl.Kd.a);
	glUniform4f(shaderProgram.KsLocation, mtl.Ks.r, mtl.Ks.g, mtl.Ks.b, mtl.Ks.a);
	glUniform1f(shaderProgram.NsLocation, mtl.Ns);
}



// ---------------------------------------------------------------------------- //
// -  Task 3.2 Create a depth framebuffer and a texture to store the depthmap - //
// ---------------------------------------------------------------------------- //
void initDepthFBO(GLuint& fboID, GLuint& textureID, int& currentSizeVariable, int size)
{
	// Cleanup existing buffers if they exist...
	if (fboID != 0)     glDeleteFramebuffers(1, &fboID);
	if (textureID != 0) glDeleteTextures(1, &textureID);

	currentSizeVariable = size; // Update current size variable

	// Tell opengl to generate a framebuffer
	glGenFramebuffers(1, &fboID);
	// Binding the framebuffer, all changes bellow will affect the binded framebuffer
	// **Don't forget to bind the default framebuffer at the end of initialization
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	// We need a texture to store the depth image
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Telling opengl the required information about the texture
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		size, size,
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
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

	// Since the depth buffer is only for the generation of the depth texture, 
	// there is no need to have a color output
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
}



void createContext()
{
	// Create and compile our GLSL program from the shader
	shaderProgram.initialize();

	// Task 3.1 
	// Create and load the shader program for the depth buffer construction
	// You need to load and use the Depth.vertexshader, Depth.fragmentshader
	depthProgram = loadShaders("shaders/Depth.vertexshader", "shaders/Depth.fragmentshader");

	// Prompt program for texture rendering on a quad (2D)
	promptProgram = loadShaders("shaders/prompt.vertexshader", "shaders/prompt.fragmentshader");

	// NOTE: Don't forget to delete the shader programs on the free() function

	

	// --- depthProgram --- //
	shadowViewProjectionLocation = glGetUniformLocation(depthProgram, "VP");
	shadowModelLocation          = glGetUniformLocation(depthProgram, "M");

	depthTextureSamplerLocation    = glGetUniformLocation(depthProgram, "textureSampler");
	depthUseTransparentTexLocation = glGetUniformLocation(depthProgram, "useTransparentTex");
	// For meadow's shadow wind animation
	depthTimeLocation      = glGetUniformLocation(depthProgram, "time");
	depthWindPowerLocation = glGetUniformLocation(depthProgram, "windPower");

	// --- promptProgram --- //
	quadTextureSamplerLocation = glGetUniformLocation(promptProgram, "textureSampler");



	// =====< Systems Initialization >===== //

	// Terrain
	terrainSystem = new TerrainRenderer(shaderProgram.programID);

	// Cabin
	cabinModel = new Cabin(shaderProgram.programID);

	// Boat
	boatModel = new Boat(shaderProgram.programID, window);

	// Marina
	marinaModel = new Marina(
		shaderProgram.programID,
		vec3(-58.2f, 58.7f, 4.5f),
		1.6f
	);

	// Forests
	forestSystem = new Forest(shaderProgram.programID);
	forestSystem->setPosition(vec3(26.5f, 60.2f, 30.5f));
	forestSystem->setRotation(-0.5f);

	forestSystem2 = new Forest(shaderProgram.programID);
	forestSystem2->setPosition(vec3(-66.5f, 60.7f, 43.0f));

	// Meadow
	meadowSystem = new Meadow(shaderProgram.programID);

	// ===< Skydome >=== //
	skyProgram.initialize();
	skyTexture = loadBMP("assets/skydome.bmp");

	// Clouds
	cloudSystem = new CloudRenderer();

	// ===< Snow >=== // ===< Particles >=== //
	snowfallSystem = new Snowfall(
		3000, // max particles
		2.0f, // min fall speed
		5.0f  // max fall speed
	);

	// Lake reflection
	lakeReflection = new LakeReflection(currentReflectionBufferSize);
	lakeReflection->initialize();



	// ---< Loading a model >--- //

	// Task 1.2 Load earth.obj using drawable 
	sphere = new Drawable("assets/earth.obj"); // Sun!!!



	// Creating a 2D quad to visualize the prompt!
	// create geometry and vao for screen-space quad
	vector<vec3> quadVertices = {
		// Triangle 1
		vec3(-0.15f, -0.2f, 0.0f),
		vec3( 0.15f, -0.2f, 0.0f),
		vec3( 0.15f,  0.2f, 0.0f),

		// Triangle 2
		vec3( 0.15f,  0.2f, 0.0f),
		vec3(-0.15f,  0.2f, 0.0f),
		vec3(-0.15f, -0.2f, 0.0f)
	};

	vector<vec2> quadUVs = {
	  vec2(0.0, 0.0),
	  vec2(1.0, 0.0),
	  vec2(1.0, 1.0),
	  vec2(1.0, 1.0),
	  vec2(0.0, 1.0),
	  vec2(0.0, 0.0)
	};

	quad          = new Drawable(quadVertices, quadUVs);
	promptTexture = loadBMP("assets/prompt_text.bmp");



	// Initialize Shadow FBO
	initDepthFBO(
		depthFBO1,
		depthTexture1,
		currentShadowBufferSize,
		currentShadowBufferSize
	);

	// Initialize Snow FBO
	initDepthFBO(
		snowSource->snowDepthFBO,
		snowSource->snowDepthTexture,
		currentSnowBufferSize,
		currentSnowBufferSize
	);



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
	myMenu.shutdown();

	// Delete Shader Programs
	glDeleteProgram(shaderProgram.programID);
	glDeleteProgram(depthProgram);
	glDeleteProgram(promptProgram);

	delete terrainSystem;
	delete cabinModel;
	delete forestSystem;
	delete forestSystem2;	
	delete meadowSystem;
	delete cloudSystem;
	delete snowfallSystem;
	delete lakeReflection;
	delete sphere;

	soundSystem.cleanup();

	glfwTerminate();
}



void depth_pass(mat4 viewMatrix, mat4 projectionMatrix, GLuint fbo, int buffer_size)
{
	// Task 3.3

	// Setting viewport to shadow map size
	glViewport(0, 0, buffer_size, buffer_size);

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

	// Terrain
	terrainSystem->drawOnlyObjects(shadowModelLocation, light1->frustumPlanes);

	// Cabin
	cabinModel->drawOnlyObjects(shadowModelLocation);

	// Boat
	if (cameraFarPlane < FAR_PLANE_INITIAL) // Optimization for shadow mapping
		boatModel->drawOnlyObjects(shadowModelLocation);

	// Marina
	marinaModel->drawOnlyObjects(shadowModelLocation);

	// Forests
	glUniform1f(depthTimeLocation, simulatedFrameTime); // For wind animation!
	forestSystem->drawOnlyObjects(shadowModelLocation, windPower);
	forestSystem2->drawOnlyObjects(shadowModelLocation, windPower);

	// Meadow
	meadowSystem->drawOnlyObjects(shadowModelLocation, windPower + 1);



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

	if (!(fogDensity > 0.0f)) renderSky(viewMatrix, projectionMatrix);

	// Step 3: Selecting shader program
	shaderProgram.useProgram();



	// ===< Fog >=== //
	// Fog is gradualy applied based on snow amount!
	// Blend so the sky color doesn't have to change...
	glUniform3f(
		shaderProgram.fogColorLocation,
		currentSkyColor.x,
		currentSkyColor.y,
		currentSkyColor.z
	);
	glUniform1f(shaderProgram.fogDensityLocation, fogDensity);



	// Making view and projection matrices uniform to the shader program
	glUniformMatrix4fv(shaderProgram.viewMatrixLocation,       1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(shaderProgram.projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	glUniform1i(shaderProgram.ChampionOfLight, 0); // Κανονικά αντικείμενα - Κάνε υπολογισμούς Phong!



	// Uploading the >-- light --< parameters to the shader program
	uploadLight(
		*light1,
		shaderProgram.LaLocation1,
		shaderProgram.LdLocation1,
		shaderProgram.LsLocation1,
		shaderProgram.light1PositionLocation
	);

	// Task 4.1 Display shadows on the plane
	// Sending the shadow texture to the shaderProgram
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, depthTexture1);
	glUniform1i(shaderProgram.depthMapSampler1, 23);

	// Sending the light View-Projection matrix to the shader program
	mat4 light1VP = light1->lightVP();
	glUniformMatrix4fv(shaderProgram.light1VPLocation, 1, GL_FALSE, &light1VP[0][0]);



	// ===< Snow >=== //
	glUniform3f(
		shaderProgram.snowPositionLocation,
		snowSource->snowSourcePosition_worldspace.x,
		snowSource->snowSourcePosition_worldspace.y,
		snowSource->snowSourcePosition_worldspace.z
	);
	glUniform1f(shaderProgram.snowAmountLocation, snowAmount);
	glUniform1f(shaderProgram.snowInflateLocation, snowInflate);

	glActiveTexture(GL_TEXTURE25);
	glBindTexture(GL_TEXTURE_2D, snowSource->snowDepthTexture);
	glUniform1i(shaderProgram.snowDepthMapSampler, 25);

	glActiveTexture(GL_TEXTURE26);
	glBindTexture(GL_TEXTURE_2D, shaderProgram.textureSnowMask);
	glUniform1i(shaderProgram.textureSamplerSnowMask, 26);

	mat4 snowVP = snowSource->snowVP();
	glUniformMatrix4fv(shaderProgram.snowVPLocation, 1, GL_FALSE, &snowVP[0][0]);



	// Lake Reflection
	glActiveTexture(GL_TEXTURE27);
	glBindTexture(GL_TEXTURE_2D, lakeReflection->getReflectionTexture());
	glUniform1i(shaderProgram.reflectionTextureSamplerLocation, 27);
	glUniform1i(shaderProgram.useReflectionLocation, 1); // Enable reflection...



	// ----------------------------------------------------------------- //
	// --------------------- Drawing scene objects --------------------- //	
	// ----------------------------------------------------------------- //

	// Draw terrain!
	terrainSystem->draw(cameraProjectionMatrix * cameraViewMatrix, light1->frustumPlanes, terrainTime);

	// Remove the texture from terrain and use material instead!
	// ** Use bool variable to tell the shader not to use a texture
	// ** Look at if statement in the fragment shader
	//uploadMaterial(gold); glUniform1i(useTextureLocation, 0); // Not used anymore!

	// Draw the cabin
	cabinModel->draw();

	// Draw the marina
	marinaModel->draw();

	// Draw forests
	glUniform1f(shaderProgram.timeLocation, simulatedFrameTime); // For wind animation!
	forestSystem->draw(windPower);
	forestSystem2->draw(windPower);

	// Draw meadow
	meadowSystem->draw(windPower + 1);



	// Sun
	glUniform1i(shaderProgram.ChampionOfLight, 1);

	mat4 light1SphereModel = translate(mat4(), light1->lightPosition_worldspace) * scale(mat4(), vec3(4.0f));
	glUniformMatrix4fv(shaderProgram.modelMatrixLocation, 1, GL_FALSE, &light1SphereModel[0][0]);

	uploadMaterial(gold);
	glUniform1i(shaderProgram.useTextureLocation, 0);
	sphere->bind(); sphere->draw();

	glUniform1i(shaderProgram.ChampionOfLight, 0); // End of sun rendering



	// Draw the boat - Always on Front trick (draw last)!
	glUniform1i(shaderProgram.skipSnowLocation, 1); // Skip snow on boat!
	boatModel->draw();
	glUniform1i(shaderProgram.skipSnowLocation, 0); // Resume snow on other objects!



	// ======< CLOUDS >====== //
	if (!forceClearFog)
	{
		GLboolean cull = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);

		cloudSystem->draw(
			viewMatrix,
			projectionMatrix,
			cloudTime,
			currentSkyColor,
			fogDensity
		);
		// Remember to change the parameters in reflection pass too...

		if (cull) glEnable(GL_CULL_FACE);
	}
}



void reflection_pass(mat4 viewMatrix, mat4 projectionMatrix)
{
	// Calculate mirrored view matrix!
	mat4 mirroredView = lakeReflection->getMirroredViewMatrix(viewMatrix, WATER_HEIGHT);

	// Begin reflection rendering
	lakeReflection->beginReflectionPass();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	shaderProgram.useProgram(); // Use main shader

	// Upload mirrored matrices
	glUniformMatrix4fv(shaderProgram.viewMatrixLocation,       1, GL_FALSE, &mirroredView[0][0]);
	glUniformMatrix4fv(shaderProgram.projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	// Disable reflection rendering in the reflection pass (avoid recursion)
	glUniform1i(shaderProgram.useReflectionLocation, 0);



	// Upload lighting (same as normal pass!)
	uploadLight(
		*light1,
		shaderProgram.LaLocation1,
		shaderProgram.LdLocation1,
		shaderProgram.LsLocation1,
		shaderProgram.light1PositionLocation
	);

	// Shadow maps
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, depthTexture1);
	glUniform1i(shaderProgram.depthMapSampler1, 23);

	mat4 light1VP = light1->lightVP();
	glUniformMatrix4fv(shaderProgram.light1VPLocation, 1, GL_FALSE, &light1VP[0][0]);



	// Snow (keep same for reflection)
	glUniform3f(shaderProgram.snowPositionLocation,
		snowSource->snowSourcePosition_worldspace.x,
		snowSource->snowSourcePosition_worldspace.y,
		snowSource->snowSourcePosition_worldspace.z
	);
	glUniform1f(shaderProgram.snowAmountLocation, snowAmount);
	glUniform1f(shaderProgram.snowInflateLocation, snowInflate);

	glActiveTexture(GL_TEXTURE25);
	glBindTexture(GL_TEXTURE_2D, snowSource->snowDepthTexture);
	glUniform1i(shaderProgram.snowDepthMapSampler, 25);

	glActiveTexture(GL_TEXTURE26);
	glBindTexture(GL_TEXTURE_2D, shaderProgram.textureSnowMask);
	glUniform1i(shaderProgram.textureSamplerSnowMask, 26);

	mat4 snowVP = snowSource->snowVP();
	glUniformMatrix4fv(shaderProgram.snowVPLocation, 1, GL_FALSE, &snowVP[0][0]);

	glCullFace(GL_FRONT); // Flip culling for mirrored rendering



	// ===< Render scene objects (reflected) >=== //

	// Draw terrain
	terrainSystem->draw(projectionMatrix * mirroredView, light1->frustumPlanes, terrainTime, false);

	// Draw cabin
	cabinModel->draw();

	// Draw marina
	marinaModel->draw();

	// Draw forests
	glUniform1f(shaderProgram.timeLocation, simulatedFrameTime); // For wind animation!
	forestSystem->draw(windPower);
	forestSystem2->draw(windPower);

	// Draw meadow
	meadowSystem->draw(windPower + 1);

	// Draw boat - Always on Front trick (draw last)!
	glUniform1i(shaderProgram.skipSnowLocation, 1); // Skip snow on boat!
	boatModel->draw();
	glUniform1i(shaderProgram.skipSnowLocation, 0); // Resume snow on other objects!

	// Draw clouds
	if (!forceClearFog)
	{
		GLboolean cull = glIsEnabled(GL_CULL_FACE); glDisable(GL_CULL_FACE);
		cloudSystem->draw(
			mirroredView,
			projectionMatrix,
			cloudTime,
			currentSkyColor,
			fogDensity
		);
		if (cull) glEnable(GL_CULL_FACE);
	}



	// End reflection pass
	glCullFace(GL_BACK); // Restore culling
	lakeReflection->endReflectionPass();
}



void renderPrompt()
{
	glDisable(GL_DEPTH_TEST); // Disable depth test so quad is always on top

	// Using the correct shaders to visualize the depth texture on the quad
	glUseProgram(promptProgram);

	//Enabling the texture - follow the aforementioned pipeline
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, promptTexture);
	glUniform1i(quadTextureSamplerLocation, 0);

	// Drawing the quad
	quad->bind(); quad->draw();

	glEnable(GL_DEPTH_TEST);
}



void mainLoop()
{
	// ===< Snow >=== // ===< Depth Pass >=== //
	snowSource->update();
	//snowSource->fitToCameraFrustum(viewMatrix, projectionMatrix); // Not working with snow...
	// After all, the snow source depth map doesn't need to be update in every frame...
	// Also, fix the moving snow artifacts (because of wind animation)!
	mat4 snow_proj = snowSource->projectionMatrix;
	mat4 snow_view = snowSource->viewMatrix;
	depth_pass(snow_view, snow_proj, snowSource->snowDepthFBO, currentSnowBufferSize);

	do
	{
		// Real Time Control
		float currentFrameTime = glfwGetTime();
		float deltaTime        = currentFrameTime - lastFrameTime;
		lastFrameTime          = currentFrameTime;

		myMenu.render(window,
			currentShadowBufferSize, currentSnowBufferSize, currentReflectionBufferSize,
			myMenu.isFullscreen,
			[&](int newSize) { initDepthFBO(depthFBO1, depthTexture1, currentShadowBufferSize, newSize); },
			[&](int newSize)
			{
				initDepthFBO(snowSource->snowDepthFBO, snowSource->snowDepthTexture, currentSnowBufferSize, newSize);
				depth_pass(snow_view, snow_proj, snowSource->snowDepthFBO, currentSnowBufferSize);
			},
			[&](int newSize)
			{
				// initDepthFBO is not used for lake reflection!
				// So, the variable has to be updated manually...
				currentReflectionBufferSize = newSize;

				// Re-create the reflection system with the new size
				delete lakeReflection;
				lakeReflection = new LakeReflection(currentReflectionBufferSize);
				lakeReflection->initialize();
			},
			[&](bool goFullscreen)
			{
				if (goFullscreen)
				{
					// Backup window position and size
					glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
					glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

					// Get monitor resolution
					const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
					glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);

					W_WIDTH  = mode->width;
					W_HEIGHT = mode->height;
				}
				else
				{
					// Restore to windowed mode
					glfwSetWindowMonitor(window, NULL, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);

					W_WIDTH  = windowedWidth;
					W_HEIGHT = windowedHeight;
				}
			}
		);

		// ===< HANDLE TIME >=== //

		// Simulated Time Control (for pausing the gameplay)
		float simulatedDeltaTime = myMenu.isMenuOpen ? 0.0f : deltaTime;
		simulatedFrameTime      += simulatedDeltaTime;

		// Getting camera information
		cameraProjectionMatrix = camera->projectionMatrix;
		static float footstepTimer = 0.0f;
		if (boatModel->isOnBoat())
		{
			vec3 oldBoatPosition = boatModel->getWorldPosition();

			if (!myMenu.isMenuOpen) boatModel->steer(simulatedDeltaTime);

			// --- COLLISION CHECKS --- //
			vec3 currentBoatPosition = boatModel->getWorldPosition();
			if (terrainSystem->checkCollisionBoat(currentBoatPosition, BOAT_RADIUS)
			 || marinaModel ->checkCollision(currentBoatPosition, BOAT_RADIUS))
				boatModel->setPosition(oldBoatPosition);

			cameraViewMatrix = boatModel->getViewMatrix();
		}
		else
		{
			vec3 oldCameraPosition = camera->position;
			bool isMoving          = false;

			if (!myMenu.isMenuOpen) isMoving = camera->update(simulatedDeltaTime); // Only move camera if menu is NOT open!

			// --- COLLISION CHECKS --- //
			vec3 currentCameraPosition = camera->position - vec3(0.0, 1.2, 0.0f);
			if (isMoving && !camera->flyingMode)
			{
				footstepTimer += simulatedDeltaTime;

				if (cabinModel->checkCollision(currentCameraPosition, PLAYER_RADIUS)
				 || forestSystem->checkCollision(currentCameraPosition, PLAYER_RADIUS)
				 || forestSystem2->checkCollision(currentCameraPosition, PLAYER_RADIUS)
				 || marinaModel->checkCollision(currentCameraPosition, PLAYER_RADIUS, true)
				 || boatModel->checkCollision(currentCameraPosition, PLAYER_RADIUS)
				 || terrainSystem->checkCollision(currentCameraPosition, PLAYER_RADIUS, isLakeFrozen))
					camera->position = oldCameraPosition;
				else if (footstepTimer > 0.7f)
				{
					soundSystem.play("assets/sounds/dirt_walk.ogg");
					footstepTimer = 0.0f;
				}
			}
			else footstepTimer = 0.7f;

			cameraViewMatrix = camera->viewMatrix;
		}

		// Control terrain's water speed...
		float flowSpeed = snowAmount > 0.0f ? mix(1.0f, 0.05f, fogDensity) : 1.0f;
		terrainTime    += (simulatedDeltaTime / 20.0f) * flowSpeed;
		cloudTime       =  simulatedFrameTime / 25.0f;



		// Light Depth Pass
		if (!(snowAmount == 1.0 && fogDensity > 0.1 && fogDensity < 0.9)) // Cheat during lake cracking...
		{
			light1->update(); // Light's view matrix
			light1->fitToCameraFrustum(cameraViewMatrix, cameraProjectionMatrix); // Light's projection matrix
			mat4 light1_proj = light1->projectionMatrix;
			mat4 light1_view = light1->viewMatrix;
			depth_pass(light1_view, light1_proj, depthFBO1, currentShadowBufferSize); // Create the depth buffer
		}



		const float growRate = 0.008f;

		// Snow accumulation
		if (snowingActive && snowStartTimer.hasFinished(3.5f))
		{
			snowAmount += simulatedDeltaTime * growRate;
			snowAmount  = clamp(snowAmount, 0.0f, 1.0f);
		}
		if (snowAmount == 1.0f && snowingActive)
		{
			snowInflate += simulatedDeltaTime * growRate * 3.0f;
			snowInflate  = clamp(snowInflate, 0.0f, 1.0f);
		}

		// Fog density control
		if (snowAmount > fogDensity && !forceClearFog) fogDensity = snowAmount;
		else if (forceClearFog)
		{
			fogDensity -= simulatedDeltaTime * growRate * 6.0f;
			fogDensity  = clamp(fogDensity, 0.0f, 1.0f);
		}
		isLakeFrozen = fogDensity > 0.4f && snowAmount > 0.0f; // Lake freezes when fog is dense enough!
		// Performance optimization...
		if (fogDensity == 1.0f) cameraFarPlane = 110.0f; // Reduce far plane for better performance!
		else                    cameraFarPlane = FAR_PLANE_INITIAL; // Reset far plane

		// ===< UPDATE SKY COLOR >=== //
		currentSkyColor = mix(skyColor, snowFogColor, fogDensity);
		glClearColor(currentSkyColor.x, currentSkyColor.y, currentSkyColor.z, 0.0f); // Better fog effect!



		// Lake Reflection Pass
		reflection_pass(cameraViewMatrix, cameraProjectionMatrix);
		
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F1))
			lighting_pass(snow_view, snow_proj); // See terrain's Frustum Culling!
		else
			lighting_pass(cameraViewMatrix, cameraProjectionMatrix); // Render the scene from camera's perspective!



		// Render SNOW PARTICLES - AFTER the lighting pass!
		snowfallSystem->setActive(snowingActive);
		snowfallSystem->update(simulatedDeltaTime, cameraViewMatrix, cameraProjectionMatrix, windPower);
		snowfallSystem->draw(cameraViewMatrix, cameraProjectionMatrix);



		// ---< Player interactions >--- //
		isDoorClose = distance(camera->position, cabinModel->getHingeWorldPosition()) < 1.8f;
		isBoatClose = (forceClearFog ? fogDensity < 0.1f : !isLakeFrozen) // DETAIL - When fog clears, the lake melts/cracks...
			       && distance(camera->position, boatModel->INITIAL_POSITION) < 5.0f
			       && distance(boatModel->getWorldPosition(), boatModel->INITIAL_POSITION) < 2.0f;

		// Door animation update!
		cabinModel->update(simulatedDeltaTime);
		// Render the prompt quad (when the camera is near the cabin door)
		if (isDoorClose) renderPrompt();

		// Boat animation update! Take into account the frozen lake...
		if      (!isLakeFrozen && !forceClearFog) boatModel->update(simulatedDeltaTime);
		else if (fogDensity < 0.1f)               boatModel->update(simulatedDeltaTime);
		// Render the prompt quad (when the camera is near the boat)
		if (isBoatClose) renderPrompt();



		// Render the ImGui frame (myMenu)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwWindowShouldClose(window) == 0);
}



void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ---< Menu toggle >--- //
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		myMenu.setMenuState(!myMenu.isMenuOpen, window);
	// Do not process any game keys below this line...
	ImGuiIO& io = ImGui::GetIO();
	if (myMenu.isMenuOpen || io.WantCaptureKeyboard) return;



	// Toggle polygon mode
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		GLint polygonMode[2];
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode[0]);

		// if GL_LINE, if GL_FILL check with polygonMode[0]
		if (polygonMode[0] == GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (polygonMode[0] == GL_FILL) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// ---< Player interactions >--- //
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		if (isDoorClose)
		{
			cabinModel->toggleDoor();
			soundSystem.play("assets/sounds/door_open.ogg");
		}

		if (isBoatClose)
		{
			if (boatModel->isOnBoat()) boatModel->setToPort1();
			boatModel->invertOnBoat();
		}
	}

	// ---< Snow control >--- //
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		snowStartTimer.start();
		snowingActive = !snowingActive;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		if (snowAmount < 1.0f)
			snowAmount = 1.0f; // Instant full snow!
		else
		{
			// Remove snow!
			snowAmount  = 0.0f;
			snowInflate = 0.0f;
		}

	// ---< Fog control >--- //
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		if (fogDensity < 1.0f)
			fogDensity = 1.0f; // Instant full fog!
		else
			fogDensity = 0.0f; // Remove fog!
	}
	// Clear fog/sky!
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		forceClearFog = !forceClearFog;
		snowingActive = false; // Stop snowing too!
		windPower     = -1;    // Stop wind!
	}

	// Wind power control
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		windPower = (windPower > 1) ? 1 : 6;

	// Move model [x] with numpad!
	/*else if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		const float step = 0.5f; // movement step size

		bool moved = false;

		if      (key == GLFW_KEY_KP_8) { tempPosition.z -= step; moved = true; } // forward (-Z)
		else if (key == GLFW_KEY_KP_5) { tempPosition.z += step; moved = true; } // back (+Z)
		else if (key == GLFW_KEY_KP_4) { tempPosition.x -= step; moved = true; } // left
		else if (key == GLFW_KEY_KP_6) { tempPosition.x += step; moved = true; } // right

		else if (key == GLFW_KEY_KP_ADD)      { tempPosition.y += step; moved = true; } // up
		else if (key == GLFW_KEY_KP_SUBTRACT) {tempPosition.y -= step; moved = true; } // down

		else if (key == GLFW_KEY_KP_7) { tempRotationAngle += 0.1f; moved = true; }
		else if (key == GLFW_KEY_KP_9) { tempRotationAngle -= 0.1f; moved = true; }

		if (moved)
		{

			printf("Position: (%.2f, %.2f, %.2f)\n",
				tempPosition.x, tempPosition.y, tempPosition.z);
			printf("Rotation angle: %.2f radians\n", tempRotationAngle);
		}
	}*/
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
	glClearColor(skyColor[0], skyColor[1], skyColor[2], 0.0f);

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
		vec4{ 0.8, 0.8, 1, 1 },
		vec4{ 0.8, 0.8, 1, 1 },
		vec4{ 0.8, 0.8, 1, 1 },
		vec3{ 0, 300, -300 }
	);

	// ===< SNOW SOURCE INIT >=== //
	snowSource = new SnowSource(vec3(0.0f, 340.0f, 0.0f));

	// ===< Setup ImGui >=== //
	myMenu.initialize(window);

	// ===< Audio >=== //
	soundSystem.init();
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
