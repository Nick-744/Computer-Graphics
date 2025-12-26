#ifndef MENUGUI_H
#define MENUGUI_H

#include <GL/glew.h>
#include <glfw3.h>
#include <string>
#include <vector>
#include <functional>

// ===< ImGui >=== //
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

class MenuGUI
{
public:
    bool isMenuOpen = false; // Menu State

    // Constants - CHOOSE WISELY BASED ON YOUR PC HARDWARE...
    const std::vector<int> BUFFER_OPTIONS = { 512, 1024, 2048, 4096, 8192, 16384, 32768 };

    void initialize(GLFWwindow* window);

    // Main Render Function
    // Uses callbacks (std::function) to handle the complex logic of resizing buffers in main.cpp!
    void render(GLFWwindow* window,
        int& currentShadowSize, int& currentSnowSize, int& currentReflectionSize,
        std::function<void(int)> onShadowResolutionChange,
        std::function<void(int)> onSnowResolutionChange,
        std::function<void(int)> onReflectionResolutionChange
    );

    void setMenuState(bool open, GLFWwindow* window);

    void shutdown(); // Cleanup

private:
    void applyWinterTheme();
};

#endif
