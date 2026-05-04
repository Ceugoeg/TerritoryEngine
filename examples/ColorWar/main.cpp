#include <Territory/Core/Application.hpp>
#include <Territory/Core/Config.hpp>
#include <Territory/Renderer/Shader.hpp>
#include <Territory/Memory/BufferManager.hpp>
#include <Territory/Pipeline/ComputeDispatcher.hpp>
#include <Territory/Pipeline/Pipeline.hpp>
#include <Territory/Renderer/InstancedRenderer.hpp>

#include <iostream>
#include <vector>
#include <glad/glad.h>

// 严格对齐 GLSL std430 布局
struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
};

// =========================================================
// 业务组件 1：空间哈希通道 (引擎基础设施)
// =========================================================
class SpatialHashPass : public Territory::Pass {
public:
    SpatialHashPass(uint32_t count, int gridRes) 
        : Territory::Pass("SpatialHashPass"), m_Count(count), m_GridRes(gridRes) {
        m_Shader = std::make_shared<Territory::Shader>("assets/shaders/compute/spatial_hash.comp");
        m_Dispatcher = std::make_unique<Territory::ComputeDispatcher>(m_Shader);
    }

    void Execute(float deltaTime) override {
        auto& bm = Territory::BufferManager::Get();
        bm.GetBuffer("GridHead")->ClearInt(-1);
        
        bm.Bind("Particles", 0);
        bm.Bind("GridHead", 1);
        bm.Bind("ParticleList", 2);

        m_Shader->Bind();
        m_Shader->SetInt("u_ParticleCount", m_Count);
        m_Shader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        
        m_Dispatcher->DispatchWithBarrier((m_Count + 255) / 256, 1, 1);
    }
private:
    uint32_t m_Count; int m_GridRes;
    std::shared_ptr<Territory::Shader> m_Shader;
    std::unique_ptr<Territory::ComputeDispatcher> m_Dispatcher;
};

// =========================================================
// 业务组件 2：物理与炮台逻辑通道 (实例特化业务)
// =========================================================
class PhysicsPass : public Territory::Pass {
public:
    PhysicsPass(uint32_t count, int gridRes) 
        : Territory::Pass("PhysicsPass"), m_Count(count), m_GridRes(gridRes) {
        // 加载实例专属的业务 Shader
        m_Shader = std::make_shared<Territory::Shader>("examples/ColorWar/shaders/particles.comp");
        m_Dispatcher = std::make_unique<Territory::ComputeDispatcher>(m_Shader);
    }

    void Execute(float deltaTime) override {
        m_TotalTime += deltaTime;
        auto& bm = Territory::BufferManager::Get();

        m_Shader->Bind();
        m_Shader->SetFloat("u_DeltaTime", deltaTime);
        m_Shader->SetFloat("u_Time", m_TotalTime);
        m_Shader->SetInt("u_ParticleCount", m_Count);
        m_Shader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        
        bm.Bind("Terrain", 3); 
        m_Shader->SetInt2("u_Resolution", glm::ivec2(1280, 720)); 

        m_Dispatcher->DispatchWithBarrier((m_Count + 255) / 256, 1, 1);
    }
private:
    uint32_t m_Count; int m_GridRes; float m_TotalTime = 0.0f;
    std::shared_ptr<Territory::Shader> m_Shader;
    std::unique_ptr<Territory::ComputeDispatcher> m_Dispatcher;
};

// =========================================================
// 业务组件 3：双层渲染通道 (混合设施)
// =========================================================
class RenderPass : public Territory::Pass {
public:
    RenderPass(uint32_t count, int width, int height) 
        : Territory::Pass("RenderPass"), m_Count(count), m_Width(width), m_Height(height) {
        
        // 底层画布：通用顶点 + 实例专属的四色阵营渲染
        m_TerrainShader = std::make_shared<Territory::Shader>(
            "assets/shaders/graphics/terrain.vert", 
            "examples/ColorWar/shaders/terrain.frag"
        );
        
        // 顶层粒子：通用实例化渲染
        m_ParticleShader = std::make_shared<Territory::Shader>(
            "assets/shaders/graphics/instanced.vert", 
            "assets/shaders/graphics/instanced.frag"
        );
        
        m_Renderer = std::make_unique<Territory::InstancedRenderer>();
        
        // 申请 Dummy VAO 欺骗 Core Profile
        glCreateVertexArrays(1, &m_DummyVAO);
    }
    
