#pragma once

#include "asset/gltf_loader.h"
#include "renderer/vulkan_context.h"

#include <optional>

struct GLFWwindow;

namespace viewer {

class ImGuiLayer;

enum class DebugView {
    FinalLit,
    Normal,
    BaseColor,
    Metallic,
    Roughness,
};

struct RendererDebugState {
    DebugView debugView = DebugView::FinalLit;
    float exposure = 1.0f;
    float iblStrength = 1.0f;
    bool enableDirectionalLight = true;
    float directionalLightIntensity = 3.0f;
};

class Renderer {
  public:
    void initialize(GLFWwindow* window);
    void shutdown();

    void setScene(Scene scene);
    void render(ImGuiLayer& imguiLayer);

    [[nodiscard]] VulkanContext& context() { return context_; }
    [[nodiscard]] RendererDebugState& debugState() { return debugState_; }

  private:
    void uploadScene();

    VulkanContext context_;
    std::optional<Scene> scene_;
    RendererDebugState debugState_{};
};

} // namespace viewer
