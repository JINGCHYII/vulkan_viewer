#include "renderer/resource_manager.hpp"

#include <stdexcept>

namespace viewer::renderer {

void ResourceManager::initialize(VkPhysicalDevice physical_device, VkDevice device) {
    physical_device_ = physical_device;
    device_ = device;
}

void ResourceManager::cleanup() {
    device_ = VK_NULL_HANDLE;
    physical_device_ = VK_NULL_HANDLE;
}

VkDescriptorSetLayout ResourceManager::create_descriptor_set_layout() const {
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorCount = 1;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device_, &create_info, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout.");
    }

    return layout;
}

VkSampler ResourceManager::create_linear_sampler() const {
    VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.maxLod = 1.0f;

    VkSampler sampler = VK_NULL_HANDLE;
    if (vkCreateSampler(device_, &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create sampler.");
    }

    return sampler;
}

uint32_t ResourceManager::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties mem_properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        const bool type_ok = (type_filter & (1 << i)) != 0;
        const bool props_ok =
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties;
        if (type_ok && props_ok) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type.");
}

}  // namespace viewer::renderer
