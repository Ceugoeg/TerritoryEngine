# TerritoryEngine: Heterogeneous GPU-Driven Simulation Framework

## Overview
TerritoryEngine 是一款基于 C++17 构建的极高性能 2D 异构仿真底层框架。针对海量实体碰撞、复杂流体动力学以及网格领地演化场景，引擎采用全链路 **GPU-Driven Pipeline**，彻底摒弃 CPU 轮询。通过将计算逻辑、稀疏空间结构与渲染数据完全驻留于显存，并在底层融合张量核心计算与物理级时钟调度，本框架能在普通消费级显卡上实现十万级粒子并发与千万级稀疏网格的实时推演。

## Core Architecture & Optimizations

本引擎的底层驱动由六大极致优化策略深度融合而成，形成完整的算力闭环：

### 1. Data Flow & Execution
* **GPU-Driven Pipeline:** 世界状态不占用主存，逻辑步进与状态覆写全在 VRAM 内通过 Compute Shader 闭环，实现“计算即渲染”，彻底斩断 PCIe 总线瓶颈。
* **Adaptive Spatial Tick (LOD):** 引入空间时钟调度，基于视锥体剔除与活跃度休眠机制，动态冻结非活跃区域的计算管线，将宝贵的 GPU 算力倾斜至碰撞激烈的核心战区。

### 2. Spatial Indexing & Memory
* **Z-Morton Spatial Hashing:** 在显存中实时计算粒子的 Z-Order 码并调用底层 Radix Sort 重构数组，确保物理相邻的实体在 VRAM 物理地址上绝对连续，极大提升 L1/L2 Cache 命中率。
* **Sparse Virtual Data Structures (2D VDB):** 摒弃全局密集网格，采用多级哈希页表机制，仅对发生着色演化的“活跃领地区块”动态分配显存，突破硬件 VRAM 限制，支撑无缝大世界。

### 3. Simulation & Physics
* **Tensor Core CA (Cellular Automata):** 将领地状态网格的扩张、腐蚀规则抽象为多维张量卷积，底层调用显卡 Tensor Core 执行混合精度矩阵乘加运算（MMA），实现降维加速。
* **SPH Advanced Physics:** 依托空间哈希提供的极速 $O(1)$ 邻域查询，顺势接入平滑粒子流体动力学（SPH），赋予碰撞粒子粘性、表面张力与不可压缩性等真实的流体质感。

## Directory Framework

整个工程遵循严格的模块化解耦，核心计算着色器与 C++ 宿主逻辑分离：

```
TerritoryEngine/
├── CMakeLists.txt                # 顶层 CMake 构建脚本 (C++17 强制要求)
├── README.md                     # 项目说明与架构蓝图
├── third_party/                  # 外部依赖 (GLFW, GLAD/VulkanHpp, glm)
├── include/Territory/            # 核心 API (宿主端控制层)
│   ├── Core/
│   │   ├── Math.hpp              # 数学基础设施
│   │   └── Application.hpp       # 主循环与上下文生命周期
│   ├── Memory/
│   │   ├── SparsePageTable.hpp   # 宿主端虚拟稀疏页表管理器
│   │   └── BufferAllocator.hpp   # GPU 显存池分配器
│   ├── Pipeline/
│   │   ├── ComputeDispatcher.hpp # 异构计算调度指令分发
│   │   └── IntentQueue.hpp       # 无锁意图队列 (处理少量来自 CPU 的玩家输入)
│   └── Render/
│       └── InstancedRenderer.hpp # 间接绘制管线封装
├── src/                          # C++ 核心源码实现
│   ├── Core/
│   ├── Memory/
│   ├── Pipeline/
│   └── Render/
├── assets/shaders/               # 引擎灵魂：GPU 计算与渲染核心
│   ├── math/
│   │   └── morton_codes.glsl     # Z-Morton 编码与解码函数库
│   ├── compute/
│   │   ├── radix_sort.comp       # 全 GPU 并行基数排序
│   │   ├── sph_physics.comp      # SPH 流体力学积分与碰撞解算
│   │   └── tensor_ca.comp        # Tensor Core 调用的元胞自动机卷积核
│   └── graphics/
│       ├── instanced.vert        # 批量拉取 VRAM 数据顶点着色器
│       └── sparse_grid.frag      # 稀疏网格着色器
└── examples/                     # 测试用例
    ├── FluidWar/                 # 结合 SPH 流体与领地争夺的具体游戏逻辑
    └── main.cpp
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