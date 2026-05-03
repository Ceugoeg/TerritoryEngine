#include "Territory/Core/Application.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

namespace Territory {

Application::Application(const WindowProps& props) {
    InitWindow(props);
}

Application::~Application() {
    ShutdownWindow();
}

void Application::InitWindow(const WindowProps& props) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_WindowHandle = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), nullptr, nullptr);
    if (!m_WindowHandle) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window!");
    }

    glfwMakeContextCurrent(m_WindowHandle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD!");
    }

    std::cout << "--- Territory Engine GPU Context ---" << std::endl;
    std::cout << "Vendor:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version:  " << glGetString(GL_VERSION) << std::endl;
    std::cout << "------------------------------------" << std::endl;

    m_IsRunning = true;
}

void Application::ShutdownWindow() {
    if (m_WindowHandle) {
        glfwDestroyWindow(m_WindowHandle);
        m_WindowHandle = nullptr;
    }
    glfwTerminate();
}

void Application::Run() {
    OnInit();

    // 极速 Main Loop
    while (m_IsRunning && !glfwWindowShouldClose(m_WindowHandle)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - m_LastFrameTime;
        m_LastFrameTime = currentFrameTime;

        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        OnUpdate(deltaTime);

        OnRender();

        glfwSwapBuffers(m_WindowHandle);
    }

    OnShutdown();
}

} // namespace Territory