// #version 450

// layout(location = 0) out vec3 fragColor;

// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5, 0.5),
//     vec2(-0.5, 0.5)
// );

// vec3 colors[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

// void main() {
//     gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
//     fragColor = colors[gl_VertexIndex];
// }

// #version 450

// layout(binding = 0) uniform UniformBufferObject {
//     mat4 model;
//     mat4 view;
//     mat4 proj;
// } ubo;

// layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec3 inColor;
// layout (location = 2) in vec3 a_normal;
// layout (location = 3) in vec3 a_tangent;
// // layout(location = 2) in vec2 inTexCoord;

// layout(location = 0) out vec3 fragColor;
// // layout(location = 1) out vec2 fragTexCoord;

// void main() {
//     gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
//     // gl_Position = vec4(inPosition, 1.0);
//     fragColor = inColor;
//     // fragTexCoord = inTexCoord;
// }

#version 450

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texCoord;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec3 a_tangent;

// layout (binding = 0, std140) uniform UniformsModel {
//     bool u_reverseZ;
//     mat4 u_modelMatrix;
//     mat4 u_modelViewProjectionMatrix;
//     mat3 u_inverseTransposeModelMatrix;
//     mat4 u_shadowMVPMatrix;
// };

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (binding = 1) uniform UniformsMaterial {
    bool u_enableLight;
    bool u_enableIBL;
    bool u_enableShadow;

    float u_pointSize;
    float u_kSpecular;
    vec4 u_baseColor;
};

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(a_position, 1.0);
    gl_PointSize = u_pointSize;
}