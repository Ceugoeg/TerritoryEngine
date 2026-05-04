#pragma once
#include <cstdint>

namespace Territory {

    // 引擎的全局仿真配置，未来可由 JSON 文件反序列化生成
    struct SimulationConfig {
        // 窗口设置
        int WindowWidth = 1280;
        int WindowHeight = 720;
        
        // 核心仿真参数
        uint32_t ParticleCount = 100000;
        int GridResolution = 128; // 空间哈希网格划分精度
        
        // 柏青哥物理室预留参数 (未来可扩充)
        // float Gravity = 9.8f;
        // float Bounciness = 0.8f;
    };

} // namespace Territory