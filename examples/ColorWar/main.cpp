#include <Territory/Core/Application.hpp>
#include <Territory/Renderer/Shader.hpp>
#include <Territory/Memory/ShaderStorageBuffer.hpp>
#include <Territory/Pipeline/ComputeDispatcher.hpp>
#include <Territory/Renderer/InstancedRenderer.hpp>

#include <iostream>
#include <vector>
#include <random>
#include <glad/glad.h>

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
          m_GridRes(128) {}

    ~ColorWarApp() {
        if (m_DummyVAO != 0) glDeleteVertexArrays(1, &m_DummyVAO);
    }

protected:
    void OnInit() override {
        std::cout << "[ColorWar] Initializing Decoupled Simulation...\n";

        std::vector<Particle> initialParticles(m_ParticleCount);
        for (int i = 0; i < m_ParticleCount; ++i) {
            initialParticles[i].position = { 999.0f, 999.0f };
            initialParticles[i].velocity = { 0.0f, 0.0f };
            initialParticles[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        }

        m_ParticleSSBO = std::make_shared<Territory::ShaderStorageBuffer>(initialParticles.data(), m_ParticleCount * sizeof(Particle));
        m_GridHeadSSBO = std::make_shared<Territory::ShaderStorageBuffer>(m_GridRes * m_GridRes * sizeof(int));
        m_ParticleListSSBO = std::make_shared<Territory::ShaderStorageBuffer>(m_ParticleCount * sizeof(int));
        
        std::vector<int> initialTerrain(1280 * 720, -1);
        m_TerrainSSBO = std::make_shared<Territory::ShaderStorageBuffer>(initialTerrain.data(), initialTerrain.size() * sizeof(int));

        // =========================================================
        // 【路径重构】：区分引擎级基础 Shader 与 实例级业务 Shader
        // =========================================================
        
        // 1. 引擎级算法：空间哈希
        m_SpatialHashShader = std::make_shared<Territory::Shader>("assets/shaders/compute/spatial_hash.comp");
        
        // 2. 实例级业务：旋转炮台与涂地逻辑
        m_PhysicsShader = std::make_shared<Territory::Shader>("examples/ColorWar/shaders/particles.comp");
        
        // 3. 引擎级算法：通用实例化渲染器
        m_RenderShader = std::make_shared<Territory::Shader>(
            "assets/shaders/graphics/instanced.vert", 
            "assets/shaders/graphics/instanced.frag"
        );
        
        // 4. 混合：引擎级的全屏三角形顶点生成器 + 实例级的四色阵营片元着色器
        m_TerrainShader = std::make_shared<Territory::Shader>(
            "assets/shaders/graphics/terrain.vert", 
            "examples/ColorWar/shaders/terrain.frag"
        );

        m_HashDispatcher = std::make_unique<Territory::ComputeDispatcher>(m_SpatialHashShader);
        m_PhysicsDispatcher = std::make_unique<Territory::ComputeDispatcher>(m_PhysicsShader);
        m_Renderer = std::make_unique<Territory::InstancedRenderer>();

        glCreateVertexArrays(1, &m_DummyVAO);
    }

    void OnUpdate(float deltaTime) override {
        m_GridHeadSSBO->ClearInt(-1);
        m_ParticleSSBO->Bind(0);
        m_GridHeadSSBO->Bind(1);
        m_ParticleListSSBO->Bind(2);

        m_SpatialHashShader->Bind();
        m_SpatialHashShader->SetInt("u_ParticleCount", m_ParticleCount);
        m_SpatialHashShader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        
        uint32_t workGroups = (m_ParticleCount + 255) / 256;
        m_HashDispatcher->DispatchWithBarrier(workGroups, 1, 1);

        static float s_TotalTime = 0.0f;
        s_TotalTime += deltaTime;

        m_PhysicsShader->Bind();
        m_PhysicsShader->SetFloat("u_DeltaTime", deltaTime);
        m_PhysicsShader->SetFloat("u_Time", s_TotalTime);
        m_PhysicsShader->SetInt("u_ParticleCount", m_ParticleCount);
        m_PhysicsShader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        
        m_TerrainSSBO->Bind(3); 
        m_PhysicsShader->SetInt2("u_Resolution", glm::ivec2(1280, 720)); 

        m_PhysicsDispatcher->DispatchWithBarrier(workGroups, 1, 1);
    }

    void OnRender() override {
        m_TerrainShader->Bind();
        m_TerrainShader->SetInt2("u_Resolution", glm::ivec2(1280, 720));
        m_TerrainSSBO->Bind(3);
        glBindVertexArray(m_DummyVAO); 
        glDrawArrays(GL_TRIANGLES, 0, 3);

        m_Renderer->Draw(m_RenderShader, m_ParticleCount);
    }

private:
    uint32_t m_ParticleCount;
    int m_GridRes;
    uint32_t m_DummyVAO = 0;

    std::shared_ptr<Territory::ShaderStorageBuffer> m_ParticleSSBO;
    std::shared_ptr<Territory::ShaderStorageBuffer> m_GridHeadSSBO;
    std::shared_ptr<Territory::ShaderStorageBuffer> m_ParticleListSSBO;
    std::shared_ptr<Territory::ShaderStorageBuffer> m_TerrainSSBO;

    std::shared_ptr<Territory::Shader> m_SpatialHashShader;
    std::shared_ptr<Territory::Shader> m_PhysicsShader;
    std::shared_ptr<Territory::Shader> m_RenderShader;
    std::shared_ptr<Territory::Shader> m_TerrainShader;

    std::unique_ptr<Territory::ComputeDispatcher> m_HashDispatcher;
    std::unique_ptr<Territory::ComputeDispatcher> m_PhysicsDispatcher;
    std::unique_ptr<Territory::InstancedRenderer> m_Renderer;
};

int main() {
    try {
        Territory::WindowProps props("Territory Engine - Decoupled App", 1280, 720);
        ColorWarApp app(props);
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Engine Error: " << e.what() << '\n';
        return -1;
    }
    return 0;
}