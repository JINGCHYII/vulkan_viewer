cmake_minimum_required(VERSION 3.24)
project(vulkan_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_IMGUI_DOCKING "Use Dear ImGui docking branch" ON)

find_package(Vulkan 1.4.321 REQUIRED)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

if(USE_IMGUI_DOCKING)
  set(IMGUI_TAG docking)
else()
  set(IMGUI_TAG v1.91.9b)
endif()

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG ${IMGUI_TAG}
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  tinygltf
  GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
  GIT_TAG v2.9.3
)
FetchContent_MakeAvailable(tinygltf)

add_library(imgui_backend STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui_backend PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_backend PUBLIC glfw Vulkan::Vulkan)

target_compile_definitions(imgui_backend PUBLIC
  GLFW_INCLUDE_NONE
)

add_executable(vulkan_viewer
  src/main.cpp
  src/renderer/vulkan_context.cpp
  src/renderer/render_passes.cpp
  src/renderer/resource_manager.cpp
)

target_include_directories(vulkan_viewer PRIVATE
  src
  ${tinygltf_SOURCE_DIR}
)

target_link_libraries(vulkan_viewer PRIVATE
  glfw
  Vulkan::Vulkan
  imgui_backend
)

target_compile_definitions(vulkan_viewer PRIVATE
  TINYGLTF_NO_STB_IMAGE_WRITE
)
