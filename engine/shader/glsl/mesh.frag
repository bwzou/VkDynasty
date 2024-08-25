#version 450


layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;


float box2(vec2 st) {
    float left = 0.0;
    float right = 0.4;
    float top = 0.6;
    float bottom = 0.2;

    //左下边界
    vec2 bl = step(vec2(left,bottom),st);
    float pct = bl.x * bl.y;

    //右上边界
    vec2 tr = step(vec2(right,top),1.0-st);//检测值要小于右上边界才应该返回1.0，所以使用1.0-st
    pct *= tr.x * tr.y;

    return pct;
}


void main() {
    // float u_resolution = 1.78;
    // float u_resolution_y = 1440.0;

    // vec2 st = (gl_FragCoord.xy * 2.0 - u_resolution) / u_resolution_y;

    // vec3 line_color = vec3(1.0, 1.0, 0.0);
    // vec3 color = vec3(0.6);//背景色
    // float pct = 0.0;

    // pct = box2(st);
    // color = mix(color, line_color, pct);

    outColor = vec4(fragColor, 1.0);
}