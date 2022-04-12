#version 330 core

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform int iFrame;

in vec2 texCoord;
uniform sampler2D tex;

// constant vars
const int stride = 16;
const int quad = stride / 2;
const vec4 clear = vec4(0, 0, 0, 1);
float diag = 0.5 * (iResolution.x + iResolution.y);
float thresh1 = 0.1 * diag;  // smallest foveal region
float thresh2 = 0.25 * diag; // middle region
float thresh3 = 0.4 * diag;  // far region

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
    vec2 coord = gl_FragCoord.xy;
    float d2 = norm2(coord - 2 * vec2(iMouse.x, -iMouse.y + iResolution.y / 2));

    // which quad am on?
    float xmod = mod(coord.x - 0.5, stride);
    float ymod = mod(coord.y - 0.5, stride);

    if (xmod < quad && ymod < quad) // top left
    {
        // no op
        discard;
    }
    else if (xmod < quad && ymod >= quad) // top right
    {
        if (d2 > sqr(thresh1))
            fragColor = texelFetch(tex, ivec2(500, 500), 0);
        else
            discard;
    }
    else if (xmod >= quad && ymod < quad) // bottom left
    {
        if (d2 > sqr(thresh2))
            fragColor = texelFetch(tex, ivec2(500, 500), 0);
        else
            discard;
    }
    else // bottom right
    {
        if (d2 > sqr(thresh3))
            fragColor = texelFetch(tex, ivec2(500, 500), 0);
        else
            discard;
    }
}