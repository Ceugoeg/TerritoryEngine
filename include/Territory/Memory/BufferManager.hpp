#pragma once

#include "Territory/Memory/ShaderStorageBuffer.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>

namespace Territory {

class BufferManager {
public:
    // 经典的单例模式，全局唯一的显存管家
    static BufferManager& Get() {
        static BufferManager instance;
        return instance;
    }

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    // 根据字符串 ID 创建显存块
    std::shared_ptr<ShaderStorageBuffer> Create(const std::string& name, size_t size, const void* data = nullptr);
    
    // 获取显存块引用
    std::shared_ptr<ShaderStorageBuffer> GetBuffer(const std::string& name);
    
    // 极速绑定通道
    void Bind(const std::string& name, uint32_t slot);
    
    // 引擎退出或切换关卡时清理显存
    void Clear();

private:
    BufferManager() = default;
    
    // 底层数据结构：一个通过字符串映射到 SSBO 的哈希表
    std::unordered_map<std::string, std::shared_ptr<ShaderStorageBuffer>> m_Buffers;
};

} // namespace Territory