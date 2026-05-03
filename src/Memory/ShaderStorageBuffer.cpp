#include "Territory/Memory/ShaderStorageBuffer.hpp"
#include <glad/glad.h>

namespace Territory {

ShaderStorageBuffer::ShaderStorageBuffer(size_t size) {
    // 使用 OpenGL 4.5 DSA 语法直接创建对象
    glCreateBuffers(1, &m_RendererID);
    
    // GL_DYNAMIC_DRAW 提示显卡驱动：这块内存的数据会被频繁修改和读取
    glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
}

ShaderStorageBuffer::ShaderStorageBuffer(const void* data, size_t size) {
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferData(m_RendererID, size, data, GL_DYNAMIC_DRAW);
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
    glDeleteBuffers(1, &m_RendererID);
}

void ShaderStorageBuffer::Bind(uint32_t slot) const {
    // 极其关键：将整个 Buffer 挂载到指定的 binding point
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_RendererID);
}

void ShaderStorageBuffer::Unbind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset) {
    // 直接拿着 ID 修改特定偏移量的数据，无需 glBindBuffer
    glNamedBufferSubData(m_RendererID, offset, size, data);
}

void ShaderStorageBuffer::ClearInt(int clearValue) {
    // GL_R32I: 内部格式为 32位整型
    // GL_RED_INTEGER, GL_INT: 传入数据的格式和类型
    glClearNamedBufferData(m_RendererID, GL_R32I, GL_RED_INTEGER, GL_INT, &clearValue);
}

} // namespace Territory