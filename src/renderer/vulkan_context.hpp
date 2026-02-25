#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace viewer::renderer {

struct FrameContext {
    uint32_t image_index = 0;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
};

class VulkanContext {
public:
    void initialize(GLFWwindow* window);
    void cleanup();

    [[nodiscard]] FrameContext begin_frame(float clear_r, float clear_g, float clear_b, float clear_a);
    void end_frame(const FrameContext& frame);

    [[nodiscard]] VkDevice device() const { return device_; }
    [[nodiscard]] VkPhysicalDevice physical_device() const { return physical_device_; }
    [[nodiscard]] VkInstance instance() const { return instance_; }
    [[nodiscard]] VkQueue graphics_queue() const { return graphics_queue_; }
    [[nodiscard]] uint32_t graphics_queue_family() const { return graphics_queue_family_; }
    [[nodiscard]] VkRenderPass render_pass() const { return render_pass_; }
    [[nodiscard]] VkExtent2D swapchain_extent() const { return swapchain_extent_; }
    [[nodiscard]] uint32_t current_frame_index() const { return current_frame_; }
    [[nodiscard]] VkCommandPool command_pool() const { return command_pool_; }
    [[nodiscard]] VkDescriptorPool imgui_descriptor_pool() const { return imgui_descriptor_pool_; }

private:
    void create_instance();
    void create_surface(GLFWwindow* window);
    void pick_physical_device();
    void create_device();
    void create_swapchain();
    void create_image_views();
    void create_render_pass();
    void create_framebuffers();
    void create_command_pool();
    void create_command_buffers();
    void create_sync_objects();
    void create_imgui_descriptor_pool();

    void destroy_swapchain_objects();

    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;
    };

    [[nodiscard]] QueueFamilies query_queue_families(VkPhysicalDevice device) const;

    static constexpr uint32_t kMaxFramesInFlight = 2;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;

    uint32_t graphics_queue_family_ = 0;
    uint32_t present_queue_family_ = 0;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchain_format_ = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D swapchain_extent_{};
    std::vector<VkImage> swapchain_images_;
    std::vector<VkImageView> swapchain_image_views_;

    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers_;

    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_;

    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> in_flight_fences_;

    VkDescriptorPool imgui_descriptor_pool_ = VK_NULL_HANDLE;

    uint32_t current_frame_ = 0;
};

}  // namespace viewer::renderer
