// #version 450

// layout(location = 0) in vec3 fragColor;

// layout(location = 0) out vec4 outColor;

// void main() {
//     outColor = vec4(fragColor, 1.0);
// }

// #version 450

// // layout(binding = 1) uniform sampler2D texSampler;

// layout(location = 0) in vec3 fragColor;
// // layout(location = 1) in vec2 fragTexCoord;

// layout(location = 0) out vec4 outColor;

// void main() {
//     // outColor = texture(texSampler, fragTexCoord);

//     outColor = vec4(fragColor, 1.0);
// }

#version 450

layout (location = 0) out vec4 FragColor;

layout (binding = 1, std140) uniform UniformsMaterial {
    bool u_enableLight;
    bool u_enableIBL;
    bool u_enableShadow;

    float u_pointSize;
    float u_kSpecular;
    vec4 u_baseColor;
};

void main() {
    FragColor = u_baseColor;
}

