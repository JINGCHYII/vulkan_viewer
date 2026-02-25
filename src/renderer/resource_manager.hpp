#pragma once

#include <vulkan/vulkan.h>

namespace viewer::renderer {

class ResourceManager {
public:
    void initialize(VkPhysicalDevice physical_device, VkDevice device);
    void cleanup();

    [[nodiscard]] VkDescriptorSetLayout create_descriptor_set_layout() const;
    [[nodiscard]] VkSampler create_linear_sampler() const;

private:
    [[nodiscard]] uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
};

}  // namespace viewer::renderer
