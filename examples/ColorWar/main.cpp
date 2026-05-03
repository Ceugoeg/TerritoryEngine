#include <Territory/Core/Application.hpp>
#include <Territory/Renderer/Shader.hpp>
#include <Territory/Memory/ShaderStorageBuffer.hpp>
#include <Territory/Pipeline/ComputeDispatcher.hpp>
#include <Territory/Renderer/InstancedRenderer.hpp>

#include <iostream>
#include <vector>
#include <random>
#include <glad/glad.h>

// 保持 std430 内存布局一致
struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
};

class ColorWarApp : public Territory::Application {
public:
    ColorWarApp(const Territory::WindowProps& props) 
        : Territory::Application(props), 
          m_ParticleCount(100000), 
          m_GridRes(128) {} // 128x128 的网格划分

protected:
    void OnInit() override {
        std::cout << "[ColorWar] Initializing GPU-Driven Spatial Pipeline...\n";

        // 1. 初始化十万粒子数据
        std::vector<Particle> initialParticles(m_ParticleCount);
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> distPos(-0.9f, 0.9f);
        std::uniform_real_distribution<float> distVel(-0.3f, 0.3f);

        for (int i = 0; i < m_ParticleCount; ++i) {
            initialParticles[i].position = { distPos(rng), distPos(rng) };
            initialParticles[i].velocity = { distVel(rng), distVel(rng) };
            int team = i % 3;
            initialParticles[i].color = {
                team == 0 ? 0.9f : 0.1f,
                team == 1 ? 0.9f : 0.1f,
                team == 2 ? 0.9f : 0.1f,
                1.0f
            };
        }

        // 2. 分配三大核心 SSBO
        // 槽位 0: 粒子池
        m_ParticleSSBO = std::make_shared<Territory::ShaderStorageBuffer>(
            initialParticles.data(), 
            m_ParticleCount * sizeof(Particle)
        );
        // 槽位 1: 网格头 (128x128 个 int)
        m_GridHeadSSBO = std::make_shared<Territory::ShaderStorageBuffer>(
            m_GridRes * m_GridRes * sizeof(int)
        );
        // 槽位 2: 链表索引 (100,000 个 int)
        m_ParticleListSSBO = std::make_shared<Territory::ShaderStorageBuffer>(
            m_ParticleCount * sizeof(int)
        );

        // 3. 编译着色器
        m_SpatialHashShader = std::make_shared<Territory::Shader>("assets/shaders/compute/spatial_hash.comp");
        m_PhysicsShader = std::make_shared<Territory::Shader>("assets/shaders/compute/particles.comp");
        m_RenderShader = std::make_shared<Territory::Shader>(
            "assets/shaders/graphics/instanced.vert",
            "assets/shaders/graphics/instanced.frag"
        );

        // 4. 初始化组件
        m_HashDispatcher = std::make_unique<Territory::ComputeDispatcher>(m_SpatialHashShader);
        m_PhysicsDispatcher = std::make_unique<Territory::ComputeDispatcher>(m_PhysicsShader);
        m_Renderer = std::make_unique<Territory::InstancedRenderer>();
    }

    void OnUpdate(float deltaTime) override {
        // --- 第一步：空间哈希构建阶段 (Hashing Pass) ---
        
        // 1.1 极速清空网格头，所有格子设为 -1
        m_GridHeadSSBO->ClearInt(-1);

        // 1.2 绑定三大 Buffer 槽位
        m_ParticleSSBO->Bind(0);
        m_GridHeadSSBO->Bind(1);
        m_ParticleListSSBO->Bind(2);

        // 1.3 调度哈希着色器
        m_SpatialHashShader->Bind();
        m_SpatialHashShader->SetInt("u_ParticleCount", m_ParticleCount);
        m_SpatialHashShader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0)); // 我们暂时只用 2D
        
        uint32_t workGroups = (m_ParticleCount + 255) / 256;
        m_HashDispatcher->DispatchWithBarrier(workGroups, 1, 1);

        // --- 第二步：物理碰撞模拟阶段 (Physics Pass) ---
        
        m_PhysicsShader->Bind();
        m_PhysicsShader->SetFloat("u_DeltaTime", deltaTime);
        m_PhysicsShader->SetInt("u_ParticleCount", m_ParticleCount);
        m_PhysicsShader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));

        // 再次派发物理模拟。注意：由于我们在 DispatchWithBarrier 里加了 Shader Storage Barrier，
        // 这里的物理着色器能保证读到上面哈希着色器刚刚写完的、热乎的链表数据。
        m_PhysicsDispatcher->DispatchWithBarrier(workGroups, 1, 1);
    }

    void OnRender() override {
        // --- 第三步：实例化渲染阶段 (Render Pass) ---
        m_Renderer->Draw(m_RenderShader, m_ParticleCount);
    }

private:
    uint32_t m_ParticleCount;
    int m_GridRes;

    std::shared_ptr<Territory::ShaderStorageBuffer> m_ParticleSSBO;
    std::shared_ptr<Territory::ShaderStorageBuffer> m_GridHeadSSBO;
    std::shared_ptr<Territory::ShaderStorageBuffer> m_ParticleListSSBO;

    std::shared_ptr<Territory::Shader> m_SpatialHashShader;
    std::shared_ptr<Territory::Shader> m_PhysicsShader;
    std::shared_ptr<Territory::Shader> m_RenderShader;

    std::unique_ptr<Territory::ComputeDispatcher> m_HashDispatcher;
    std::unique_ptr<Territory::ComputeDispatcher> m_PhysicsDispatcher;
    std::unique_ptr<Territory::InstancedRenderer> m_Renderer;
};

int main() {
    try {
        Territory::WindowProps props("Territory Engine - 100k Spatial Particles", 1280, 720);
        ColorWarApp app(props);
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Engine Fatal Error: " << e.what() << '\n';
        return -1;
    }
    return 0;
}