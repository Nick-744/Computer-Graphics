// Include C++ headers
#include <iostream>
#include <string>

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

#include "Cube.h"
#include "Sphere.h"
#include "Box.h"
#include "MassSpringDamper.h"
#include "Collision.h"

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
struct Light; struct Material;
void uploadMaterial(const Material& mtl);
void uploadLight(const Light& light);

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Lab 07"
#define MATERIALS

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram;
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation;

GLuint LaLocation, LdLocation, LsLocation, lightPositionLocation, lightPowerLocation;
GLuint KdLocation, KsLocation, KaLocation, NsLocation;

struct Light {
    vec4 La;
    vec4 Ld;
    vec4 Ls;
    vec3 lightPosition_worldspace;
    float power;
};

struct Material {
    vec4 Ka;
    vec4 Kd;
    vec4 Ks;
    float Ns;
};

const Material defaultMaterial{
    vec4{0.1, 0.1, 0.1, 1},
    vec4{1.0, 0.8, 0.0, 1},
    vec4{0.3, 0.3, 0.3, 1},
    20.0f
};

Light light{
    vec4{ 1, 1, 1, 1},
    vec4{ 1, 1, 1, 1},
    vec4{ 1, 1, 1, 1},
    vec3(8, 8, 8),
    30.0f
};

Cube* cube;
Sphere* sphere;

Sphere* sphere2; Sphere* sphere3; // HOMEWORK 2

Box* box;
MassSpringDamper* msd;

// Standard acceleration due to gravity
const float g = 9.80665f;

void createContext() {
    shaderProgram = loadShaders(
        "StandardShading.vertexshader",
        "StandardShading.fragmentshader");

    projectionMatrixLocation = glGetUniformLocation(shaderProgram, "P");
    viewMatrixLocation = glGetUniformLocation(shaderProgram, "V");
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    
    KaLocation = glGetUniformLocation(shaderProgram, "mtl.Ka");
    KdLocation = glGetUniformLocation(shaderProgram, "mtl.Kd");
    KsLocation = glGetUniformLocation(shaderProgram, "mtl.Ks");
    NsLocation = glGetUniformLocation(shaderProgram, "mtl.Ns");
    LaLocation = glGetUniformLocation(shaderProgram, "light.La");
    LdLocation = glGetUniformLocation(shaderProgram, "light.Ld");
    LsLocation = glGetUniformLocation(shaderProgram, "light.Ls");
    lightPositionLocation = glGetUniformLocation(shaderProgram, "light.lightPosition_worldspace");
    lightPowerLocation = glGetUniformLocation(shaderProgram, "light.power");

    box = new Box(8);
    // Task 1a: change the parameters of the cube
    cube = new Cube(vec3(4, 5, 4), vec3(0, 0, 0), vec3(1, 0, 1), 0.5, 1);
    // Task 2a: change the initial velocity and position, mass and radius
    sphere = new Sphere(vec3(4, 4, 4), vec3(0, 5, 0), 0.4, 10);
    // Task 3a: change the initial parameters of the system
    msd = new MassSpringDamper(vec3(4, 0, 4), vec3(0, 0, 0), 0.5, 1,
                               vec3(4, 4, 4), 50, 10, 2);

	// ===< HOMEWORK 2 >=== //
	sphere2 = new Sphere(vec3(5, 5, 4), vec3(-2, -1, 0), 0.4, 10);
	sphere3 = new Sphere(vec3(3, 5, 4), vec3( 2, -1, 0), 0.4, 10);
}

