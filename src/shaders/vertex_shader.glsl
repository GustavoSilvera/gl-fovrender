#version 330 core
// a very simple vertex shader passing primitives to the fragment shader

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;

out vec2 texCoord;
void main()
{
    texCoord = inTexCoord;
    gl_Position = vec4(position.xyz, 1.0);
}