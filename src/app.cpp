#include "app.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

namespace viewer {

namespace {
constexpr int kWindowWidth = 1600;
constexpr int kWindowHeight = 900;
}

App::App() = default;

App::~App() {
    cleanup();
}

void App::setStartupModelPath(std::string path) {
    startupModelPath_ = std::move(path);
}

void App::run() {
    initWindow();
    initSystems();

    if (startupModelPath_) {
        auto scene = gltfLoader_->load(*startupModelPath_);
        renderer_->setScene(std::move(scene));
    }

    while (!glfwWindowShouldClose(window_) && !requestQuit_) {
        glfwPollEvents();
        processUI();
        drawFrame();
    }
}

void App::initWindow() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(kWindowWidth, kWindowHeight, "Vulkan glTF Viewer", nullptr, nullptr);
    if (window_ == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
    }
}

void App::initSystems() {
    renderer_ = std::make_unique<Renderer>();
    renderer_->initialize(window_);

    imgui_ = std::make_unique<ImGuiLayer>();
    imgui_->initialize(window_, renderer_->context());

    gltfLoader_ = std::make_unique<GltfLoader>();
}

void App::processUI() {
    imgui_->beginFrame();

    const auto request = imgui_->drawScenePanel();
    if (request.type == UiRequest::Type::LoadModel && !request.path.empty()) {
        auto scene = gltfLoader_->load(request.path);
        renderer_->setScene(std::move(scene));
    } else if (request.type == UiRequest::Type::Quit) {
        requestQuit_ = true;
    }

    imgui_->drawRendererPanel(renderer_->debugState());
    imgui_->endFrame();
}

void App::drawFrame() {
    renderer_->render(*imgui_);
}

void App::cleanup() {
    if (imgui_) {
        imgui_->shutdown();
        imgui_.reset();
    }

    if (renderer_) {
        renderer_->shutdown();
        renderer_.reset();
    }

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    glfwTerminate();
}

} // namespace viewer
