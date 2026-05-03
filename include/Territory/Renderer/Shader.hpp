#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Territory {

class Shader {
public:
    // 构造函数 1：用于传统的图形渲染管线 (Vertex + Fragment)
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    
    // 构造函数 2：用于我们核心的 GPU-Driven 计算管线 (Compute)
    Shader(const std::string& computePath);
    
    // RAII 机制，销毁时释放 GPU 显存中的程序对象
    ~Shader();

    // 禁用拷贝，防止 OpenGL ID 被重复释放
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // 激活该着色器程序 (相当于告诉 GPU：接下来按这个逻辑算)
    void Bind() const;
    void Unbind() const;

    // 向 GPU 传递 Uniform 变量 (CPU 向 GPU 传递少量控制参数的通道)
    void SetInt(const std::string& name, int value);
    void SetInt2(const std::string& name, const glm::ivec2& value);
    void SetFloat(const std::string& name, float value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);

    // 获取底层 OpenGL Program ID
    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID;
    std::unordered_map<std::string, int> m_UniformLocationCache; // 缓存 Uniform 位置，压榨性能

    // 内部帮助函数：读取文件与编译
    std::string ReadFile(const std::string& filepath);
    uint32_t CompileShader(uint32_t type, const std::string& source);
    void CheckCompileErrors(uint32_t shader, const std::string& type);
    int GetUniformLocation(const std::string& name);
};

} // namespace Territory