#version 450

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput inSceneColor;

layout(input_attachment_index = 1, set = 0, binding = 1) uniform highp subpassInput inUiColor;

layout(location = 0) out highp vec4 outColor;

void main() 
{
    highp vec4 sceneColor = subpassLoad(inSceneColor).rgba;

    highp vec4 uiColor = subpassLoad(inUiColor).rgba;

    // Gamma correct
    // TODO: select the VK_FORMAT_B8G8R8A8_SRGB surface format,
    // thre is no need to do gamma correction in the fragment shader
    if (uiColor.r < 1e-6 && uiColor.g < 1e-6 && uiColor.a < 1e-6) 
    {
        uiColor = vec4(pow(uiColor.r, 1.0/2.2), pow(uiColor.g, 1.0/2.2), pow(uiColor.b, 1.0/2.2), pow(uiColor.a, 1.0/2.2));
        outColor = sceneColor;
    }
    else
    {
        uiColor = vec4(pow(uiColor.r, 1.0/2.2), pow(uiColor.g, 1.0/2.2), pow(uiColor.b, 1.0/2.2), pow(uiColor.a, 1.0/2.2));
        outColor = uiColor;
    }

}