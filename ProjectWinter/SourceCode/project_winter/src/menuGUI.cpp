#include "menuGUI.h"
#include <iostream>

using namespace std;

void MenuGUI::initialize(GLFWwindow* window)
{
    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;

    // Link ImGui to GLFW and OpenGL!
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    applyWinterTheme(); // Apply custom style
}

void MenuGUI::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void MenuGUI::render(GLFWwindow* window,
    int& currentShadowSize, int& currentSnowSize, int& currentReflectionSize,
    std::function<void(int)> onShadowResolutionChange,
    std::function<void(int)> onSnowResolutionChange,
    std::function<void(int)> onReflectionResolutionChange)
{
    // Start Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (isMenuOpen)
    {
        // Center the window
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        ImGui::SetNextWindowPos(ImVec2(width * 0.5f, height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 320));

        // Window Flags
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

        ImGui::Begin("Game Menu", nullptr, window_flags);

        // --- TITLE --- //
        float font_size  = ImGui::GetFontSize() * 1.5f;
        float text_width = ImGui::CalcTextSize("PAUSED").x * 1.5f;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);

        ImGui::SetWindowFontScale(1.5f);
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "PAUSED");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

        // --- SETTINGS --- //
        ImGui::TextDisabled("GRAPHICS SETTINGS");
        ImGui::Spacing();

        ImGui::Columns(2, "settings_columns", false);
        ImGui::SetColumnWidth(0, 150.0f);

        // Row 1: Shadow
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Shadow Resolution");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##shadow", to_string(currentShadowSize).c_str()))
        {
            for (int size : BUFFER_OPTIONS)
            {
                bool is_selected = (currentShadowSize == size);
                if (ImGui::Selectable(to_string(size).c_str(), is_selected))
                    if (onShadowResolutionChange) onShadowResolutionChange(size);
                    // Call the callback provided by main.cpp
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::NextColumn();

        // Row 2: Snow
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Snow Resolution");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##snow", to_string(currentSnowSize).c_str()))
        {
            for (int size : BUFFER_OPTIONS)
            {
                bool is_selected = (currentSnowSize == size);
                if (ImGui::Selectable(to_string(size).c_str(), is_selected))
                    if (onSnowResolutionChange) onSnowResolutionChange(size);
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::NextColumn();

        // Row 3: Reflection
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Reflection Quality");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##reflection", to_string(currentReflectionSize).c_str()))
        {
            for (int size : BUFFER_OPTIONS) {
                bool is_selected = (currentReflectionSize == size);
                if (ImGui::Selectable(to_string(size).c_str(), is_selected))
                    if (onReflectionResolutionChange) onReflectionResolutionChange(size);
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Columns(1);
        ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

        // --- BUTTONS --- //
        float buttonWidth  = 150.0f;
        float buttonHeight = 45.0f;
        float windowWidth  = ImGui::GetWindowSize().x;

        // RESUME
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        if (ImGui::Button("RESUME", ImVec2(buttonWidth, buttonHeight))) setMenuState(false, window);

        ImGui::Spacing();

        // EXIT
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("EXIT GAME", ImVec2(buttonWidth, buttonHeight)))
            glfwSetWindowShouldClose(window, 1);
        ImGui::PopStyleColor(2);

        ImGui::End();
    }
}

void MenuGUI::setMenuState(bool open, GLFWwindow* window)
{
    isMenuOpen = open;

    if (isMenuOpen) // Unlock mouse for menu
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else
    {
        // Lock mouse for game
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Prevent camera jump
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        glfwSetCursorPos(window, w / 2.0, h / 2.0);
    }
}

void MenuGUI::applyWinterTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 4.0f;
    style.GrabRounding      = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.ItemSpacing       = ImVec2(8, 6);
    style.WindowPadding     = ImVec2(15, 15);

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]       = ImVec4(0.09f, 0.11f, 0.15f, 0.96f);
    colors[ImGuiCol_Header]         = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.25f, 0.32f, 0.45f, 1.00f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.30f, 0.38f, 0.55f, 1.00f);
    colors[ImGuiCol_Button]         = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.28f, 0.35f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.15f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBg]        = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.18f, 0.24f, 0.32f, 1.00f);
    colors[ImGuiCol_Text]           = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
    colors[ImGuiCol_Border]         = ImVec4(0.25f, 0.30f, 0.40f, 0.50f);
}
