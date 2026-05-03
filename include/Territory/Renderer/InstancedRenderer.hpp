#pragma once

#include <memory>
#include <cstdint>
#include "Territory/Renderer/Shader.hpp"

namespace Territory {

class InstancedRenderer {
public:
    // 构造时在显存中生成一个最基础的 2D 几何体（比如单位四边形）
    InstancedRenderer();
    ~InstancedRenderer();

    // 禁用拷贝与赋值，保护 VAO/VBO 句柄
    InstancedRenderer(const InstancedRenderer&) = delete;
    InstancedRenderer& operator=(const InstancedRenderer&) = delete;

    // 核心绘制指令：一键拉取显存数据，渲染海量实体
    void Draw(std::shared_ptr<Shader> shader, uint32_t instanceCount) const;

private:
    uint32_t m_QuadVAO = 0;
    uint32_t m_QuadVBO = 0;
};

} // namespace Territory