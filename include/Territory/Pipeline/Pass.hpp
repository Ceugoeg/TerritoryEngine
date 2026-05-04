#pragma once
#include <string>

namespace Territory {

    // 所有计算、渲染通道的绝对基类
    class Pass {
    public:
        Pass(const std::string& name) : m_Name(name) {}
        virtual ~Pass() = default;

        // 核心接口：每个 Pass 自己决定怎么绑定显存、怎么调度 Shader
        virtual void Execute(float deltaTime) = 0;

        const std::string& GetName() const { return m_Name; }

    protected:
        std::string m_Name;
    };

} // namespace Territory