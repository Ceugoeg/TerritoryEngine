#version 450 core
struct Ball { vec2 pos; vec2 vel; vec4 col; float rad; int isActive; vec2 pad; };
layout(std430, binding = 0) buffer BallBuffer { Ball balls[]; };
out vec4 v_Color;
uniform ivec2 u_Resolution;
void main() {
    Ball b = balls[gl_VertexID];
    if(b.isActive == 0) { 
        gl_Position = vec4(999.0); return; // 死球不可见
    }
    gl_Position = vec4(b.pos, 0.0, 1.0);
    // 把物理半径转换成屏幕像素大小
    gl_PointSize = b.rad * float(u_Resolution.y); 
    v_Color = b.col;
}