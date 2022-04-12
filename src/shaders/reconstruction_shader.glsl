#version 330 core

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform int iFrame;

in vec2 texCoord;
uniform sampler2D tex;

// foveated render vars
uniform int stride;
uniform float thresh1; // smallest foveal region
uniform float thresh2; // middle region
uniform float thresh3; // far region

// constant vars
int quad = stride / 2; // how wide the group of dropped pixels is
const vec4 clear = vec4(0, 0, 0, 1);

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
    float d2 = norm2(coord - 2 * vec2(iMouse.x, -iMouse.y + iResolution.y / 2));

    // which quad am on?
    float xmod = mod(coord.x, stride);
    float ymod = mod(coord.y, stride);

    int x2 = int(xmod / 2);
    int y2 = int(ymod / 2);

    if (xmod < quad && ymod < quad) // top left
    {
        // no op
        discard;
    }
    else if (xmod < quad && ymod >= quad) // top right
    {
        if (d2 > sqr(thresh1))
        {
            fragColor = vec4(0.0);
            fragColor += texelFetch(tex, ivec2(coord.x - (x2 - quad), coord.y), 0);
            fragColor += texelFetch(tex, ivec2(coord.x + (quad - x2), coord.y), 0);
            fragColor += texelFetch(tex, ivec2(coord.x, coord.y - quad), 0);
            fragColor += texelFetch(tex, ivec2(coord.x, coord.y + quad), 0);
            fragColor /= 4;
        }
        else
            discard;
    }
    else if (xmod >= quad && ymod < quad) // bottom left
    {
        if (d2 > sqr(thresh2))
        {
            fragColor = vec4(0.0);
            fragColor += texelFetch(tex, ivec2(coord.x - quad, coord.y), 0);
            fragColor += texelFetch(tex, ivec2(coord.x + quad, coord.y), 0);
            fragColor += texelFetch(tex, ivec2(coord.x, coord.y - quad), 0);
            fragColor += texelFetch(tex, ivec2(coord.x, coord.y + quad), 0);
            fragColor /= 4;
        }
        else
            discard;
    }
    else // bottom right
    {
        if (d2 > sqr(thresh3))
        {
            fragColor = vec4(0.0);
            fragColor += texelFetch(tex, ivec2(coord.x - (x2 - quad), coord.y), 0);
            fragColor += texelFetch(tex, ivec2(coord.x + (quad - x2), coord.y), 0);
            fragColor /= 2;
        }
        else
            discard;
    }
}