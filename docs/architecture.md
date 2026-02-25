# Architecture

## Module split

- `src/app.*`: App lifecycle, window loop, subsystem wiring.
- `src/renderer/vulkan_context.*`: Vulkan instance/device bootstrap.
- `src/renderer/renderer.*`: Frame orchestration and scene upload boundary.
- `src/asset/gltf_loader.*`: glTF scene/material extraction.
- `src/ui/imgui_layer.*`: UI draw and runtime control panel.

## Render flow (target)

1. Acquire swapchain image.
2. Geometry pass with PBR shader (dynamic rendering).
3. Optional post process (tone mapping).
4. ImGui overlay pass.
5. Queue submit + present.

## Material model

Uses glTF 2.0 metallic-roughness workflow:

- baseColorFactor + baseColorTexture
- metallicFactor + roughnessFactor + metallicRoughnessTexture
- normal / occlusion / emissive texture slots

## Planned extensions

- Descriptor buffer / bindless material tables.
- GPU-driven culling and indirect draw.
- Offline environment map prefilter pipeline.
