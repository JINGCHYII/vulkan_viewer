cmake_minimum_required(VERSION 3.24)
project(vulkan_viewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VULKAN_VIEWER_BUILD_SHADERS "Compile GLSL shaders to SPIR-V" ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(vulkan_viewer
    src/main.cpp
    src/app.cpp
    src/app.h
    src/renderer/vulkan_context.cpp
    src/renderer/vulkan_context.h
    src/renderer/renderer.cpp
    src/renderer/renderer.h
    src/asset/gltf_loader.cpp
    src/asset/gltf_loader.h
    src/ui/imgui_layer.cpp
    src/ui/imgui_layer.h
)

target_include_directories(vulkan_viewer PRIVATE src)
target_link_libraries(vulkan_viewer PRIVATE Vulkan::Vulkan glfw)

target_compile_definitions(vulkan_viewer PRIVATE VK_USE_PLATFORM_WIN32_KHR=0)

# ImGui and tinygltf are expected as vendored subdirectories.
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/CMakeLists.txt")
    add_subdirectory(third_party/imgui)
    target_link_libraries(vulkan_viewer PRIVATE imgui)
    target_include_directories(vulkan_viewer PRIVATE third_party/imgui)
else()
    message(WARNING "third_party/imgui not found; build will fail until dependency is added")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/tinygltf")
    target_include_directories(vulkan_viewer PRIVATE third_party/tinygltf)
    target_compile_definitions(vulkan_viewer PRIVATE TINYGLTF_NO_STB_IMAGE_WRITE)
else()
    message(WARNING "third_party/tinygltf not found; glTF loading is disabled")
    target_compile_definitions(vulkan_viewer PRIVATE VULKAN_VIEWER_DISABLE_GLTF)
endif()

if(VULKAN_VIEWER_BUILD_SHADERS)
    find_program(GLSLANG_VALIDATOR glslangValidator)
    if(GLSLANG_VALIDATOR)
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
        set(SHADER_SOURCES
            shaders/pbr.vert
            shaders/pbr.frag
        )
        foreach(shader ${SHADER_SOURCES})
            get_filename_component(shader_name ${shader} NAME)
            add_custom_command(
                OUTPUT "${CMAKE_BINARY_DIR}/shaders/${shader_name}.spv"
                COMMAND ${GLSLANG_VALIDATOR} -V "${CMAKE_SOURCE_DIR}/${shader}" -o "${CMAKE_BINARY_DIR}/shaders/${shader_name}.spv"
                DEPENDS "${CMAKE_SOURCE_DIR}/${shader}"
                COMMENT "Compiling shader ${shader_name}"
                VERBATIM
            )
            list(APPEND SHADER_OUTPUTS "${CMAKE_BINARY_DIR}/shaders/${shader_name}.spv")
        endforeach()
        add_custom_target(compile_shaders ALL DEPENDS ${SHADER_OUTPUTS})
        add_dependencies(vulkan_viewer compile_shaders)
    else()
        message(WARNING "glslangValidator not found; shader compilation skipped")
    endif()
endif()
