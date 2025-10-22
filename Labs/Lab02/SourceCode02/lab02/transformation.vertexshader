#version 330 core

// input vertex and color data, different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;

// Task 2
uniform mat4 MVP;

// output data
out vec3 color;

void main()
{
    // Task 3: multiply with MVP
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);

    // propagate color to fragment shader
    color = vertexColor;
}
