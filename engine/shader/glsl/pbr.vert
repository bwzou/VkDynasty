#version 450

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texCoord;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec3 a_tangent;

layout (location = 0) out vec2 v_texCoord;
layout (location = 1) out vec3 v_normalVector;
layout (location = 2) out vec3 v_worldPos;
layout (location = 3) out vec3 v_cameraDirection;
layout (location = 4) out vec3 v_lightDirection;
layout (location = 5) out vec3 v_normal;
layout (location = 6) out vec3 v_tangent;

layout (binding = 0, std140) uniform UniformsModel {
    bool u_reverseZ;
    mat4 u_modelMatrix;
    mat4 u_modelViewProjectionMatrix;
    mat3 u_inverseTransposeModelMatrix;
    mat4 u_shadowMVPMatrix;
};


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

void main() {
    vec4 position = vec4(a_position, 1.0);
    gl_Position = u_modelViewProjectionMatrix * position;
    v_texCoord = a_texCoord;

    v_worldPos = vec3(u_modelMatrix * position);
    v_normalVector = mat3(u_modelMatrix) * a_normal;
    v_lightDirection = u_pointLightPosition - v_worldPos;
    v_cameraDirection = u_cameraPosition - v_worldPos;

    // 计算世界空间的 v_normal 和 v_tangent
    vec3 N = normalize(u_inverseTransposeModelMatrix * a_normal);
    vec3 T = normalize(u_inverseTransposeModelMatrix * a_tangent);
    v_normal = N;
    // 保证 v_normal 和 v_tangent是垂直的
    v_tangent = normalize(T - dot(T, N) * N);

    gl_PointSize = u_pointSize;
}