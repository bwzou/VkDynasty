#version 450

layout (location = 0) in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;

layout (binding = 2, std140) uniform UniformsMaterial {
    bool u_enableLight;
    bool u_enableIBL;
    bool u_enableShadow;

    float u_pointSize;
    float u_kSpecular;
    vec4 u_baseColor;
};

layout (binding = 3) uniform sampler2D u_albedoMap;

layout (binding = 4) uniform sampler2D u_normalMap;

layout (binding = 5) uniform sampler2D u_emissiveMap;

layout (binding = 6) uniform sampler2D u_metalRoughnessMap;

void main() {
    // FragColor = u_baseColor;
    FragColor= texture(u_albedoMap, v_texCoord);

    
}

