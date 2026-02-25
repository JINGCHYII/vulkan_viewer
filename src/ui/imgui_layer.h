#pragma once

#include "renderer/renderer.h"

#include <array>
#include <string>

struct GLFWwindow;

namespace viewer {

class VulkanContext;

struct UiRequest {
    enum class Type {
        None,
        LoadModel,
        Quit,
    } type = Type::None;

    std::string path;
};

class ImGuiLayer {
  public:
    void initialize(GLFWwindow* window, VulkanContext& context);
    void shutdown();

    void beginFrame();
    UiRequest drawScenePanel();
    void drawRendererPanel(RendererDebugState& state);
    void endFrame();

    void recordRenderData();

  private:
    std::array<char, 512> modelPathBuffer_{};
};

} // namespace viewer
