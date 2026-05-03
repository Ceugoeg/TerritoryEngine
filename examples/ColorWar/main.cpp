#include <Territory/Core/Application.hpp>
#include <iostream>

// 上层的沙盒游戏类，继承自引擎的核心 Application
class ColorWarApp : public Territory::Application {
public:
    ColorWarApp(const Territory::WindowProps& props) 
        : Territory::Application(props) {}

    ~ColorWarApp() override = default;

protected:
    void OnInit() override {
        std::cout << "[ColorWar] Game Initialized. Ready to spawn particles!\n";
        // 未来在这里初始化 GPU Buffer 和 Compute Shader
    }

    void OnUpdate(float deltaTime) override {
        // 这里会以极高的频率执行
        // std::cout << "Delta Time: " << deltaTime << "s\n";
    }

    void OnRender() override {
        // GPU 渲染指令分发
    }

    void OnShutdown() override {
        std::cout << "[ColorWar] Game Shutting Down. Cleaning up VRAM...\n";
    }
};

// 整个程序的唯一入口
int main() {
    try {
        // 设置窗口属性
        Territory::WindowProps props("Territory Engine - Tensor Core CA Simulator", 1280, 720);
        
        // 实例化游戏并启动主循环
        ColorWarApp app(props);
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Engine Fatal Error: " << e.what() << '\n';
        return -1;
    }

    return 0;
}