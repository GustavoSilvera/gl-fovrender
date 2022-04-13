#version 330 core

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform vec2 Mouse;
uniform int iFrame;

// foveated render vars
uniform int stride;
uniform float thresh1; // smallest foveal region
uniform float thresh2; // middle region
uniform float thresh3; // far region

// constant vars
int quad = stride / 2; // how wide the group of dropped pixels is
const vec4 clear = vec4(0, 0, 0, 1);

vec4 expensive_main(); // declaration, definition in fragment shader

float norm2(const vec2 a)
{
    return dot(a, a);
}

float sqr(const float a)
{
    return a * a;
}

void main()
{
    vec2 coord = gl_FragCoord.xy - 0.5; // top left corner of pixel
    float d2 = norm2(coord - vec2(Mouse.x, -Mouse.y + iResolution.y));

    // which quad am on?
    float xmod = mod(coord.x, stride);
    float ymod = mod(coord.y, stride);

    if (xmod < quad && ymod < quad) // top left
    {
        fragColor = expensive_main();
    }
    else if (xmod < quad && ymod >= quad) // top right
    {
        fragColor = (d2 > sqr(thresh1)) ? clear : expensive_main();
    }
    else if (xmod >= quad && ymod < quad) // bottom left
    {
        fragColor = (d2 > sqr(thresh2)) ? clear : expensive_main();
    }
    else // bottom right
    {
        fragColor = (d2 > sqr(thresh3)) ? clear : expensive_main();
    }
}