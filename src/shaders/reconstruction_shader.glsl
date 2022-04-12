#version 330 core

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform int iFrame;

// constant vars
const int stride = 16;
const int quad = stride / 2;
const vec4 clear = vec4(0, 0, 0, 1);
float diag2 = 0.5 * (iResolution.x + iResolution.y);
float thresh1_2 = 0.1 * diag2;  // smallest foveal region
float thresh2_2 = 0.25 * diag2; // middle region
float thresh3_2 = 0.4 * diag2;  // far region

float norm2_2(const vec2 a)
{
    return dot(a, a);
}

float sqr_2(const float a)
{
    return a * a;
}

void main()
{
    vec2 coord = gl_FragCoord.xy;
    float d2 = norm2_2(coord - 2 * vec2(iMouse.x, -iMouse.y + iResolution.y / 2));

    // which quad am on?
    float xmod = mod(coord.x - 0.5, stride);
    float ymod = mod(coord.y - 0.5, stride);

    if (xmod < quad && ymod < quad) // top left
    {
        // no op
        discard;
        // fragColor = expensive_main();
    }
    else if (xmod < quad && ymod >= quad) // top right
    {
        if (d2 > sqr_2(thresh1_2))
            fragColor = vec4(1, 0, 1, 1);
        else
            discard;
    }
    else if (xmod >= quad && ymod < quad) // bottom left
    {
        if (d2 > sqr_2(thresh2_2))
            fragColor = vec4(1, 0, 1, 1);
        else
            discard;
    }
    else // bottom right
    {
        if (d2 > sqr_2(thresh3_2))
            fragColor = vec4(1, 0, 1, 1);
        else
            discard;
    }
}