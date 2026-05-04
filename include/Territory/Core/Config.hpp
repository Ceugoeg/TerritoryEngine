#pragma once
#include <cstdint>

namespace Territory {

    struct SimulationConfig {
        // 窗口设置
        int WindowWidth = 1280;
        int WindowHeight = 720;
        
        // --- 领地管线 (A轨) 参数 ---
        uint32_t ParticleCount = 100000;
        int GridResolution = 128;
        
        // --- 柏青哥管线 (B轨) 参数 ---
        uint32_t PachinkoBallCount = 500;  // 500 个受重力影响的小球
        uint32_t PachinkoPegCount = 200;   // 200 个静态钉子障碍物
        float Gravity = 0.8f;              // 向下的重力加速度
    };

} // namespace Territory