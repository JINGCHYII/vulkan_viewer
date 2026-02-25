#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace viewer {

struct Vertex {
    float position[3]{};
    float normal[3]{};
    float tangent[4]{};
    float uv[2]{};
};

struct Material {
    float baseColorFactor[4]{1.0f, 1.0f, 1.0f, 1.0f};
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    std::string baseColorTexture;
    std::string metallicRoughnessTexture;
    std::string normalTexture;
    std::string occlusionTexture;
    std::string emissiveTexture;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t materialIndex = 0;
};

struct Scene {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::string sourcePath;
};

class GltfLoader {
  public:
    Scene load(const std::string& path) const;
};

} // namespace viewer
