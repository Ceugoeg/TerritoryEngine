#include <Territory/Core/Application.hpp>
#include <Territory/Core/Config.hpp>
#include <Territory/Renderer/Shader.hpp>
#include <Territory/Memory/BufferManager.hpp>
#include <Territory/Pipeline/ComputeDispatcher.hpp>
#include <Territory/Pipeline/Pipeline.hpp>
#include <Territory/Renderer/InstancedRenderer.hpp>

#include <iostream>
#include <vector>
#include <random>
#include <glad/glad.h>

struct Particle { glm::vec2 pos; glm::vec2 vel; glm::vec4 col; };

struct Ball {
    glm::vec2 pos; glm::vec2 vel; glm::vec4 col;
    float rad; int isActive; glm::vec2 padding; 
};

struct Peg {
    glm::vec2 pos; float rad; float padding;  
};

// =========================================================
// A 轨：领地系统 Passes
// =========================================================
class SpatialHashPass : public Territory::Pass {
public:
    SpatialHashPass(uint32_t count, int gridRes) : Territory::Pass("SpatialHashPass"), m_Count(count), m_GridRes(gridRes) {
        m_Shader = std::make_shared<Territory::Shader>("assets/shaders/compute/spatial_hash.comp");
        m_Dispatcher = std::make_unique<Territory::ComputeDispatcher>(m_Shader);
    }
    void Execute(float deltaTime) override {
        auto& bm = Territory::BufferManager::Get();
        bm.GetBuffer("GridHead")->ClearInt(-1);
        bm.Bind("Particles", 0); bm.Bind("GridHead", 1); bm.Bind("ParticleList", 2);
        m_Shader->Bind();
        m_Shader->SetInt("u_ParticleCount", m_Count);
        m_Shader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        m_Dispatcher->DispatchWithBarrier((m_Count + 255) / 256, 1, 1);
    }
private:
    uint32_t m_Count; int m_GridRes; std::shared_ptr<Territory::Shader> m_Shader; std::unique_ptr<Territory::ComputeDispatcher> m_Dispatcher;
};

class PhysicsPass : public Territory::Pass {
public:
    PhysicsPass(uint32_t count, int gridRes, int width, int height) 
        : Territory::Pass("PhysicsPass"), m_Count(count), m_GridRes(gridRes), m_HalfWidth(width/2), m_Height(height) {
        m_Shader = std::make_shared<Territory::Shader>("examples/ColorWar/shaders/particles.comp");
        m_Dispatcher = std::make_unique<Territory::ComputeDispatcher>(m_Shader);
    }
    void Execute(float deltaTime) override {
        m_TotalTime += deltaTime;
        auto& bm = Territory::BufferManager::Get();
        m_Shader->Bind();
        m_Shader->SetFloat("u_DeltaTime", deltaTime); m_Shader->SetFloat("u_Time", m_TotalTime);
        m_Shader->SetInt("u_ParticleCount", m_Count); m_Shader->SetVec3("u_GridSize", glm::vec3(m_GridRes, m_GridRes, 0));
        bm.Bind("Terrain", 3); 
        
        // 【关键】：传给领地涂色的是半屏分辨率 640x720
        m_Shader->SetInt2("u_Resolution", glm::ivec2(m_HalfWidth, m_Height)); 
        
        m_Dispatcher->DispatchWithBarrier((m_Count + 255) / 256, 1, 1);
    }
private:
    uint32_t m_Count; int m_GridRes; int m_HalfWidth; int m_Height; float m_TotalTime = 0.0f; 
    std::shared_ptr<Territory::Shader> m_Shader; std::unique_ptr<Territory::ComputeDispatcher> m_Dispatcher;
};

// =========================================================
// B 轨：柏青哥系统 Pass
// =========================================================
class PachinkoPass : public Territory::Pass {
public:
    PachinkoPass(const Territory::SimulationConfig& config) : Territory::Pass("PachinkoPass"), m_Config(config) {
        m_Shader = std::make_shared<Territory::Shader>("assets/shaders/compute/pachinko.comp");
        m_Dispatcher = std::make_unique<Territory::ComputeDispatcher>(m_Shader);
    }
    void Execute(float deltaTime) override {
        auto& bm = Territory::BufferManager::Get();
        bm.Bind("PachinkoBalls", 0);
        bm.Bind("PachinkoPegs", 1);

        m_Shader->Bind();
        m_Shader->SetFloat("u_DeltaTime", deltaTime);
        m_Shader->SetInt("u_BallCount", m_Config.PachinkoBallCount);
        m_Shader->SetInt("u_PegCount", m_Config.PachinkoPegCount);
        m_Shader->SetFloat("u_Gravity", m_Config.Gravity);
        
        m_Dispatcher->DispatchWithBarrier((m_Config.PachinkoBallCount + 255) / 256, 1, 1);
    }
private:
    Territory::SimulationConfig m_Config;
    std::shared_ptr<Territory::Shader> m_Shader;
    std::unique_ptr<Territory::ComputeDispatcher> m_Dispatcher;
};

