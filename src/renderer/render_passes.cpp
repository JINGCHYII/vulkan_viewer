#include "renderer/render_passes.hpp"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace viewer::renderer {

void RenderPasses::geometry_pass(const FrameContext& /*frame*/) const {
    // Placeholder: scene geometry recording lives here.
}

void RenderPasses::tonemap_pass(const FrameContext& /*frame*/) const {
    // Placeholder: HDR->LDR tonemap/fullscreen composition lives here.
}

void RenderPasses::ui_pass(const FrameContext& frame) const {
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data != nullptr) {
        ImGui_ImplVulkan_RenderDrawData(draw_data, frame.command_buffer);
    }
}

}  // namespace viewer::renderer