    ~RenderPass() { 
        glDeleteVertexArrays(1, &m_DummyVAO); 
    }

    void Execute(float deltaTime) override {
        auto& bm = Territory::BufferManager::Get();
        
        // 1. 画底层领地
        m_TerrainShader->Bind();
        m_TerrainShader->SetInt2("u_Resolution", glm::ivec2(m_Width, m_Height));
        bm.Bind("Terrain", 3); 
        glBindVertexArray(m_DummyVAO); 
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // 2. 画顶层粒子
        m_Renderer->Draw(m_ParticleShader, m_Count);
    }
private:
    uint32_t m_Count; int m_Width; int m_Height; uint32_t m_DummyVAO;
    std::shared_ptr<Territory::Shader> m_TerrainShader;
    std::shared_ptr<Territory::Shader> m_ParticleShader;
    std::unique_ptr<Territory::InstancedRenderer> m_Renderer;
};

// =========================================================
// 主体 App：极其干净，完全数据驱动，只负责按配置组装管线！
// =========================================================
class ColorWarApp : public Territory::Application {
public:
    ColorWarApp(const Territory::WindowProps& props, const Territory::SimulationConfig& config) 
        : Territory::Application(props), m_Config(config) {}

    ~ColorWarApp() { 
        // 引擎退出时释放所有显存
        Territory::BufferManager::Get().Clear(); 
    }

protected:
    void OnInit() override {
        std::cout << "[TerritoryEngine] Loading Data-Driven Level...\n";

        // 1. 根据注入的 Config 生成初始数据
        std::vector<Particle> initParticles(m_Config.ParticleCount, { {999.f, 999.f}, {0.f, 0.f}, {1.f, 1.f, 1.f, 1.f} });
        std::vector<int> initTerrain(m_Config.WindowWidth * m_Config.WindowHeight, -1);

        // 2. 向全局显存管家注册缓冲
        auto& bm = Territory::BufferManager::Get();
        bm.Create("Particles", m_Config.ParticleCount * sizeof(Particle), initParticles.data());
        bm.Create("GridHead", m_Config.GridResolution * m_Config.GridResolution * sizeof(int));
        bm.Create("ParticleList", m_Config.ParticleCount * sizeof(int));
        bm.Create("Terrain", initTerrain.size() * sizeof(int), initTerrain.data());

        // 3. 将 Pass 组装进 Pipeline
        m_Pipeline.AddPass(std::make_shared<SpatialHashPass>(m_Config.ParticleCount, m_Config.GridResolution));
        m_Pipeline.AddPass(std::make_shared<PhysicsPass>(m_Config.ParticleCount, m_Config.GridResolution));
        m_RenderPipeline.AddPass(std::make_shared<RenderPass>(m_Config.ParticleCount, m_Config.WindowWidth, m_Config.WindowHeight));
    }

    void OnUpdate(float deltaTime) override {
        // 无脑扣动计算扳机
        m_Pipeline.ExecuteAll(deltaTime);
    }

    void OnRender() override {
        // 无脑扣动渲染扳机
        m_RenderPipeline.ExecuteAll(0.0f);
    }

private:
    Territory::SimulationConfig m_Config; 
    Territory::Pipeline m_Pipeline;
    Territory::Pipeline m_RenderPipeline;
};

// =========================================================
// 程序的真正入口：模拟读取配置
// =========================================================
int main() {
    try {
        // 未来这里可以替换成从 external_level.json 读取
        Territory::SimulationConfig config;
        config.ParticleCount = 100000;
        config.GridResolution = 128;
        config.WindowWidth = 1280;
        config.WindowHeight = 720;
        
        Territory::WindowProps props("Territory Engine - Data Driven Level", config.WindowWidth, config.WindowHeight);
        ColorWarApp app(props, config);
        
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Engine Fatal Error: " << e.what() << '\n'; 
        return -1;
    }
    return 0;
}