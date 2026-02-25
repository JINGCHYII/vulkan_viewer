#pragma once

#include "asset/gltf_loader.h"
#include "renderer/renderer.h"
#include "ui/imgui_layer.h"

#include <memory>
#include <optional>
#include <string>

struct GLFWwindow;

namespace viewer {

class App {
  public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();
    void setStartupModelPath(std::string path);

  private:
    void initWindow();
    void initSystems();
    void processUI();
    void drawFrame();
    void cleanup();

    GLFWwindow* window_ = nullptr;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<ImGuiLayer> imgui_;
    std::unique_ptr<GltfLoader> gltfLoader_;

    std::optional<std::string> startupModelPath_;
    bool requestQuit_ = false;
};

} // namespace viewer
