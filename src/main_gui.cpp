#include <algorithm>
#include <vector>
#include <random>
#include <memory>
#include <string>
#include <functional> // For std::function

// Include ImGui and GLFW headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// Project headers
#include "../include/Project/Quadtree.h"
#include "../include/Project/AxisAlignedBoundingBox.h"
#include "Project/Quadtree.h"

using namespace Project;

// Simple point struct for GUI
struct Point {
    float x, y;
    Point() : x(0), y(0) {}
    Point(float x, float y) : x(x), y(y) {}
};

// Forward declarations for drawing functions
void DrawPoint(const Point& p, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color = IM_COL32(255, 0, 0, 255));
void DrawRect(const AxisAlignedBoundingBox& aabb, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color = IM_COL32(0, 255, 0, 100), float thickness = 1.0f);
void DrawQuadtree(Quadtree<int>& qt, ImDrawList* drawList, const ImVec2& offset, float scale);

// Application state
struct AppState {
    std::unique_ptr<Quadtree<int>> quadtree;
    std::vector<Point> points;
    bool showBoundaries = true;
    bool showPoints = true;
    int pointCount = 100;
    float scale = 10.0f;
    ImVec2 offset{0, 0};
    bool isDragging = false;
    ImVec2 dragStartPos{0, 0};
    ImVec2 dragStartOffset{0, 0};
    std::string status;

    void generatePoints(int count, float width, float height) {
        points.clear();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distX(0, width);
        std::uniform_real_distribution<float> distY(0, height);

        // Create quadtree bounds slightly larger than the view
        AxisAlignedBoundingBox bounds(-width * 0.1f, -height * 0.1f, width * 1.1f, height * 1.1f);
        quadtree = std::make_unique<Quadtree<int>>(bounds, 4);
        
        for (int i = 0; i < count; ++i) {
            float x = distX(gen);
            float y = distY(gen);
            points.emplace_back(x, y);
            
            // Add to quadtree with index as metadata
            AxisAlignedBoundingBox pointAABB(x - 0.5f, y - 0.5f, x + 0.5f, y + 0.5f);
            quadtree->insert(pointAABB, i);
        }
        
        status = "Generated " + std::to_string(count) + " points";
    }
};

// Drawing functions
void DrawPoint(const Point& p, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float x = pos.x + offset.x + p.x * scale;
    float y = pos.y + offset.y + p.y * scale;
    drawList->AddCircleFilled(ImVec2(x, y), 3.0f, color);
}

void DrawRect(const AxisAlignedBoundingBox& aabb, ImDrawList* drawList, const ImVec2& offset, float scale, ImU32 color, float thickness) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float x1 = pos.x + offset.x + aabb.getMinX() * scale;
    float y1 = pos.y + offset.y + aabb.getMinY() * scale;
    float x2 = pos.x + offset.x + aabb.getMaxX() * scale;
    float y2 = pos.y + offset.y + aabb.getMaxY() * scale;
    drawList->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), color, 0.0f, 15, thickness);
}

void DrawQuadtree(Quadtree<int>& qt, ImDrawList* drawList, const ImVec2& offset, float scale) {
    // Draw the quadtree boundaries (recursively)
    std::function<void(const typename Quadtree<int>::QuadTreeNode*)> drawNode;
    drawNode = [&](const auto* node) {
        if (!node) return;
        
        // Draw this node's boundary
        if (!node->is_divided) {
            DrawRect(node->bounds, drawList, offset, scale, IM_COL32(100, 100, 100, 100), 1.0f); // Grey border for leaf nodes
        } else {
            DrawRect(node->bounds, drawList, offset, scale, IM_COL32(50, 50, 50, 150), 1.0f);    // Darker grey for non-leaf nodes
        }
        
        // Recursively draw children
        if (node->nw) {
            drawNode(node->nw.get());
            drawNode(node->ne.get());
            drawNode(node->sw.get());
            drawNode(node->se.get());
        }
    };
    
    if (qt.get_root()) {
        drawNode(qt.get_root());
    }
    
    // Draw all points in the quadtree
    for (const auto& [aabb, meta] : qt) {
        Point p{
            static_cast<float>(aabb.getCenterX()),
            static_cast<float>(aabb.getCenterY())
        };
        DrawPoint(p, drawList, offset, scale, IM_COL32(255, 0, 0, 255));
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
    
    // Initial window size
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    
    // Initialize with points
    appState.generatePoints(100, static_cast<float>(windowWidth), static_cast<float>(windowHeight));

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
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            appState.generatePoints(appState.pointCount, static_cast<float>(width), static_cast<float>(height));
        }
        
        ImGui::SliderInt("Point Count", &appState.pointCount, 10, 1000);
        ImGui::Checkbox("Show Boundaries", &appState.showBoundaries);
        ImGui::Checkbox("Show Points", &appState.showPoints);
        ImGui::SliderFloat("Scale", &appState.scale, 0.1f, 20.0f);
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        // Display status information
        ImGui::Separator();
        ImGui::Text("Status: %s", appState.status.c_str());
        ImGui::Text("Points: %zu", appState.points.size());
        ImGui::Text("Quadtree size: %zu", appState.quadtree ? appState.quadtree->size() : 0);
        
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
            float x = (mouse_pos.x - appState.offset.x - canvas_pos.x) / appState.scale;
            float y = (mouse_pos.y - appState.offset.y - canvas_pos.y) / appState.scale;
            
            // Create a small AABB for the point
            AxisAlignedBoundingBox pointAABB(x - 0.5f, y - 0.5f, x + 0.5f, y + 0.5f);
            
            // Add to points list and quadtree
            appState.points.emplace_back(x, y);
            int pointIndex = static_cast<int>(appState.points.size() - 1);
            appState.quadtree->insert(pointAABB, pointIndex);
            
            appState.status = "Added point at (" + std::to_string(x) + ", " + std::to_string(y) + ")";
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
