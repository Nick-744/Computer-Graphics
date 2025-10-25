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
#include <glm/gtx/string_cast.hpp>   // glm::to_string

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Lab 02"

// Global variables
GLFWwindow* window;
GLuint shaderProgram;
GLuint MVPLocation;
GLuint triangleVAO, cubeVAO;
GLuint triangleVerticiesVBO, triangleColorsVBO, cubeVerticiesVBO, cubeColorsVBO;

void createContext() {
    // Create and compile the GLSL shader program from vertex and fragment shaders
    shaderProgram = loadShaders("transformation.vertexshader",
                                "simple.fragmentshader");

    // Task 1: Get a uniform location for the "MVP" (Model-View-Projection) matrix in the shader program
    MVPLocation = glGetUniformLocation(shaderProgram,"MVP");
 
    // --- Setting up the triangle ---
    // Generate and bind a Vertex Array Object (VAO) for the triangle
    glGenVertexArrays(1, &triangleVAO);
    glBindVertexArray(triangleVAO);

    // Define the vertex positions for the triangle
    static const GLfloat triangleVertices[] = {
         0.0f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };
    // Generate a Vertex Buffer Object (VBO) for triangle vertices
    glGenBuffers(1, &triangleVerticiesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVerticiesVBO);
    // Copy the vertex data into the VBO (static draw as it's not dynamic)
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    // Enable the first attribute (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // Define the color data for the triangle's vertices
    static const GLfloat triangleColors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    // Generate a VBO for the triangle's vertex colors
    glGenBuffers(1, &triangleColorsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleColorsVBO);
    // Copy color data into the VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColors), triangleColors, GL_STATIC_DRAW);
    // Enable the second attribute (color)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    // --- Setting up the cube ---
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    // Our vertices. Three consecutive floats give a 3D vertex; Three
    // consecutive vertices give a triangle. A cube has 6 faces with 2
    // triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    static const GLfloat cubeVertices[] = {
        -1.0f, -1.0f, -1.0f, // triangle 1 : begin
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f, // triangle 1 : end

         1.0f,  1.0f, -1.0f, // triangle 2 : begin
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, // triangle 2 : end

         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,

        -1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, 1.0f,

        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,

         1.0f, 1.0f,  1.0f,
         1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        
         1.0f, 1.0f,  1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f,  1.0f,

         1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 1.0f
    };
    glGenBuffers(1, &cubeVerticiesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVerticiesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // color VBO
    static const GLfloat cubeColors[] = {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };
    glGenBuffers(1, &cubeColorsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleColorsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void free() {
    glDeleteBuffers(1, &triangleVerticiesVBO);
    glDeleteBuffers(1, &triangleColorsVBO);
    glDeleteVertexArrays(1, &triangleVAO);
    glDeleteBuffers(1, &cubeVerticiesVBO);
    glDeleteBuffers(1, &cubeColorsVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
}

void mainLoop() {
    float T = 2.0f;
    
    // Define transformation matrices: Model, View, Projection, MVP
    mat4 Model, View, Proj, MVP, Identity, Scaling, Translation, Rotation;
    Scaling = glm::scale(mat4(), vec3(2.0f, 2.0f, 2.0f));
    //cout << glm::to_string(Scaling) << endl;

    Rotation = glm::rotate(mat4(), 3.14f/4.0f, vec3(0.0f, 0.0f, 1.0f));
    //cout << glm::to_string(Rotation) << endl;
    // Τα έκανε κατευθείαν transpose ώστε να είναι έτοιμα για την c++

    Translation = glm::translate(mat4(), vec3(0.0f, 2.0f, 1.0f));
    //cout << glm::to_string(Translation) << endl;

    // Main rendering loop: runs until the user presses ESC or closes the window
    do {
        float time  = (float) glfwGetTime();
        float theta = 2 * 3.14f * time / T;

        // Set the Model matrix
        Model = Translation * Rotation * Scaling;
        // Όταν κάνουμε Scaling, μεταφέρουμε τις κορυφές (συντεταγμένες)!
        // Οπότε, πρέπει να γίνει στην αρχή των αξόνων (αλλιώς, έξτρα μεταφορά - ΕΚΤΟΣ ΚΑΙ ΑΝ ΕΙΝΑΙ UNIFORM SCALE)
        
        // Define the Camera View Parameters
        vec3 camPos    = vec3(0.0f, 5.0f, 10.0f);
        vec3 camTarget = vec3(0.0f, 0.0f,  0.0f);
        vec3 camUp     = vec3(0.0f, 1.0f,  0.0f);

        // Construct the view matrix with camera position, target (lookAtPos), and up vector
        View = lookAt(camPos, camTarget, camUp);

        // Define the Camera Projection Parameters
        // Field of View (FOV) of 45 degrees (converted to radians), 4:3 aspect ratio, near and far clipping planes
        float fov      = radians(45.0f); // FOV in radians
        float aspect   = 4.0f / 3.0f;    // Aspect ratio of the screen (width / height)
        float nearClip = 0.1f;           // Near clipping plane (objects closer than 0.1 units are not rendered)
        float farClip  = 100.0f;         // Far clipping plane (objects further than 100 units are not rendered)

        // Construct the projection matrix with FOV, aspect ratio, and clipping planes
        Proj = perspective(fov, aspect, nearClip, farClip);
        
        // Construct the, Model - View - Projection  Matrix
        //MVP = Proj * View * Model;
        
        // Define the active shader program
        glUseProgram(shaderProgram);
        
        // Pass the MVP matrix to the shader as a uniform variable
        glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]); // Του λέμε να ψάξει στην MVP μεταβλητή!

        // Clear the screen to prepare for a new frame (only color buffer is cleared here)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Bind the triangle VAO (so OpenGL knows which object to render)
        glBindVertexArray(triangleVAO);
        
        // Draw the triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // Optional: You can uncomment the following lines to render a cube as well

        // Bind the cube VAO
        glBindVertexArray(cubeVAO);

        mat4 ModelCube = mat4();

        /* - <> - TASK 1 - <> - */
        if (false)
        {
            mat4 rotationCube = {
                { 0.5f * cos(theta) + 0.5f,  0.5f - 0.5f * cos(theta), -0.707 * sin(theta), 0.0f },
                { 0.5f - 0.5f * cos(theta),  0.5f * cos(theta) + 0.5f,  0.707 * sin(theta), 0.0f },
                { 0.707 * sin(theta),       -0.707 * sin(theta),        cos(theta),         0.0f },
                { 0.0f,                      0.0f,                      0.0f,               1.0f }
            }; // Rodrigues' Rotation Formula

            ModelCube = rotationCube * ModelCube;
        }

        /* - <> - TASK 2 - <> - */
        if (false)
        {
            mat4 rotationAxis = glm::rotate(mat4(), theta, vec3(-0.5f, 1.0f, 0.0f));

            mat4 translationCube        = glm::translate(mat4(), vec3(-2.0f, 0.0f, 0.0f));
            mat4 translationCubeReverse = glm::translate(mat4(), vec3( 2.0f, 0.0f, 0.0f));

            ModelCube = translationCubeReverse * rotationAxis * translationCube * ModelCube;
        }

        /* - <> - TASK 3 - <> - */
        if (true)
        {
            mat4 rotate = glm::rotate(mat4(), 3.14f / 4.0f, vec3(0.0f, 1.0f, 1.0f));

			ModelCube = rotate * ModelCube;
            // ...to the cube that is not aligned with the axes!
			// Το κάνω έτσι, ώστε να είναι ανεξάρτητες οι υλοποιήσεις των tasks.

            mat4 scaleCube     = glm::scale(mat4(), vec3(2.0f, 1.5f, 3.0f));
			mat4 rotateReverse = glm::rotate(mat4(), -3.14f / 4.0f, vec3(0.0f, 1.0f, 1.0f));

            ModelCube = rotate * scaleCube * rotateReverse * ModelCube;
        }

        MVP = Proj * View * ModelCube;
        
        // Draw the cube (12 triangles, 36 vertices)
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

        // Swap the front and back buffers to display the rendered frame
        glfwSwapBuffers(window);
        
        // Poll for events (e.g., keyboard input, window close event)
        glfwPollEvents();

    // Exit the loop when ESC key is pressed or the window is closed
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
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

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Task 12: Enable depth test
     glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
     glDepthFunc(GL_LESS);

    // Log
    logGLParameters();
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