// =========================================================
// 渲染通道 (真·物理分屏版)
// =========================================================
class RenderPass : public Territory::Pass {
public:
    RenderPass(const Territory::SimulationConfig& config) : Territory::Pass("RenderPass"), m_Config(config) {
        m_TerrainShader = std::make_shared<Territory::Shader>("assets/shaders/graphics/terrain.vert", "examples/ColorWar/shaders/terrain.frag");
        m_ParticleShader = std::make_shared<Territory::Shader>("assets/shaders/graphics/instanced.vert", "assets/shaders/graphics/instanced.frag");
        m_BallShader = std::make_shared<Territory::Shader>("examples/ColorWar/shaders/pachinko_ball.vert", "examples/ColorWar/shaders/circle.frag");
        m_PegShader = std::make_shared<Territory::Shader>("examples/ColorWar/shaders/pachinko_peg.vert", "examples/ColorWar/shaders/circle.frag");
        
        m_Renderer = std::make_unique<Territory::InstancedRenderer>();
        glCreateVertexArrays(1, &m_DummyVAO);
        
        glEnable(GL_PROGRAM_POINT_SIZE);
    }
    ~RenderPass() { glDeleteVertexArrays(1, &m_DummyVAO); }

    void Execute(float deltaTime) override {
        auto& bm = Territory::BufferManager::Get();
        
        int halfWidth = m_Config.WindowWidth / 2;
        int height = m_Config.WindowHeight;

        // ==========================================
        // 📺 左半分屏：A 轨 领地争夺 (640x720)
        // ==========================================
        glViewport(0, 0, halfWidth, height);
        
        // 1. 明确绑定 DummyVAO
        glBindVertexArray(m_DummyVAO); 

        m_TerrainShader->Bind();
        m_TerrainShader->SetInt2("u_Resolution", glm::ivec2(halfWidth, height));
        bm.Bind("Terrain", 3); 
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // 2. 画 A 轨粒子 (此时 VAO 会被 m_Renderer 劫持！)
        bm.Bind("Particles", 0);
        m_Renderer->Draw(m_ParticleShader, m_Config.ParticleCount);

        // ==========================================
        // 📺 右半分屏：B 轨 柏青哥 (640x720)
        // ==========================================
        glViewport(halfWidth, 0, halfWidth, height);

        // 【极其致命的修复】：夺回 VAO！
        // 必须重新绑定，否则拿着别人的 VAO 画自己的点，显卡会直接丢弃画面！
        glBindVertexArray(m_DummyVAO);

        // 3. 画 B 轨静态钉子 (Pegs)
        m_PegShader->Bind();
        m_PegShader->SetInt2("u_Resolution", glm::ivec2(halfWidth, height));
        bm.Bind("PachinkoPegs", 1);
        glDrawArrays(GL_POINTS, 0, m_Config.PachinkoPegCount);

        // 4. 画 B 轨动态小球 (Balls)
        m_BallShader->Bind();
        m_BallShader->SetInt2("u_Resolution", glm::ivec2(halfWidth, height));
        bm.Bind("PachinkoBalls", 0);
        glDrawArrays(GL_POINTS, 0, m_Config.PachinkoBallCount);

        // ==========================================
        // 恢复全局 Viewport
        // ==========================================
        glViewport(0, 0, m_Config.WindowWidth, m_Config.WindowHeight);
    }
private:
    Territory::SimulationConfig m_Config; uint32_t m_DummyVAO;
    std::shared_ptr<Territory::Shader> m_TerrainShader, m_ParticleShader, m_BallShader, m_PegShader;
    std::unique_ptr<Territory::InstancedRenderer> m_Renderer;
};

