# TerritoryEngine: Heterogeneous GPU-Driven Simulation Framework

## Overview
TerritoryEngine 是一款基于 C++17 构建的高性能 2D 异构仿真底层框架。针对海量实体碰撞与领地演化场景，引擎采用全链路 **GPU-Driven Pipeline**。通过将计算逻辑、空间结构与渲染数据完全驻留于显存，本框架能在普通消费级显卡上轻松实现十万级粒子并发与全屏点对点像素网格的实时推演。

## Current Architecture & Features (v0.1)

目前引擎已跑通第一阶段的核心闭环，主要依托 Compute Shader 实现：

### 1. GPU-Driven Pipeline & Kamikaze Paint
* **零回读渲染:** 物理运算与领地状态覆写全在 VRAM 内闭环。利用神级空 VAO 技巧直接利用 Fragment Shader 映射 SSBO 渲染底图，彻底斩断 PCIe 总线瓶颈。
* **自杀式涂地逻辑:** 粒子具备阵营感知，触碰敌占区后通过 `atomicExchange` 实现极速状态覆写，并执行“墓地传送”与“母巢重生”状态机逻辑。

### 2. O(1) Spatial Hashing (Atomic Linked Lists)
* 抛弃低效的全局遍历，在显卡内部实时构建覆盖全图的网格哈希表。通过 `atomicExchange` 极速争夺 Grid Head 建立无锁虚拟链表，将十万粒子的邻域碰撞复杂度降维至 $O(1)$。

## Future Roadmap

随着引擎架构的演进，未来将逐步解锁以下硬核特性：

* [ ] **Z-Morton & Radix Sort:** 引入 Z-Order 曲线与基数排序，重构显存物理地址，进一步压榨 L1/L2 Cache 性能。
* [ ] **Pachinko Physics System:** 构建带有独立重力室与复杂几何体碰撞求交的副物理管线。
* [ ] **Tensor Core CA:** 底层调用显卡 Tensor Core 执行 MMA（混合精度矩阵乘加），实现降维加速的元胞自动机领地演化。
* [ ] **SPH Advanced Physics:** 顺势接入平滑粒子流体动力学（SPH），赋予粒子表面张力与不可压缩性。
* [ ] **Data-Driven Level Editor:** 创意工坊支持，实现关卡数据的动态解析。

## Directory Framework

```text
TerritoryEngine/
├── CMakeLists.txt                
├── third_party/                  
├── include/Territory/            
│   ├── Core/Application.hpp      
│   ├── Memory/ShaderStorageBuffer.hpp 
│   ├── Pipeline/ComputeDispatcher.hpp 
│   └── Renderer/InstancedRenderer.hpp 
├── src/                          
├── assets/shaders/               
│   ├── compute/
│   │   ├── particles.comp        # 核心物理与生命周期状态机
│   │   └── spatial_hash.comp     # GPU 原子链表构建
│   └── graphics/
│       ├── instanced.vert / .frag # 批量拉取 VRAM 数据的粒子渲染
│       └── terrain.vert / .frag   # 空 VAO 全屏底图映射渲染
└── examples/                     
    └── ColorWar/                 
        └── main.cpp              # 核心宿主调度端
```

## Build Instructions

本项目严格遵循现代 CMake 工程规范，原生支持并推荐在 Linux 环境下进行开发与编译。

### Prerequisites
- Compiler: GCC 9+ 或 Clang 10+ (Fully C++17 compliant)
- Build System: CMake 3.15+
- Graphics API: Vulkan SDK 1.2+ 或 OpenGL 4.3+ (需支持 Compute Shader 与 SSBO)
- Libraries: GLFW3, GLM

### Build Steps
```bash
# 1. Clone the repository
git clone [https://github.com/Ceugoeg/TerritoryEngine.git](https://github.com/Ceugoeg/TerritoryEngine.git)
cd TerritoryEngine

# 2. Generate build files
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. Compile the engine and examples
make -j$(nproc)
```

## Directory Structure
- `include/Territory/` - 对外暴露的核心 API，包括 `ComputeDispatcher` 与 `BufferManager` 等接口。
- `src/` - 引擎内部实现，涵盖 GPU Radix Sort 算法封装、Tensor Core 矩阵映射逻辑等。
- `assets/shaders/` - 核心的 Compute Shaders，包含 `morton_hash.comp`, `particle_kinematics.comp`, `tensor_ca_update.comp`。
- `examples/` - 包含 "四色领地争夺" 等海量实体交互演示程序的源码。

## License
MIT License.