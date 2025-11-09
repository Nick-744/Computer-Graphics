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

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Lab 03"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram;
GLuint MVPLocation;
GLuint textureSampler;
GLuint texture;
GLuint movingTexture;
GLuint movingTextureSampler;
GLuint displacementTexture;
GLuint displacementTextureSampler;
GLuint timeUniform;
GLuint suzanneVAO;
GLuint suzanneVerticiesVBO, suzanneUVVBO;
std::vector<vec3> suzanneVertices, suzanneNormals;
std::vector<vec2> suzanneUVs;

void createContext() {
    // Create and compile our GLSL program from the shaders
    shaderProgram = loadShaders("texture.vertexshader", "texture.fragmentshader");

    // Set polygon rendering mode for all faces to wireframe (GL_LINE) or solid fill (GL_FILL).
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Get the location of the MVP matrix uniform in the shader program
    MVPLocation = glGetUniformLocation(shaderProgram, "MVP");

    // Load 3D model from the specified OBJ file.
    //loadOBJ("suzanne.obj", suzanneVertices, suzanneUVs, suzanneNormals);
    loadOBJ("cube.obj", suzanneVertices, suzanneUVs, suzanneNormals);

    // VAO
    glGenVertexArrays(1, &suzanneVAO);
    glBindVertexArray(suzanneVAO);

    // vertex VBO
    glGenBuffers(1, &suzanneVerticiesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, suzanneVerticiesVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        suzanneVertices.size() * sizeof(glm::vec3),
        &suzanneVertices[0],
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // Task 6: texture loading
    // uncomment Task 6 in main loop
    // Get a handle for our "textureSampler" uniform
    textureSampler = glGetUniformLocation(shaderProgram, "textureSampler");

    // load BMP
    texture = loadBMP("uvtemplate.bmp");

    // uvs VBO
    glGenBuffers(1, &suzanneUVVBO);
    glBindBuffer(GL_ARRAY_BUFFER, suzanneUVVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        suzanneUVs.size() * sizeof(glm::vec2),
        &suzanneUVs[0],
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Task 7 change main texture, load moving texture BMP, get handle and get uniform time

    // water or fire?
    texture = loadBMP("glass_rock_bottom.bmp");
    //texture = loadBMP("bottom.bmp");
    //texture = loadBMP("lava.bmp");

    movingTexture = loadBMP("water.bmp");
    //movingTexture = loadBMP("fiery.bmp");
    movingTextureSampler = glGetUniformLocation(shaderProgram, "movingTextureSampler");

    timeUniform = glGetUniformLocation(shaderProgram, "time");

    // Task 8 load displacement texture BMP and get handle
    //displacementTexture = loadBMP("gray.bmp");
    displacementTexture = loadBMP("my_perlin_noise.bmp");
    displacementTextureSampler = glGetUniformLocation(shaderProgram, "displacementTextureSampler");
}

void free() {
    glDeleteBuffers(1, &suzanneVerticiesVBO);
    glDeleteBuffers(1, &suzanneUVVBO);
    glDeleteTextures(1, &texture);
    glDeleteTextures(1, &movingTexture);
    glDeleteTextures(1, &displacementTexture);
    glDeleteVertexArrays(1, &suzanneVAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void mainLoop() {
    do {
        // Declare the transformation matrices
        mat4 MVP, modelMatrix, viewMatrix, projectionMatrix;

        // Clear the screen and depth buffer to prepare for the new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program for rendering
        glUseProgram(shaderProgram);

        // Bind the Vertex Array Object (VAO) for the model to be drawn (e.g., 'suzanne')
        glBindVertexArray(suzanneVAO);

        // Task 5: Camera updates for movement and orientation
        camera->update(); // Update the camera based on user input

        // Retrieve the camera's projection and view matrices after the update
        projectionMatrix = camera->projectionMatrix;
        viewMatrix       = camera->viewMatrix;

        // Reset the model matrix to identity (no transformations)
        modelMatrix = glm::mat4(1.0);
        
        
        
        // --- Setup objects in the scene --- //

        // Move Suzanne!
        mat4 translateSuzanne = glm::translate(mat4(), vec3(0.0f, 0.0f, -2.0f));
        modelMatrix           = translateSuzanne * modelMatrix;



        // Recalculate the MVP matrix with the updated camera settings
        MVP = projectionMatrix * viewMatrix * modelMatrix; // Combined transformation matrix

        // Pass the MVP matrix to the shader
        glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]);


        
        // Task 6: texture
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        // Set our "textureSampler" sampler to use Texture Unit 0
        glUniform1i(textureSampler, 0);



        // Task 7: moving water/fire texture
        // Activate texture1
        glActiveTexture(GL_TEXTURE1);
        // Bind our texture in the currently active texture unit (which now is 1)
        glBindTexture(GL_TEXTURE_2D, movingTexture);
        // Set our "textureSampler" sampler to use Texture Unit 1
        glUniform1i(movingTextureSampler, 1);
        // Pass time to shader
        glUniform1f(timeUniform, (float) glfwGetTime() / 20.0);


         
        // Task 8: displacement texture
        // Activate texture1
        glActiveTexture(GL_TEXTURE2);
        // Bind our texture in the currently active texture unit (which now is 1)
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        // Set our "textureSampler" sampler to use Texture Unit 1
        glUniform1i(displacementTextureSampler, 2);



        // Draw the model using the currently bound VAO
        glDrawArrays(GL_TRIANGLES, 0, suzanneVertices.size()); // Render the model as triangles

        // Swap the front and back buffers to display the rendered frame
        glfwSwapBuffers(window);

        // Poll for and process events (like user input, window events, etc.)
        glfwPollEvents();
        
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
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

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Task 6.1
    // Enable blending for transparency
    // change alpha in fragment shader
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Task 6.2
    // Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE); // Για να μην κάνεις render περιττή πληροφορία!

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
    }
    catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }
    return 0;
}
