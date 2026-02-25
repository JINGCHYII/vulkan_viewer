#include "renderer/vulkan_context.hpp"

#include <array>
#include <set>
#include <stdexcept>

namespace viewer::renderer {

namespace {

void vk_check(VkResult result, const char* message) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(message);
    }
}

}  // namespace

void VulkanContext::initialize(GLFWwindow* window) {
    create_instance();
    create_surface(window);
    pick_physical_device();
    create_device();
    create_swapchain();
    create_image_views();
    create_render_pass();
    create_framebuffers();
    create_command_pool();
    create_command_buffers();
    create_sync_objects();
    create_imgui_descriptor_pool();
}

void VulkanContext::cleanup() {
    if (device_ == VK_NULL_HANDLE) {
        return;
    }

    vkDeviceWaitIdle(device_);

    if (imgui_descriptor_pool_ != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device_, imgui_descriptor_pool_, nullptr);
    }

    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
        vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
        vkDestroyFence(device_, in_flight_fences_[i], nullptr);
    }

    if (command_pool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, command_pool_, nullptr);
    }

    destroy_swapchain_objects();

    vkDestroyDevice(device_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);

    device_ = VK_NULL_HANDLE;
}

FrameContext VulkanContext::begin_frame(float clear_r, float clear_g, float clear_b, float clear_a) {
    vkWaitForFences(device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

    FrameContext frame{};
    vk_check(vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                   image_available_semaphores_[current_frame_], VK_NULL_HANDLE,
                                   &frame.image_index),
             "Failed to acquire swapchain image.");

    vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);
    vkResetCommandBuffer(command_buffers_[current_frame_], 0);

    frame.command_buffer = command_buffers_[current_frame_];

    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vk_check(vkBeginCommandBuffer(frame.command_buffer, &begin_info),
             "Failed to begin command buffer.");

    VkClearValue clear_color{{clear_r, clear_g, clear_b, clear_a}};
    VkRenderPassBeginInfo render_begin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    render_begin.renderPass = render_pass_;
    render_begin.framebuffer = framebuffers_[frame.image_index];
    render_begin.renderArea.offset = {0, 0};
    render_begin.renderArea.extent = swapchain_extent_;
    render_begin.clearValueCount = 1;
    render_begin.pClearValues = &clear_color;

    vkCmdBeginRenderPass(frame.command_buffer, &render_begin, VK_SUBPASS_CONTENTS_INLINE);

    return frame;
}

void VulkanContext::end_frame(const FrameContext& frame) {
    vkCmdEndRenderPass(frame.command_buffer);
    vk_check(vkEndCommandBuffer(frame.command_buffer), "Failed to end command buffer.");

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_available_semaphores_[current_frame_];
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame.command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_finished_semaphores_[current_frame_];

    vk_check(vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fences_[current_frame_]),
             "Failed to submit queue.");

    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_finished_semaphores_[current_frame_];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain_;
    present_info.pImageIndices = &frame.image_index;

    vk_check(vkQueuePresentKHR(present_queue_, &present_info), "Failed to present image.");

    current_frame_ = (current_frame_ + 1) % kMaxFramesInFlight;
}

void VulkanContext::create_instance() {
    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = "vulkan_viewer";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "viewer_skeleton";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    uint32_t extension_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extensions;

    vk_check(vkCreateInstance(&create_info, nullptr, &instance_), "Failed to create instance.");
}

void VulkanContext::create_surface(GLFWwindow* window) {
    if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create surface.");
    }
}

void VulkanContext::pick_physical_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
    if (device_count == 0) {
        throw std::runtime_error("No Vulkan physical device found.");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

    for (const auto& device : devices) {
        auto families = query_queue_families(device);
        if (families.graphics.has_value() && families.present.has_value()) {
            physical_device_ = device;
            graphics_queue_family_ = families.graphics.value();
            present_queue_family_ = families.present.value();
            return;
        }
    }

    throw std::runtime_error("No suitable queue families found.");
}

void VulkanContext::create_device() {
    std::set<uint32_t> queue_families = {graphics_queue_family_, present_queue_family_};
    float queue_priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queue_infos;
    queue_infos.reserve(queue_families.size());

    for (uint32_t family : queue_families) {
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = family;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_priority;
        queue_infos.push_back(queue_info);
    }

    const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
    create_info.pQueueCreateInfos = queue_infos.data();
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = extensions;

    vk_check(vkCreateDevice(physical_device_, &create_info, nullptr, &device_),
             "Failed to create logical device.");

    vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, present_queue_family_, 0, &present_queue_);
}

