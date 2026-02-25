#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct GLFWwindow;

namespace viewer {

class VulkanContext {
  public:
    void initialize(GLFWwindow* window);
    void shutdown();

    [[nodiscard]] VkInstance instance() const { return instance_; }
    [[nodiscard]] VkDevice device() const { return device_; }
    [[nodiscard]] VkPhysicalDevice physicalDevice() const { return physicalDevice_; }
    [[nodiscard]] VkQueue graphicsQueue() const { return graphicsQueue_; }

  private:
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();

    GLFWwindow* window_ = nullptr;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;

    std::vector<const char*> requiredInstanceExtensions_;
};

} // namespace viewer
