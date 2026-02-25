#include "renderer.h"

#include "ui/imgui_layer.h"

#include <GLFW/glfw3.h>

namespace viewer {

void Renderer::initialize(GLFWwindow* window) {
    context_.initialize(window);
}

void Renderer::shutdown() {
    context_.shutdown();
}

void Renderer::setScene(Scene scene) {
    scene_ = std::move(scene);
    uploadScene();
}

void Renderer::render(ImGuiLayer& imguiLayer) {
    // Placeholder frame orchestration.
    // 1. begin frame
    // 2. render PBR scene with dynamic rendering
    // 3. render ImGui
    // 4. submit/present
    imguiLayer.recordRenderData();
}

void Renderer::uploadScene() {
    if (!scene_) {
        return;
    }
    // TODO: create GPU buffers/images and descriptor sets for scene resources.
}

} // namespace viewer
