#version 450 core
struct Peg { vec2 pos; float rad; float pad; };
layout(std430, binding = 1) buffer PegBuffer { Peg pegs[]; };
out vec4 v_Color;
uniform ivec2 u_Resolution;
void main() {
    Peg p = pegs[gl_VertexID];
    gl_Position = vec4(p.pos, 0.0, 1.0);
    gl_PointSize = p.rad * float(u_Resolution.y);
    v_Color = vec4(0.8, 0.8, 0.8, 1.0); // 银白色的金属钉子
}