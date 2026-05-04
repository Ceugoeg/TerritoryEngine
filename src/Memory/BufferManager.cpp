#include "Territory/Memory/BufferManager.hpp"
#include <iostream>

namespace Territory {

std::shared_ptr<ShaderStorageBuffer> BufferManager::Create(const std::string& name, size_t size, const void* data) {
    if (m_Buffers.find(name) != m_Buffers.end()) {
        std::cerr << "[BufferManager] Warning: Buffer '" << name << "' already exists! Overwriting...\n";
    }
    
    auto buffer = data ? std::make_shared<ShaderStorageBuffer>(data, size) 
                       : std::make_shared<ShaderStorageBuffer>(size);
                       
    m_Buffers[name] = buffer;
    return buffer;
}

std::shared_ptr<ShaderStorageBuffer> BufferManager::GetBuffer(const std::string& name) {
    auto it = m_Buffers.find(name);
    if (it != m_Buffers.end()) {
        return it->second;
    }
    std::cerr << "[BufferManager] Error: Buffer '" << name << "' not found!\n";
    return nullptr;
}

void BufferManager::Bind(const std::string& name, uint32_t slot) {
    auto buffer = GetBuffer(name);
    if (buffer) {
        buffer->Bind(slot);
    }
}

void BufferManager::Clear() {
    m_Buffers.clear();
}

} // namespace Territory