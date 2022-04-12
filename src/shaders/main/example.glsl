#version 330 core

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform int iFrame;

vec4 expensive_main()
{
    fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    return fragColor;
}