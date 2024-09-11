#version 450

void main()
{
    const vec3 fullscreenTrianglePositions[3] = vec3[3](vec3(10.0, 1.0, 0.5), vec3(-1.0, 1.0, 0.5), vec3(-1.0, -10.0, 0.5));
    gl_Position = vec4(fullscreenTrianglePositions[gl_VertexIndex], 1.0);
}