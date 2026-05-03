#version 450 core

in vec2 v_TexCoord;
out vec4 FragColor;

// 槽位 3：领地画布
layout(std430, binding = 3) buffer TerrainGridBuffer {
    int terrainGrid[];
};

uniform ivec2 u_Resolution; 

void main() {
    // 1. 映射 UV 到像素网格索引
    ivec2 pixelCoord = ivec2(v_TexCoord * vec2(u_Resolution));
    pixelCoord = clamp(pixelCoord, ivec2(0), u_Resolution - ivec2(1));
    int index = pixelCoord.y * u_Resolution.x + pixelCoord.x;

    // 2. 获取领地归属权
    int teamID = terrainGrid[index];

    // 3. 四阵营上色逻辑
    if (teamID == 0) {
        FragColor = vec4(0.8, 0.2, 0.2, 1.0); // 0: 红
    } else if (teamID == 1) {
        FragColor = vec4(0.2, 0.8, 0.2, 1.0); // 1: 绿
    } else if (teamID == 2) {
        FragColor = vec4(0.2, 0.2, 0.8, 1.0); // 2: 蓝
    } else if (teamID == 3) {
        FragColor = vec4(0.8, 0.8, 0.2, 1.0); // 3: 黄
    } else {
        FragColor = vec4(0.15, 0.15, 0.15, 1.0); // 中立地带
    }
}