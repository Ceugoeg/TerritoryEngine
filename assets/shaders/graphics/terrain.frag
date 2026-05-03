#version 450 core

in vec2 v_TexCoord;
out vec4 FragColor;

// 槽位 3：我们的领地画布 (1280 x 720 个 int)
layout(std430, binding = 3) buffer TerrainGridBuffer {
    int terrainGrid[];
};

uniform ivec2 u_Resolution; // 屏幕分辨率 (1280, 720)

void main() {
    // 1. 根据当前像素的 UV 坐标，算出它属于网格的哪个索引
    ivec2 pixelCoord = ivec2(v_TexCoord * vec2(u_Resolution));
    
    // 确保不越界
    pixelCoord = clamp(pixelCoord, ivec2(0), u_Resolution - ivec2(1));
    
    // 转化为 1D 数组索引
    int index = pixelCoord.y * u_Resolution.x + pixelCoord.x;
    
    // 2. 查表获取领地归属权
    int teamID = terrainGrid[index];
    
    // 3. 根据 teamID 上色
    if (teamID == 0) {
        FragColor = vec4(0.8, 0.2, 0.2, 1.0); // 红色阵营领地
    } else if (teamID == 1) {
        FragColor = vec4(0.2, 0.8, 0.2, 1.0); // 绿色阵营领地
    } else if (teamID == 2) {
        FragColor = vec4(0.2, 0.2, 0.8, 1.0); // 蓝色阵营领地
    } else {
        FragColor = vec4(0.15, 0.15, 0.15, 1.0); // -1 是深灰色中立地带
    }
}