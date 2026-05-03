#version 450 core

layout(location = 0) in vec2 aPos; // 基础的正方形四个顶点

struct Particle {
    vec2 position;
    vec2 velocity;
    vec4 color;
};

// 同样的 SSBO，渲染管线也能只读访问
layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

out vec4 v_Color;

void main() {
    // gl_InstanceID 是实例化渲染的魔法变量，表示当前正在画第几个粒子
    Particle p = particles[gl_InstanceID];
    
    // 设置粒子的大小为 0.005
    float particleSize = 0.005;
    
    // 最终屏幕坐标 = 基础几何体坐标 * 缩放 + 粒子中心位置
    vec2 finalPos = (aPos * particleSize) + p.position;
    
    gl_Position = vec4(finalPos, 0.0, 1.0);
    
    // 把颜色传给像素着色器
    v_Color = p.color;
}