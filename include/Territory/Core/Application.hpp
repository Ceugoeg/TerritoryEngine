#pragma once

#include <string>
#include <memory>
#include <cstdint>

// Forward declaration
struct GLFWwindow;

namespace Territory {

// WindowProps struct, to facilitate unified parameter passing
struct WindowProps {
    std::string Title;
    uint32_t Width;
    uint32_t Height;

    WindowProps(std::string title = "Territory Engine - GPU Driven", uint32_t width = 1280, uint32_t height = 720)
        : Title(std::move(title)), Width(width), Height(height) {}
};

class Application {
public:
    Application(const WindowProps& props = WindowProps());
    virtual ~Application();

    // Ensure that the engine instance is globally unique in memory (to prevent double freeing of handles)
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Main Loop
    void Run();

protected:
    // Lifecycle callback functions to be overridden by upper-level game subclasses.
    virtual void OnInit() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {}
    virtual void OnShutdown() {}

private:
    void InitWindow(const WindowProps& props);
    void ShutdownWindow();

private:
    GLFWwindow* m_WindowHandle = nullptr;
    bool m_IsRunning = false;
    float m_LastFrameTime = 0.0f;
};

} // namespace Territory