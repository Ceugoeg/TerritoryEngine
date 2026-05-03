#include "Territory/Pipeline/ComputeDispatcher.hpp"
#include <glad/glad.h>

namespace Territory {

ComputeDispatcher::ComputeDispatcher(std::shared_ptr<Shader> computeShader)
    : m_ComputeShader(computeShader) {
}

void ComputeDispatcher::Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) {
    // 确保激活对应的计算着色器
    m_ComputeShader->Bind();
    
    // 唤醒显卡流处理器阵列，开始兵分多路执行计算！
    glDispatchCompute(groupX, groupY, groupZ);
}

void ComputeDispatcher::DispatchWithBarrier(uint32_t groupX, uint32_t groupY, uint32_t groupZ) {
    Dispatch(groupX, groupY, groupZ);
    
    // 内存屏障：告诉 GPU，必须等所有发往 SSBO 的写操作彻底完成，才能继续往下走。
    // 这是我们在 GPU 显存内自闭环运行“碰撞演化 -> 渲染”绝不穿模的底气。
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

} // namespace Territory