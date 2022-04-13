#version 330 core

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform vec2 Mouse;
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
    float d2 = norm2(coord - vec2(Mouse.x, -Mouse.y + iResolution.y));

    // which quad am on?
    float xmod = mod(coord.x, stride);
    float ymod = mod(coord.y, stride);

    // assume equal if nothing else
    float weight_x = 0.5;
    float weight_y = 0.5;

    if (xmod < quad && ymod < quad) // top left
    {
        // no op (always rendered in full in frag shader)
        discard;
    }
    else if (xmod < quad && ymod >= quad) // top right
    {
        weight_x = xmod / quad;          // positive is right
        weight_y = (ymod - quad) / quad; // positive is up
        if (d2 > sqr(thresh1))
        {
            fragColor = vec4(0.0);
            fragColor += weight_x * texelFetch(tex, ivec2(coord.x + quad - xmod, coord.y), 0);             // right
            fragColor += (1.0 - weight_x) * texelFetch(tex, ivec2(coord.x - xmod - 1, coord.y), 0);        // left
            fragColor += weight_y * texelFetch(tex, ivec2(coord.x, coord.y + stride - ymod), 0);           // top
            fragColor += (1.0 - weight_y) * texelFetch(tex, ivec2(coord.x, coord.y - ymod + quad - 1), 0); // bottom
            fragColor /= 2;
        }
        else
            discard;
    }
    else if (xmod >= quad && ymod < quad) // bottom left
    {
        weight_x = (xmod - quad) / quad; // positive is right
        weight_y = ymod / quad;          // positive is up
        if (d2 > sqr(thresh2))
        {
            fragColor = vec4(0.0);
            fragColor += weight_x * texelFetch(tex, ivec2(coord.x + stride - xmod, coord.y), 0);           // right
            fragColor += (1.0 - weight_x) * texelFetch(tex, ivec2(coord.x - xmod + quad - 1, coord.y), 0); // left
            fragColor += weight_y * texelFetch(tex, ivec2(coord.x, coord.y + quad - ymod), 0);             // top
            fragColor += (1.0 - weight_y) * texelFetch(tex, ivec2(coord.x, coord.y - ymod - 1), 0);        // bottom
            fragColor /= 2;
        }
        else
            discard;
    }
    else // bottom right
    {
        if (d2 > sqr(thresh3))
        {
            fragColor = vec4(0.0);
            // fragColor += texelFetch(tex, ivec2(coord.x - (x2 - quad), coord.y), 0);
            // fragColor += texelFetch(tex, ivec2(coord.x + (quad - x2), coord.y), 0);
            // fragColor /= 2;
        }
        else
            discard;
    }
}