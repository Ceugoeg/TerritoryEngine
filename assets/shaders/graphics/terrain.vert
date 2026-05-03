#version 450 core

// 零输入，纯粹利用内建的 VertexID
out vec2 v_TexCoord;

void main() {
    // 巧妙的位运算，生成一个覆盖 [-1, 1] 屏幕空间的超大三角形
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    
    // 把屏幕坐标映射到 [0, 1] 的 UV 坐标，传给像素着色器
    v_TexCoord = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);
    
    gl_Position = vec4(x, y, 0.0, 1.0);
}