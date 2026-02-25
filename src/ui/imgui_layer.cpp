#include "imgui_layer.h"

#include "renderer/vulkan_context.h"

#include <imgui.h>

#include <cstring>

namespace viewer {

void ImGuiLayer::initialize(GLFWwindow* /*window*/, VulkanContext& /*context*/) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    const char* defaultPath = "assets/DamagedHelmet/DamagedHelmet.gltf";
    std::strncpy(modelPathBuffer_.data(), defaultPath, modelPathBuffer_.size() - 1);

    // TODO: hook ImGui_ImplGlfw_InitForVulkan / ImGui_ImplVulkan_Init
}

void ImGuiLayer::shutdown() {
    ImGui::DestroyContext();
}

void ImGuiLayer::beginFrame() {
    ImGui::NewFrame();
}

UiRequest ImGuiLayer::drawScenePanel() {
    UiRequest request;

    ImGui::Begin("Scene");
    ImGui::InputText("glTF Path", modelPathBuffer_.data(), modelPathBuffer_.size());

    if (ImGui::Button("Load glTF")) {
        request.type = UiRequest::Type::LoadModel;
        request.path = modelPathBuffer_.data();
    }
    ImGui::SameLine();
    if (ImGui::Button("Quit")) {
        request.type = UiRequest::Type::Quit;
    }
    ImGui::End();

    return request;
}

void ImGuiLayer::drawRendererPanel(RendererDebugState& state) {
    ImGui::Begin("Renderer");
    const char* modes[] = {"Final Lit", "Normal", "BaseColor", "Metallic", "Roughness"};
    int mode = static_cast<int>(state.debugView);
    if (ImGui::Combo("Debug View", &mode, modes, IM_ARRAYSIZE(modes))) {
        state.debugView = static_cast<DebugView>(mode);
    }

    ImGui::SliderFloat("Exposure", &state.exposure, 0.1f, 8.0f);
    ImGui::SliderFloat("IBL Strength", &state.iblStrength, 0.0f, 2.0f);
    ImGui::Checkbox("Directional Light", &state.enableDirectionalLight);
    ImGui::SliderFloat("Dir Light Intensity", &state.directionalLightIntensity, 0.0f, 10.0f);
    ImGui::End();
}

void ImGuiLayer::endFrame() {
    ImGui::Render();
}

void ImGuiLayer::recordRenderData() {
    // TODO: translate ImGui::GetDrawData into Vulkan commands.
}

} // namespace viewer
