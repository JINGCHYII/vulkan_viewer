#include "renderer/render_passes.hpp"
#include "renderer/resource_manager.hpp"
#include "renderer/vulkan_context.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace {

constexpr int kWindowWidth = 1600;
constexpr int kWindowHeight = 900;

}  // namespace

int main() {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "vulkan_viewer", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    try {
        viewer::renderer::VulkanContext vk_context;
        vk_context.initialize(window);

        viewer::renderer::ResourceManager resource_manager;
        resource_manager.initialize(vk_context.physical_device(), vk_context.device());

        viewer::renderer::RenderPasses render_passes;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.Instance = vk_context.instance();
        init_info.PhysicalDevice = vk_context.physical_device();
        init_info.Device = vk_context.device();
        init_info.QueueFamily = vk_context.graphics_queue_family();
        init_info.Queue = vk_context.graphics_queue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = vk_context.imgui_descriptor_pool();
        init_info.RenderPass = vk_context.render_pass();
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;

        ImGui_ImplVulkan_Init(&init_info);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool show_demo_window = true;
            ImGui::ShowDemoWindow(&show_demo_window);

            auto frame = vk_context.begin_frame(0.09F, 0.1F, 0.16F, 1.0F);

            render_passes.geometry_pass(frame);
            render_passes.tonemap_pass(frame);
            render_passes.ui_pass(frame);

            vk_context.end_frame(frame);
        }

        vkDeviceWaitIdle(vk_context.device());

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        resource_manager.cleanup();
        vk_context.cleanup();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
