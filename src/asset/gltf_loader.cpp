#include "gltf_loader.h"

#ifndef VULKAN_VIEWER_DISABLE_GLTF
#include <tiny_gltf.h>
#endif

#include <stdexcept>

namespace viewer {

Scene GltfLoader::load(const std::string& path) const {
    Scene scene;
    scene.sourcePath = path;

#ifdef VULKAN_VIEWER_DISABLE_GLTF
    throw std::runtime_error("tinygltf is not available. Please add third_party/tinygltf.");
#else
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string warnings;
    std::string errors;

    const bool isBinary = path.ends_with(".glb");
    bool ok = false;
    if (isBinary) {
        ok = loader.LoadBinaryFromFile(&model, &errors, &warnings, path);
    } else {
        ok = loader.LoadASCIIFromFile(&model, &errors, &warnings, path);
    }

    if (!warnings.empty()) {
        // Warnings can be forwarded to UI logs in a future change.
    }
    if (!ok) {
        throw std::runtime_error("Failed to load glTF: " + errors);
    }

    scene.materials.reserve(model.materials.size());
    for (const auto& srcMaterial : model.materials) {
        Material material;
        if (srcMaterial.values.contains("metallicFactor")) {
            material.metallicFactor = static_cast<float>(srcMaterial.values.at("metallicFactor").Factor());
        }
        if (srcMaterial.values.contains("roughnessFactor")) {
            material.roughnessFactor = static_cast<float>(srcMaterial.values.at("roughnessFactor").Factor());
        }
        scene.materials.emplace_back(std::move(material));
    }

    if (scene.materials.empty()) {
        scene.materials.emplace_back();
    }

    // Full primitive/attribute extraction is intentionally deferred.
    // This stub provides a validated loading path and material defaults.
#endif

    return scene;
}

} // namespace viewer
