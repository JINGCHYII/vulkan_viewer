# Vulkan glTF Viewer (Vulkan 1.4.321)

一个基础的 Vulkan + ImGui glTF PBR 渲染器工程骨架，目标 API 版本为 **Vulkan 1.4.321**。

## 功能范围（当前）

- Vulkan 上下文初始化（实例、物理设备、逻辑设备）
- ImGui 基础面板（加载 glTF 路径、渲染调参项）
- glTF 加载入口（`tinygltf`）
- PBR Shader（metallic-roughness 基础 BRDF）
- CMake shader 编译管线（`glslangValidator`）

> 当前仓库是“可扩展基础框架”，完整渲染通路（交换链、命令缓冲录制、descriptor 管理、IBL 预计算）仍在 TODO 列表中。

## 依赖

- Vulkan SDK 1.4.321
- CMake >= 3.24
- C++20 编译器
- GLFW3
- third_party/imgui
- third_party/tinygltf

## 构建

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## 运行

```bash
./build/vulkan_viewer assets/DamagedHelmet/DamagedHelmet.gltf
```

## 下一步建议

1. 完成交换链与帧同步对象初始化。
2. 完成 glTF mesh primitive 顶点/索引解包。
3. 接入 descriptor allocator 与材质纹理上传。
4. 增加 IBL（三贴图）与 BRDF LUT。
5. 接入 ImGui backend（GLFW + Vulkan）并完成 draw data 提交。