void free() {
    delete cube;
    delete sphere;
    delete box;
    delete msd;
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void mainLoop() {
    float t = glfwGetTime();
    vec3 lightPos = vec3(10, 10, 10);
    camera->position = glm::vec3(box->size / 2, box->size / 2, 20);
    float maxEnergy = 0;
    do {
        // calculate dt
        float currentTime = glfwGetTime();
        float dt = currentTime - t;
		//float dt = 0.01f; // Για να παρατηρήσουμε καλύτερα την αστάθεια λόγω σφαλμάτων!

        // Task 2e: change dt to 0.001f and observe the total energy, then change
        // the numerical integration method to Runge - Kutta 4th order (in RigidBody.cpp)

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

    #ifdef MATERIALS
        uploadLight(light);
        uploadMaterial(defaultMaterial);
    #endif

        // camera
        camera->update();
        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix = camera->viewMatrix;
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        // box
        box->update(t, dt);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &box->modelMatrix[0][0]);
        box->draw();

        // Task 1: cube simulation (linear motion Newton's First Law)

        // A rigid body can rotate and translate, while the Newton-Euler
        // equations define its dynamics. In order to visualize the cube one
        // must first advance the equations in time (numerical integration) and
        // then use the model matrix to position the drawable in space.

        // cube
        /*
        cube->update(t, dt);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &cube->modelMatrix[0][0]);
        cube->draw();
        */

        // Task 2: simulation of a sphere (collision and forces)
        
        // Task 2b: define the velocity of the sphere after the collision
        /*
        handleBoxSphereCollision(*box, *sphere);

        // Task 2c: model the force due to gravity
        sphere->forcing = [&](float t, const vector<float>& y)->vector<float>
        {
            vector<float> f(6, 0.0f);
            f[1] = -sphere->m * g; // Αρνητικό y!
            return f;
        };
        sphere->update(t, dt);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere->modelMatrix[0][0]);
        sphere->draw();

        // Task 2d: calculate the total energy and comment on the previous results
        float KE = sphere->calcKinecticEnergy();
		float PE = sphere->m * g * sphere->x.y;
        float T  = KE + PE;
        if (T > maxEnergy)
        {
            // Παρατηρούμε ότι η ολική ενέργεια αυξάνεται!
			// Σφάλμα από: collision response & numerical integration
            
            // Αναλυτικότερα, κόβοντας την 2η παράγωγο στον Taylor, έχουμε θετικό σφάλμα!
            // Δηλαδή, δεν χάνεται ενέργεια λόγο της βαρύτητας...
            cout << "Total Energy: " << T << endl;
            maxEnergy = T;
        }
        */

        // Task 3: model a mass-spring-damper system
        // Task 3b: model the forces due to gravity, damper and spring
        /*
        msd->forcing = [&](float t, const vector<float>& y)->vector<float>
        {
            vector<float> f(6, 0.0f);
            f[1] = -(msd->m * g)
                  + (msd->k * (msd->a.y - msd->x.y - msd->l0))
                  - (msd->b * msd->v.y);
			// Βαρύτητα + Hooke's Law + απόσβεση
            return f;
        };
        msd->update(t, dt);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &msd->springModelMatrix[0][0]);
        msd->draw(0); // draw spring = 0
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &msd->blobModelMatrix[0][0]);
        msd->draw(1); // draw blob = 0
        */

        {   // ===< HOMEWORK 2 >=== //

            // Collision setup!
            handleSphereSphereCollision(*sphere,  *sphere2);
            handleSphereSphereCollision(*sphere,  *sphere3);
            handleSphereSphereCollision(*sphere2, *sphere3);

            handleBoxSphereCollision(*box, *sphere);
            handleBoxSphereCollision(*box, *sphere2);
            handleBoxSphereCollision(*box, *sphere3);

			// Βαρύτητα και για τις 3 σφαίρες!
            sphere->forcing = [&](float t, const vector<float>& y)->vector<float>
            {
                vector<float> f(6, 0.0f);
                f[1] = -sphere->m * g;
                return f;
            };

            sphere2->forcing = [&](float t, const vector<float>& y)->vector<float>
            {
                vector<float> f(6, 0.0f);
                f[1] = -sphere->m * g;
                return f;
            };

            sphere3->forcing = [&](float t, const vector<float>& y)->vector<float>
            {
                vector<float> f(6, 0.0f);
                f[1] = -sphere->m * g;
                return f;
            };

            // ΖΩΓΡΑΦΙΚΗΗΗ!
            sphere->update(t, dt);
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere->modelMatrix[0][0]);
            sphere->draw();

            sphere2->update(t, dt);
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere2->modelMatrix[0][0]);
            sphere2->draw();

            sphere3->update(t, dt);
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere3->modelMatrix[0][0]);
            sphere3->draw();
        }

        t += dt;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}

void initialize() {
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

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
    // glFrontFace(GL_CW);
    // glFrontFace(GL_CCW);

    // enable point size when drawing points
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);
}

void uploadMaterial(const Material& mtl)
{
    glUniform4f(KaLocation, mtl.Ka.r, mtl.Ka.g, mtl.Ka.b, mtl.Ka.a);
    glUniform4f(KdLocation, mtl.Kd.r, mtl.Kd.g, mtl.Kd.b, mtl.Kd.a);
    glUniform4f(KsLocation, mtl.Ks.r, mtl.Ks.g, mtl.Ks.b, mtl.Ks.a);
    glUniform1f(NsLocation, mtl.Ns);
}

void uploadLight(const Light& light)
{
    glUniform4f(LaLocation, light.La.r, light.La.g, light.La.b, light.La.a);
    glUniform4f(LdLocation, light.Ld.r, light.Ld.g, light.Ld.b, light.Ld.a);
    glUniform4f(LsLocation, light.Ls.r, light.Ls.g, light.Ls.b, light.Ls.a);
    glUniform3f(lightPositionLocation, light.lightPosition_worldspace.x,
        light.lightPosition_worldspace.y, light.lightPosition_worldspace.z);
    glUniform1f(lightPowerLocation, light.power);
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
