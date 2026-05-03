#include "Territory/Renderer/InstancedRenderer.hpp"
#include <glad/glad.h>

namespace Territory {

InstancedRenderer::InstancedRenderer() {
    // 定义一个最简单的 2D 单位四边形 (基于 Triangle Strip 画法)
    // 仅包含位置数据 (x, y)，范围从 -0.5 到 0.5
    float quadVertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f
    };

    // 1. 使用 DSA 语法直接创建 VAO(顶点数组对象) 和 VBO(顶点缓冲对象)
    glCreateVertexArrays(1, &m_QuadVAO);
    glCreateBuffers(1, &m_QuadVBO);

    // 2. 将几何数据写入 VBO 显存 (不可变存储，性能最高)
    glNamedBufferStorage(m_QuadVBO, sizeof(quadVertices), quadVertices, 0);

    // 3. 配置 VAO 格式 (告诉 GPU 怎么解读 VBO 里的数据)
    // 绑定点 0，偏移 0，步长 2 * sizeof(float)
    glVertexArrayVertexBuffer(m_QuadVAO, 0, m_QuadVBO, 0, 2 * sizeof(float));
    // 启用 0 号属性 (Location 0)
    glEnableVertexArrayAttrib(m_QuadVAO, 0);
    // 配置为包含 2 个 float 的向量 (vec2)
    glVertexArrayAttribFormat(m_QuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
    // 将 0 号属性链接到 0 号绑定点
    glVertexArrayAttribBinding(m_QuadVAO, 0, 0);
}

InstancedRenderer::~InstancedRenderer() {
    glDeleteBuffers(1, &m_QuadVBO);
    glDeleteVertexArrays(1, &m_QuadVAO);
}

void InstancedRenderer::Draw(std::shared_ptr<Shader> shader, uint32_t instanceCount) const {
    if (instanceCount == 0) return;

    // 激活渲染此批次的材质着色器
    shader->Bind();
    
    // 绑定基础几何体的 VAO
    glBindVertexArray(m_QuadVAO);
    
    // 【终极奥义】一次 API 调用，绘制十万个四边形！
    // 具体的坐标差异，将由 Shader 内部通过 gl_InstanceID 从 SSBO 中自行拉取
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
    
    glBindVertexArray(0);
}

} // namespace Territory