// =========================================================
// App 核心初始化与组装
// =========================================================
class ColorWarApp : public Territory::Application {
public:
    ColorWarApp(const Territory::WindowProps& props, const Territory::SimulationConfig& config) 
        : Territory::Application(props), m_Config(config) {}
    ~ColorWarApp() { Territory::BufferManager::Get().Clear(); }

protected:
    void OnInit() override {
        std::cout << "[TerritoryEngine] Loading True Split-Screen Twin-Track Simulation...\n";
        auto& bm = Territory::BufferManager::Get();

        // --- A 轨：领地系统数据初始化 (画布变窄，长度减半) ---
        int halfWidth = m_Config.WindowWidth / 2;
        std::vector<Particle> initParticles(m_Config.ParticleCount, { {999.f, 999.f}, {0.f, 0.f}, {1.f, 1.f, 1.f, 1.f} });
        std::vector<int> initTerrain(halfWidth * m_Config.WindowHeight, -1);
        bm.Create("Particles", m_Config.ParticleCount * sizeof(Particle), initParticles.data());
        bm.Create("GridHead", m_Config.GridResolution * m_Config.GridResolution * sizeof(int));
        bm.Create("ParticleList", m_Config.ParticleCount * sizeof(int));
        bm.Create("Terrain", initTerrain.size() * sizeof(int), initTerrain.data());

        // --- B 轨：柏青哥数据初始化 ---
        std::mt19937 rng(42); 
        std::uniform_real_distribution<float> distPos(-0.8f, 0.8f);

        std::vector<Ball> initBalls(m_Config.PachinkoBallCount);
        for(uint32_t i=0; i<m_Config.PachinkoBallCount; ++i) {
            initBalls[i].pos = { distPos(rng), 1.0f + (distPos(rng) * 0.5f) }; 
            initBalls[i].vel = { 0.f, 0.f };
            initBalls[i].rad = 0.025f; // 【放大】：确保看得很清楚
            initBalls[i].isActive = 1;
            initBalls[i].col = { 1.0f, 0.8f, 0.2f, 1.0f }; 
        }

        std::vector<Peg> initPegs;
        int rows = 12;
        int cols = 15;
        float spacingX = 0.12f;
        float spacingY = 0.12f;
        float startX = -0.8f;
        float startY = 0.6f;

        for (int r = 0; r < rows; ++r) {
            int currentCols = cols - (r % 2); 
            float offsetX = (r % 2) * (spacingX * 0.5f); 
            
            for (int c = 0; c < currentCols; ++c) {
                if(initPegs.size() >= m_Config.PachinkoPegCount) break; 
                Peg peg;
                peg.pos = { startX + offsetX + c * spacingX, startY - r * spacingY };
                peg.rad = 0.02f; // 【放大】：之前钉子太小被忽略了，现在翻倍！
                peg.padding = 0.0f;
                initPegs.push_back(peg);
            }
        }
        while(initPegs.size() < m_Config.PachinkoPegCount) {
            initPegs.push_back({ {999.f, 999.f}, 0.02f, 0.f });
        }

        bm.Create("PachinkoBalls", m_Config.PachinkoBallCount * sizeof(Ball), initBalls.data());
        bm.Create("PachinkoPegs", m_Config.PachinkoPegCount * sizeof(Peg), initPegs.data());

        // --- 组装流水线 ---
        m_Pipeline.AddPass(std::make_shared<SpatialHashPass>(m_Config.ParticleCount, m_Config.GridResolution));
        m_Pipeline.AddPass(std::make_shared<PhysicsPass>(m_Config.ParticleCount, m_Config.GridResolution, m_Config.WindowWidth, m_Config.WindowHeight));
        m_Pipeline.AddPass(std::make_shared<PachinkoPass>(m_Config)); 
        
        m_RenderPipeline.AddPass(std::make_shared<RenderPass>(m_Config));
    }

    void OnUpdate(float deltaTime) override { m_Pipeline.ExecuteAll(deltaTime); }
    void OnRender() override { m_RenderPipeline.ExecuteAll(0.0f); }

private:
    Territory::SimulationConfig m_Config; 
    Territory::Pipeline m_Pipeline;
    Territory::Pipeline m_RenderPipeline;
};

int main() {
    try {
        Territory::SimulationConfig config;
        config.ParticleCount = 100000;
        config.GridResolution = 128;
        config.WindowWidth = 1280;
        config.WindowHeight = 720;
        config.PachinkoBallCount = 300; 
        config.PachinkoPegCount = 200; 
        config.Gravity = 0.5f;

        Territory::WindowProps props("Territory Engine - Split Screen", config.WindowWidth, config.WindowHeight);
        ColorWarApp app(props, config);
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Engine Fatal Error: " << e.what() << '\n'; return -1;
    }
    return 0;
}