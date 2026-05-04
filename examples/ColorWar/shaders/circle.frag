#version 450 core
in vec4 v_Color;
out vec4 FragColor;
void main() {
    // gl_PointCoord 是显卡内建的 0.0~1.0 正方形坐标
    vec2 coord = gl_PointCoord - vec2(0.5);
    if(length(coord) > 0.5) discard; // 削掉四个角，变成完美圆形
    FragColor = v_Color;
}