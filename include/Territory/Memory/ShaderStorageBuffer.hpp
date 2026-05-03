#pragma once

#include <cstdint>
#include <cstddef> // 引入 size_t

namespace Territory {

class ShaderStorageBuffer {
public:
    // 构造函数 1：只在显存里开辟空间，不填入初始数据 (适用于 GPU 完全接管计算的场景)
    ShaderStorageBuffer(size_t size);
    
    // 构造函数 2：开辟空间，并把 CPU 侧的初始数据拷进去 (比如初始的粒子种子)
    ShaderStorageBuffer(const void* data, size_t size);
    
    ~ShaderStorageBuffer();

    // 禁用拷贝与赋值，保护 VRAM 句柄
    ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
    ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

    // 核心机制：将这块内存绑定到具体的槽位 (Binding Slot)，以便 Shader 里通过 layout(binding=X) 找到它
    void Bind(uint32_t slot) const;
    void Unbind() const;

    // 允许 CPU 偶尔介入修改数据 (比如玩家用鼠标强行点画了一块领地)
    void SetData(const void* data, size_t size, size_t offset = 0);

    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID;
};

} // namespace Territory