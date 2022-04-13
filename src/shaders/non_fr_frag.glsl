#version 330 core

// Fast pass-through fragment shader that performs full fidelity shading on every pixel.
// This shader gets enabled when you disable foveated rendering

layout(location = 0) out vec4 fragColor;

vec4 expensive_main(); // declaration, definition in fragment shader

void main()
{
    fragColor = expensive_main();
}