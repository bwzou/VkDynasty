#version 450

#define PI 3.141592657

layout (location = 0) in vec2 v_texCoord;
layout (location = 1) in vec3 v_normalVector;
layout (location = 2) in vec3 v_worldPos;
layout (location = 3) in vec3 v_cameraDirection;
layout (location = 4) in vec3 v_lightDirection;
layout (location = 5) in vec3 v_normal;
layout (location = 6) in vec3 v_tangent;

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

layout (binding = 3) uniform sampler2D u_albedoMap;

layout (binding = 4) uniform sampler2D u_normalMap;

layout (binding = 5) uniform sampler2D u_emissiveMap;

layout (binding = 6) uniform sampler2D u_metalRoughnessMap;


vec3 GetNormalFromMap() {
    vec3 N = normalize(v_normal);
    vec3 T = normalize(v_tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);
    // 计算变换到世界空间的 TBN 矩阵
    // 在切线空间中，三个基向量分别叫做T、B、N
    mat3 TBN = mat3(T, B, N);

    // 获取切线空间法线贴图中得到的法线
    vec3 tangentNormal = texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

// 法线分布函数 从统计学上近似的表示了与某些（如中间）向量h取向一致的微平面的比率。
// NDF = a * a / (PI * pow ((N · H) * (N · H) * （ a * a - 1) + 1), 2)
float DistributionGGX(vec3 N, vec3 H, float a) {
    float a2 = a * a;
    float NdotH = max (dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// 菲涅尔方程 定义的是在不同观察方向上，表面上被反射的光除以被折射的光的比例。
// cosTheta是表面法向量 H 与观察方向 v 的点乘的结果
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// 几何函数 模拟微平面相互遮挡导致光线的能量减少或丢失的现象。
float GeometrySmithGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1 - k) + k;
    return nom / denom;
}

// L 表示光线向量
// V 表示视线向量
// N 是表面法线
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySmithGGX(NdotV, roughness); // 视线方向的几何遮挡
    float ggx2 = GeometrySmithGGX(NdotL, roughness); // 光线方向的几何遮挡
    
    return ggx1 * ggx2;
}

void main() {
    vec4 baseColor = u_baseColor;
    baseColor = texture(u_albedoMap, v_texCoord);
    
    // gamma correct inverse
    vec3 albedo = pow(baseColor.rgb, vec3(2.2f));

    // vec4 normal = texture(u_normalMap, v_texCoord);
    // vec3 N = normalize(v_normalVector);
    
    float metallic = 0.0;
    float roughness = 1.0;
    vec4 metalRoughness = texture(u_metalRoughnessMap, v_texCoord);
    metallic = metalRoughness.b;
    roughness = metalRoughness.g;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    float ao = 1.f;

    // vec3 N = normalize(v_normalVector);
    vec3 N = GetNormalFromMap();
    vec3 V = normalize(v_cameraDirection);
    vec3 L = normalize(v_lightDirection);
    vec3 H = normalize(L + V);

    float distance = length(u_pointLightPosition - v_worldPos);
    // float attenuation = 1.0 / (distance * distance);
    // float attenuation = clamp(1.0f - dot(L, L), 0.0f, 1.0f);
    float attenuation = 1.f;
    vec3 radiance = u_pointLightColor * attenuation;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    // denominator项里的0.001是为了防止除0情况而特意加上的。
    vec3 specular = numerator / max(denominator, 0.00001);

    vec3 kS = F;
    vec3 kD = vec3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = u_ambientColor * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonmapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    FragColor= vec4(color, 1.0);

    // FragColor= texture(u_albedoMap, v_texCoord);
}

