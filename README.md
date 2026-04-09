# GPU-Accelerated 2D Pixel Renderer

[![GitHub](https://img.shields.io/badge/GitHub-Repository-blue)](https://github.com/xiaohei-5753/2d_renderer)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3%2B-green)](https://www.opengl.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10%2B-blue)](https://cmake.org/)

**English** | [中文](#中文)

A complete GPU-accelerated rewrite of a 2D pixel rendering engine using OpenGL compute shaders. Achieves stable 60 FPS on RTX 4080 while preserving the original pixel-art aesthetic and per-pixel ray tracing characteristics.

## ✨ Features

- **GPU-Parallel Ray Tracing**: OpenGL compute shaders perform light transport calculations for all 128×128 pixels simultaneously
- **Pixel-Perfect Rendering**: 128×128 canvas with individual pixel control, preserving the original pixel-art style
- **Real-Time Interaction**: Mouse drawing, camera pan/zoom, and dynamic rendering
- **Expression-Based Brushes**: Mathematical expression parser for procedural color generation
- **Performance Optimized**: Compute shader architecture separates calculation (128×128) from display (1280×1280)
- **Advanced Camera System**: Smooth 2D camera with panning, zooming, and proper coordinate transformations
- **Screenshot Support**: Press `P` to save timestamped BMP screenshots
- **Flicker-Free Rendering**: Optimized rendering pipeline eliminates camera movement flickering

## 🏗️ Architecture

### Core Components

1. **Renderer Class** (`renderer.h/cpp`) - Manages OpenGL context, shaders, textures, and rendering pipeline
2. **Compute Shader** (`raytrace.comp`) - Parallel ray tracing at canvas resolution (128×128 work groups)
3. **Display Shader** (`display.vert/frag`) - Simple texture display with camera transformation
4. **Pixel Buffer** (`pixel_buffer.h/cpp`) - GPU texture-based storage for pixel colors and light properties
5. **Expression Parser** (`expression.h/cpp`) - Mathematical expression evaluation for brush colors
6. **Texture Management** (`texture.h/cpp`) - OpenGL texture wrapper with compute shader image support

### Data Flow

```
CPU: Input → Pixel Buffer Updates → Texture Upload
       ↑                              ↓
GPU: Texture Data → Compute Shader → Display Shader → Screen
```

### Performance Optimization

- **Compute/Display Separation**: Ray tracing runs only when pixels change, display runs every frame
- **Texture-Based Storage**: Fast GPU memory access for pixel data
- **Optimal Work Group Size**: 16×16 thread groups for 128×128 canvas
- **Memory Barriers**: Proper synchronization between compute and graphics pipelines

## 🐛 Problems Solved

| Problem | Root Cause | Solution |
|---------|------------|----------|
| **Camera Flickering** | Manual render mode prevented display updates when `forceRedraw=false` | Always run display shader, only compute shader when needed |
| **Y-Axis Inversion** | Coordinate transformation functions were incorrect | Fixed functions in `pixel.h` |
| **Performance Issues** | CPU implementation couldn't achieve 60 FPS on RTX 4080 | GPU parallelization using OpenGL compute shaders |
| **Sunlight Overexposure** | Initial implementation had excessively bright sunlight | Temporarily disabled sunlight (`sunColor = (0,0,0,0)`) |
| **Incomplete Ray Directions** | Original used only 8 fixed directions instead of full circle | Implemented Bresenham circle algorithm for all directions |

## 📦 Building

### Dependencies

- **CMake 3.10+**
- **OpenGL 3.3+** (with compute shader support)
- **GLFW 3.3+** (window and input management)
- **GLEW/GLAD** (OpenGL extension loading)
- **GLM** (OpenGL mathematics)

### Build Instructions

```bash
# Clone repository
git clone https://github.com/xiaohei-5753/2d_renderer.git
cd 2d_renderer

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release

# Run the application
./PixelRenderer2D
```

## 🎮 Usage

### Controls

- **Left Mouse Button**: Draw with current brush expression
- **Right Mouse Button**: Erase (set pixel to transparent)
- **Arrow Keys**: Pan camera (Up/Down/Left/Right)
- **Ctrl + Arrow Up/Down**: Zoom in/out
- **R**: Reset rendering state (clear ray tracing cache)
- **P**: Save screenshot with timestamp
- **O**: Enter new brush expression (console input)
- **N**: Clear canvas (reset all pixels to transparent)
- **Ctrl + 0**: Reset camera to default position
- **ESC**: Exit application

### Brush Expressions

The brush system supports mathematical expressions with variables `x` and `y` (normalized coordinates 0-1):

- **Format**: `~ r g b a lr lg lb` (each component is an expression)
- **Example**: `~ x y 0 1 0 0 0` creates a gradient from red to green
- **Default**: `~ x 1 y 1 x 1 y` (complex color pattern)

### Performance Metrics

| Metric | Original (CPU) | Optimized (GPU) |
|--------|----------------|-----------------|
| **Frame Rate** | < 10 FPS | **60 FPS** (stable) |
| **Ray Tracing** | Sequential (8 rays/pixel) | **Parallel** (128×128 simultaneous) |
| **Memory Access** | System RAM | **GPU Texture Memory** |
| **Rendering Complexity** | O(n²) per frame | **O(1)** display + on-demand compute |

## 📁 File Structure

```
2d_renderer/
├── CMakeLists.txt          # Build configuration
├── README.md              # This documentation
├── LICENSE               # GNU GPL v3 license
├── assets/               # Optional asset files
├── include/              # Header files
│   ├── expression.h      # Mathematical expression parser
│   ├── pixel.h          # Pixel structure and coordinate conversion
│   ├── pixel_buffer.h   # GPU pixel buffer management
│   ├── renderer.h       # Main renderer class
│   ├── shader.h         # OpenGL shader wrapper
│   └── texture.h        # OpenGL texture wrapper
├── src/                  # Source files
│   ├── expression.cpp   # Expression implementation
│   ├── main.cpp         # Application entry point
│   ├── pixel_buffer.cpp # Pixel buffer implementation
│   ├── renderer.cpp     # Renderer implementation
│   └── shader.cpp       # Shader implementation
└── shaders/             # GLSL shader files
    ├── display.frag     # Display fragment shader
    ├── display.vert     # Display vertex shader
    ├── raytrace.comp    # Compute shader (ray tracing)
    ├── raytrace.frag    # Legacy fragment shader
    └── raytrace.vert    # Legacy vertex shader
```

## 📄 License

This project is licensed under the **GNU General Public License v3.0** - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Issues and Pull Requests are welcome! Please ensure you follow the GPL v3 license terms.

---

## 中文

### 概述

这是一个完整的GPU加速重写的2D像素渲染引擎，原始版本为CPU实现。项目利用OpenGL计算着色器进行并行光线追踪计算，在RTX 4080上实现**60 FPS稳定性能**，同时保持原始像素艺术美学和逐像素光线追踪特性。

### ✨ 主要特性

- **GPU并行光线追踪**：OpenGL计算着色器同时计算128×128像素的光线传输
- **像素级精确渲染**：128×128画布，独立像素控制，保持原始像素艺术风格
- **实时交互**：鼠标绘制、相机平移/缩放、动态渲染
- **表达式画笔**：数学表达式解析器，支持程序化颜色生成（如`~ x 1 y 1 x 1 y`）
- **性能优化**：计算着色器架构将计算（128×128）与显示（1280×1280）分离
- **高级相机系统**：平滑的2D相机，支持平移、缩放和正确的坐标转换
- **截图支持**：按`P`键保存带时间戳的BMP截图
- **无闪烁渲染**：优化的渲染管道消除相机移动时的闪烁问题

### 🏗️ 技术架构

#### 核心组件

1. **渲染器类** (`renderer.h/cpp`) - 管理OpenGL上下文、着色器、纹理和渲染管道
2. **计算着色器** (`raytrace.comp`) - 在画布分辨率（128×128工作组）下并行光线追踪
3. **显示着色器** (`display.vert/frag`) - 带相机变换的简单纹理显示
4. **像素缓冲区** (`pixel_buffer.h/cpp`) - 基于GPU纹理的像素颜色和光属性存储
5. **表达式解析器** (`expression.h/cpp`) - 画笔颜色的数学表达式求值
6. **纹理管理** (`texture.h/cpp`) - 支持计算着色器图像的OpenGL纹理包装器

#### 数据流

```
CPU: 输入 → 像素缓冲区更新 → 纹理上传
       ↑                          ↓
GPU: 纹理数据 → 计算着色器 → 显示着色器 → 屏幕
```

#### 性能优化

- **计算/显示分离**：仅当像素变化时运行光线追踪，每帧都运行显示
- **基于纹理的存储**：快速的GPU内存访问像素数据
- **优化的工作组大小**：128×128画布使用16×16线程组
- **内存屏障**：计算和图形管道之间的正确同步

### 🐛 解决的问题

| 问题 | 根本原因 | 解决方案 |
|------|----------|----------|
| **相机闪烁** | 手动渲染模式在`forceRedraw=false`时阻止显示更新 | 始终运行显示着色器，仅在需要时运行计算着色器 |
| **Y轴反转** | 坐标转换函数不正确 | 修复`pixel.h`中的函数 |
| **性能问题** | CPU实现在RTX 4080上无法达到60 FPS | 使用OpenGL计算着色器进行GPU并行化 |
| **阳光过曝** | 初始实现阳光过亮 | 暂时禁用阳光（`sunColor = (0,0,0,0)`） |
| **不完整的光线方向** | 原始版本仅使用8个固定方向而非完整圆 | 实现Bresenham圆算法支持所有方向 |

### 📦 构建

#### 依赖项

- **CMake 3.10+**
- **OpenGL 3.3+**（支持计算着色器）
- **GLFW 3.3+**（窗口和输入管理）
- **GLEW/GLAD**（OpenGL扩展加载）
- **GLM**（OpenGL数学库）

#### 构建指令

```bash
# 克隆仓库
git clone https://github.com/xiaohei-5753/2d_renderer.git
cd 2d_renderer

# 创建构建目录
mkdir build && cd build

# 使用CMake配置
cmake ..

# 构建项目
cmake --build . --config Release

# 运行应用程序
./PixelRenderer2D
```

### 🎮 使用方法

#### 控制键

- **鼠标左键**：使用当前画笔表达式绘制
- **鼠标右键**：擦除（将像素设置为透明）
- **方向键**：平移相机（上/下/左/右）
- **Ctrl + 上/下方向键**：放大/缩小
- **R**：重置渲染状态（清除光线追踪缓存）
- **P**：保存带时间戳的截图
- **O**：输入新画笔表达式（控制台输入）
- **N**：清空画布（将所有像素重置为透明）
- **Ctrl + 0**：将相机重置到默认位置
- **ESC**：退出应用程序

#### 画笔表达式

画笔系统支持包含变量`x`和`y`的数学表达式（归一化坐标0-1）：

- **格式**：`~ r g b a lr lg lb`（每个组件都是一个表达式）
- **示例**：`~ x y 0 1 0 0 0`创建从红色到绿色的渐变
- **默认**：`~ x 1 y 1 x 1 y`（复杂颜色模式）

#### 性能指标

| 指标 | 原始版本（CPU） | 优化版本（GPU） |
|------|----------------|----------------|
| **帧率** | < 10 FPS | **60 FPS**（稳定） |
| **光线追踪** | 顺序（每个像素8条光线） | **并行**（128×128同时计算） |
| **内存访问** | 系统RAM | **GPU纹理内存** |
| **渲染复杂度** | 每帧O(n²) | **O(1)**显示 + 按需计算 |

### 📁 文件结构

```
2d_renderer/
├── CMakeLists.txt          # 构建配置
├── README.md              # 本文档
├── LICENSE               # GNU GPL v3许可证
├── assets/               # 可选资源文件
├── include/              # 头文件
│   ├── expression.h      # 数学表达式解析器
│   ├── pixel.h          # 像素结构和坐标转换
│   ├── pixel_buffer.h   # GPU像素缓冲区管理
│   ├── renderer.h       # 主渲染器类
│   ├── shader.h         # OpenGL着色器包装器
│   └── texture.h        # OpenGL纹理包装器
├── src/                  # 源文件
│   ├── expression.cpp   # 表达式实现
│   ├── main.cpp         # 应用程序入口点
│   ├── pixel_buffer.cpp # 像素缓冲区实现
│   ├── renderer.cpp     # 渲染器实现
│   └── shader.cpp       # 着色器实现
└── shaders/             # GLSL着色器文件
    ├── display.frag     # 显示片段着色器
    ├── display.vert     # 显示顶点着色器
    ├── raytrace.comp    # 计算着色器（光线追踪）
    ├── raytrace.frag    # 旧版片段着色器
    └── raytrace.vert    # 旧版顶点着色器
```

### 📄 许可证

本项目采用 **GNU通用公共许可证v3.0** - 详见 [LICENSE](LICENSE) 文件。

### 🤝 贡献

欢迎提交Issue和Pull Request！请确保遵循GPL v3许可证条款。

---

### 📊 项目状态

- ✅ **完成并经过测试**
- ✅ **达到60 FPS稳定运行**
- ✅ **所有关键BUG已解决**
- ✅ **遵循现代C++和OpenGL最佳实践**

享受创建像素艺术的乐趣吧！🎨✨