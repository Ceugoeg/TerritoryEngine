#pragma once

#include <memory>
#include <cstdint>
#include "Territory/Renderer/Shader.hpp"

namespace Territory {

class ComputeDispatcher {
public:
    // 构造时绑定一个专用的计算着色器
    ComputeDispatcher(std::shared_ptr<Shader> computeShader);
    ~ComputeDispatcher() = default;

    // 纯异步派发：极速下发指令，不等待结果。适用于前后步骤完全不相干的场景
    void Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ = 1);

    // 同步派发（极其重要）：派发后插入 GL_SHADER_STORAGE_BARRIER_BIT 屏障。
    // 强制保障当前步骤对 SSBO 的写入全部落盘后，才允许后续的绘制或计算管线继续执行。
    void DispatchWithBarrier(uint32_t groupX, uint32_t groupY, uint32_t groupZ = 1);

    // 暴露底层着色器，方便上层游戏随时更新 Uniform 变量 (如传入 delta_time)
    std::shared_ptr<Shader> GetShader() const { return m_ComputeShader; }

private:
    std::shared_ptr<Shader> m_ComputeShader;
};

} // namespace Territory