void VulkanContext::create_swapchain() {
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, formats.data());

    VkSurfaceFormatKHR format = formats.front();
    for (const auto& candidate : formats) {
        if (candidate.format == VK_FORMAT_B8G8R8A8_SRGB) {
            format = candidate;
            break;
        }
    }

    swapchain_format_ = format.format;
    swapchain_extent_ = capabilities.currentExtent;

    VkSwapchainCreateInfoKHR create_info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    create_info.surface = surface_;
    create_info.minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && create_info.minImageCount > capabilities.maxImageCount) {
        create_info.minImageCount = capabilities.maxImageCount;
    }
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = swapchain_extent_;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = {graphics_queue_family_, present_queue_family_};
    if (graphics_queue_family_ != present_queue_family_) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = VK_TRUE;

    vk_check(vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_),
             "Failed to create swapchain.");

    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
    swapchain_images_.resize(image_count);
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());
}

void VulkanContext::create_image_views() {
    swapchain_image_views_.resize(swapchain_images_.size());

    for (size_t i = 0; i < swapchain_images_.size(); ++i) {
        VkImageViewCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        create_info.image = swapchain_images_[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swapchain_format_;
        create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        vk_check(vkCreateImageView(device_, &create_info, nullptr, &swapchain_image_views_[i]),
                 "Failed to create image view.");
    }
}

void VulkanContext::create_render_pass() {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain_format_;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    vk_check(vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_),
             "Failed to create render pass.");
}

void VulkanContext::create_framebuffers() {
    framebuffers_.resize(swapchain_image_views_.size());

    for (size_t i = 0; i < swapchain_image_views_.size(); ++i) {
        VkImageView attachments[] = {swapchain_image_views_[i]};

        VkFramebufferCreateInfo framebuffer_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebuffer_info.renderPass = render_pass_;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = swapchain_extent_.width;
        framebuffer_info.height = swapchain_extent_.height;
        framebuffer_info.layers = 1;

        vk_check(vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &framebuffers_[i]),
                 "Failed to create framebuffer.");
    }
}

void VulkanContext::create_command_pool() {
    VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = graphics_queue_family_;

    vk_check(vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_),
             "Failed to create command pool.");
}

void VulkanContext::create_command_buffers() {
    command_buffers_.resize(kMaxFramesInFlight);

    VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

    vk_check(vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data()),
             "Failed to allocate command buffers.");
}

void VulkanContext::create_sync_objects() {
    image_available_semaphores_.resize(kMaxFramesInFlight);
    render_finished_semaphores_.resize(kMaxFramesInFlight);
    in_flight_fences_.resize(kMaxFramesInFlight);

    VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        vk_check(vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available_semaphores_[i]),
                 "Failed to create image available semaphore.");
        vk_check(vkCreateSemaphore(device_, &semaphore_info, nullptr, &render_finished_semaphores_[i]),
                 "Failed to create render finished semaphore.");
        vk_check(vkCreateFence(device_, &fence_info, nullptr, &in_flight_fences_[i]),
                 "Failed to create in flight fence.");
    }
}

void VulkanContext::create_imgui_descriptor_pool() {
    std::array<VkDescriptorPoolSize, 1> pool_sizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    };

    VkDescriptorPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();

    vk_check(vkCreateDescriptorPool(device_, &pool_info, nullptr, &imgui_descriptor_pool_),
             "Failed to create ImGui descriptor pool.");
}

void VulkanContext::destroy_swapchain_objects() {
    for (auto framebuffer : framebuffers_) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }
    framebuffers_.clear();

    if (render_pass_ != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device_, render_pass_, nullptr);
        render_pass_ = VK_NULL_HANDLE;
    }

    for (auto image_view : swapchain_image_views_) {
        vkDestroyImageView(device_, image_view, nullptr);
    }
    swapchain_image_views_.clear();

    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
}

VulkanContext::QueueFamilies VulkanContext::query_queue_families(VkPhysicalDevice device) const {
    QueueFamilies families;

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queue_families.data());

    for (uint32_t i = 0; i < queue_count; ++i) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            families.graphics = i;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);
        if (present_support == VK_TRUE) {
            families.present = i;
        }

        if (families.graphics.has_value() && families.present.has_value()) {
            break;
        }
    }

    return families;
}

}  // namespace viewer::renderer
