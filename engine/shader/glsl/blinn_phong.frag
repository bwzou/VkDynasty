#version 450

layout (location = 0) in vec2 v_texCoord;
layout (location = 1) in vec3 v_normalVector;
layout (location = 2) in vec3 v_worldPos;
layout (location = 3) in vec3 v_cameraDirection;
layout (location = 4) in vec3 v_lightDirection;

layout (location = 0) out vec4 FragColor;

layout (binding = 1, std140) uniform UniformsScene {
    vec3 u_ambientColor;
    vec3 u_cameraPosition;
    vec3 u_pointLightPosition;
    vec3 u_pointLightColor;
};

layout (binding = 2, std140) uniform UniformsMaterial {
    bool u_enableLight;
    bool u_enableIBL;
    bool u_enableShadow;

    float u_pointSize;
    float u_kSpecular;
    vec4 u_baseColor;
};

// layout (binding = 2) uniform sampler2D u_albedoMap;

// layout (binding = 3) uniform sampler2D u_normalMap;

// layout (binding = 4) uniform sampler2D u_emissiveMap;

// layout (binding = 5) uniform sampler2D u_metalRoughnessMap;

void main() {
    // FragColor = u_baseColor;
    vec4 baseColor = u_baseColor;

    // L = La + Ld + Ls
    //   = ka * Ia + kd * (I / r^2) * max(0, N·L) + ks * (I / r^2) * max(0, N·H)
    // TODO 不考虑衰减
    
    // 计算环境光
    vec3 ambientColor = baseColor.rgb * u_ambientColor;

    // 计算漫反射系数 kd
    vec3 lightDirection = normalize(v_lightDirection);
    vec3 normalVector = normalize(v_normalVector);
    float lambertian = clamp(dot(lightDirection, normalVector), 0.0, 1.0);
    vec3 diffuseColor = baseColor.rgb * u_pointLightColor * lambertian;

    // 计算镜面反射系数 ks
    vec3 halfVector = normalize(v_lightDirection + v_cameraDirection);
    int specularExponent = 5;
    float specular = clamp(dot(halfVector, v_normalVector), 0.0, 1.0);
    vec3 specularColor = u_kSpecular * vec3(pow(specular, specularExponent));

    // 计算每一个片元的输出颜色
    FragColor = vec4(ambientColor + diffuseColor + specularColor, baseColor.a);
}

