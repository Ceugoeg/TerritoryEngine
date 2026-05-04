#include <Territory/Core/Application.hpp>
#include <Territory/Renderer/Shader.hpp>
#include <Territory/Memory/ShaderStorageBuffer.hpp>
#include <Territory/Memory/BufferManager.hpp>
#include <Territory/Pipeline/ComputeDispatcher.hpp>
#include <Territory/Renderer/InstancedRenderer.hpp>

#include <iostream>
#include <vector>
#include <random>
#include <glad/glad.h>

// 严格对齐 GLSL std430 布局
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
        if (m_DummyVAO != 0) {
            glDeleteVertexArrays(1, &m_DummyVAO);
        }
        // 清理全局显存管家，释放所有 SSBO 句柄
        Territory::BufferManager::Get().Clear();
    }

protected:
    void OnInit() override {
        std::cout << "[ColorWar] Initializing Decoupled GPU-Driven Pipeline...\n";

        // 1. 生成初始数据
        std::vector<Particle> initialParticles(m_ParticleCount);
        for (int i = 0; i < m_ParticleCount; ++i) {
            initialParticles[i].position = { 999.0f, 999.0f }; // 开局置于墓地
            initialParticles[i].velocity = { 0.0f, 0.0f };
            initialParticles[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        }

        std::vector<int> initialTerrain(1280 * 720, -1); // 全屏中立领地

        // =========================================================
        // 2. 利用 BufferManager 进行统一显存分配
        // =========================================================
        auto& bm = Territory::BufferManager::Get();
        
        bm.Create("Particles", m_ParticleCount * sizeof(Particle), initialParticles.data());
        bm.Create("GridHead", m_GridRes * m_GridRes * sizeof(int));
        bm.Create("ParticleList", m_ParticleCount * sizeof(int));
        bm.Create("Terrain", initialTerrain.size() * sizeof(int), initialTerrain.data());

        // =========================================================
        // 3. 编译着色器（区分引擎级与实例级路径）
        // =========================================================
        
        // 引擎核心算法
        m_SpatialHashShader = std::make_shared<Territory::Shader>("assets/shaders/compute/spatial_hash.comp");
        m_RenderShader = std::make_shared<Territory::Shader>(
            "assets/shaders/graphics/instanced.vert", 
            "assets/shaders/graphics/instanced.frag"
        );
        m_TerrainVertexShader = "assets/shaders/graphics/terrain.vert"; // 引擎通用顶点生成

        // 实例业务逻辑
        m_PhysicsShader = std::make_shared<Territory::Shader>("examples/ColorWar/shaders/particles.comp");
        m_TerrainShader = std::make_shared<Territory::Shader>(
            m_TerrainVertexShader, 
            "examples/ColorWar/shaders/terrain.frag"
        );

        // 4. 初始化调度器与渲染器
        m_HashDispatcher = std::make_unique<Territory::ComputeDispatcher>(m_SpatialHashShader);
        m_PhysicsDispatcher = std::make_unique<Territory::ComputeDispatcher>(m_PhysicsShader);
        m_Renderer = std::make_unique<Territory::InstancedRenderer>();

        // 申请 Dummy VAO 用于全屏三角形绘制
        glCreateVertexArrays(1, &m_DummyVAO);
    }

    void OnUpdate(float deltaTime) override {
        auto& bm = Territory::BufferManager::Get();

        // --- 步骤一：空间哈希构建 (Hashing Pass) ---
        bm.GetBuffer("GridHead")->ClearInt(-1);
        
        bm.Bind("Particles", 0);
        bm.Bind("GridHead", 1);
        bm.Bind("ParticleList", 2);

        m_SpatialHashShader->Bind();
        m_SpatialHashShader->SetInt("u_ParticleCount", m_ParticleCount);
        m_SpatialHashShader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        
        uint32_t workGroups = (m_ParticleCount + 255) / 256;
        m_HashDispatcher->DispatchWithBarrier(workGroups, 1, 1);

        // --- 步骤二：物理模拟与炮台重生 (Physics Pass) ---
        static float s_TotalTime = 0.0f;
        s_TotalTime += deltaTime;

        m_PhysicsShader->Bind();
        m_PhysicsShader->SetFloat("u_DeltaTime", deltaTime);
        m_PhysicsShader->SetFloat("u_Time", s_TotalTime);
        m_PhysicsShader->SetInt("u_ParticleCount", m_ParticleCount);
        m_PhysicsShader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        
        bm.Bind("Terrain", 3); 
        m_PhysicsShader->SetInt2("u_Resolution", glm::ivec2(1280, 720)); 

        m_PhysicsDispatcher->DispatchWithBarrier(workGroups, 1, 1);
    }

    void OnRender() override {
        auto& bm = Territory::BufferManager::Get();

        // 1. 渲染底层领地 (Terrain)
        m_TerrainShader->Bind();
        m_TerrainShader->SetInt2("u_Resolution", glm::ivec2(1280, 720));
        bm.Bind("Terrain", 3); 
        
        glBindVertexArray(m_DummyVAO); 
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 2. 渲染顶层粒子 (Particles)
        m_Renderer->Draw(m_RenderShader, m_ParticleCount);
    }

private:
    uint32_t m_ParticleCount;
    int m_GridRes;
    uint32_t m_DummyVAO = 0;

    std::string m_TerrainVertexShader;

    // 引擎组件
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
        Territory::WindowProps props("Territory Engine - Decoupled Architecture", 1280, 720);
        ColorWarApp app(props);
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Engine Fatal Error: " << e.what() << '\n';
        return -1;
    }
    return 0;
}