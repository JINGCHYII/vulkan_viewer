#pragma once

#include "renderer/vulkan_context.hpp"

namespace viewer::renderer {

class RenderPasses {
public:
    void geometry_pass(const FrameContext& frame) const;
    void tonemap_pass(const FrameContext& frame) const;
    void ui_pass(const FrameContext& frame) const;
};

}  // namespace viewer::renderer
