#include <algorithm>
#include <vector>
#include <random>
#include <memory>

// Include ImGui and GLFW headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// Project headers
#include "../include/Project/Quadtree.h"

// Forward declarations for drawing functions
void DrawPoint(const Point& p, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color = IM_COL32(255, 0, 0, 255));
void DrawRect(const Rect& r, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color = IM_COL32(0, 255, 0, 100), float thickness = 1.0f);
void DrawQuadtree(QuadTree& qt, ImDrawList* drawList, const ImVec2& offset, float scale);

// Application state
struct AppState {
    std::unique_ptr<QuadTree> quadtree;
    std::vector<Point> points;
    bool showBoundaries = true;
    bool showPoints = true;
    int pointCount = 100;
    float scale = 10.0f;
    ImVec2 offset{0, 0};
    bool isDragging = false;
    ImVec2 dragStartPos{0, 0};
    ImVec2 dragStartOffset{0, 0};

    void generatePoints(int count, float width, float height) {
        points.clear();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distX(0, width);
        std::uniform_real_distribution<float> distY(0, height);

        for (int i = 0; i < count; ++i) {
            points.push_back({distX(gen), distY(gen)});
        }
        
        // Rebuild quadtree with new points
        Rect boundary{width/2, height/2, width/2, height/2};
        quadtree = std::make_unique<QuadTree>(boundary);
        for (const auto& p : points) {
            quadtree->insert(p);
        }
    }
};

// Drawing functions
void DrawPoint(const Point& p, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float x = pos.x + offset.x + p.x * scale;
    float y = pos.y + offset.y + p.y * scale;
    drawList->AddCircleFilled(ImVec2(x, y), 3.0f, color);
}

void DrawRect(const Rect& r, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color, float thickness) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float x1 = pos.x + offset.x + (r.x - r.halfWidth) * scale;
    float y1 = pos.y + offset.y + (r.y - r.halfHeight) * scale;
    float x2 = pos.x + offset.x + (r.x + r.halfWidth) * scale;
    float y2 = pos.y + offset.y + (r.y + r.halfHeight) * scale;
    drawList->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), color, 0.0f, 15, thickness);
}

void DrawQuadtree(QuadTree& qt, ImDrawList* drawList, const ImVec2& offset, float scale) {
    // Draw the boundary of this node
    DrawRect(qt.getBoundary(), drawList, offset, scale, IM_COL32(100, 100, 100, 100));
    
    // If this node has children, draw them recursively
    if (qt.isDivided()) {
        DrawQuadtree(*qt.getNW(), drawList, offset, scale);
        DrawQuadtree(*qt.getNE(), drawList, offset, scale);
        DrawQuadtree(*qt.getSW(), drawList, offset, scale);
        DrawQuadtree(*qt.getSE(), drawList, offset, scale);
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        return -1;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "QuadTree Visualizer", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize AppState
    AppState appState;
    appState.generatePoints(100, 1000, 600);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Control window
        ImGui::Begin("QuadTree Controls");
        ImGui::Text("Use mouse wheel to zoom, left-click drag to pan, right-click to add points");
        
        if (ImGui::Button("Generate Points")) {
            appState.generatePoints(appState.pointCount, 1000, 600);
        }
        
        ImGui::SliderInt("Point Count", &appState.pointCount, 10, 1000);
        ImGui::Checkbox("Show Boundaries", &appState.showBoundaries);
        ImGui::Checkbox("Show Points", &appState.showPoints);
        ImGui::SliderFloat("Scale", &appState.scale, 0.1f, 20.0f);
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Main drawing area
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("QuadTree Visualization", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
        
        // Handle panning
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            if (!appState.isDragging) {
                appState.isDragging = true;
                appState.dragStartPos = ImGui::GetMousePos();
                appState.dragStartOffset = appState.offset;
            } else {
                ImVec2 dragDelta = ImVec2(
                    ImGui::GetMousePos().x - appState.dragStartPos.x,
                    ImGui::GetMousePos().y - appState.dragStartPos.y
                );
                appState.offset = ImVec2(
                    appState.dragStartOffset.x + dragDelta.x,
                    appState.dragStartOffset.y + dragDelta.y
                );
            }
        } else {
            appState.isDragging = false;
        }

        // Handle zooming with mouse wheel
        if (ImGui::IsWindowHovered()) {
            float wheel = io.MouseWheel;
            if (wheel != 0.0f) {
                float scaleFactor = wheel > 0 ? 1.1f : 0.9f;
                appState.scale = std::clamp(appState.scale * scaleFactor, 0.1f, 20.0f);
            }
        }

        // Get the draw list for the current window
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        
        // Draw the quadtree
        if (appState.quadtree && appState.showBoundaries) {
            DrawQuadtree(*appState.quadtree, drawList, appState.offset, appState.scale);
        }
        
        // Draw points
        if (appState.showPoints) {
            for (const auto& point : appState.points) {
                DrawPoint(point, drawList, appState.offset, appState.scale);
            }
        }
        
        // Handle point insertion on right click
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1)) { // Right click
            ImVec2 mouse_pos = ImGui::GetMousePos();
            Point p{
                (mouse_pos.x - appState.offset.x - canvas_pos.x) / appState.scale,
                (mouse_pos.y - appState.offset.y - canvas_pos.y) / appState.scale
            };
            if (appState.quadtree->insert(p)) {
                appState.points.push_back(p);
            }
        }
        
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
