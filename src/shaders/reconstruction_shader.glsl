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

    // which quad am on?
    float xmod = mod(coord.x, stride);
    float ymod = mod(coord.y, stride);

    // compute (boxy) distance to foveal region
    vec2 boxy_coord = floor(coord / stride) * stride;
    vec2 center = floor(vec2(Mouse.x, -Mouse.y + iResolution.y) / stride) * stride;
    float d2 = norm2(boxy_coord - center);

    // assume equal weights, though these change depending on interpolation
    float weight_x = 0.5;
    float weight_y = 0.5;

    if (xmod < quad && ymod < quad) // top left
    {
        // no op (always rendered in full in frag shader)
        discard;
    }
    else if (xmod < quad && ymod >= quad && d2 < sqr(thresh3)) // top right
    {
        if (d2 > sqr(thresh1))
        {
            weight_x = xmod / quad;          // positive is right
            weight_y = (ymod - quad) / quad; // positive is up
            fragColor = vec4(0.0);
            // always accumulate vertical pixels for interp
            fragColor += weight_y * texelFetch(tex, ivec2(coord.x, coord.y + stride - ymod), 0);           // top
            fragColor += (1.0 - weight_y) * texelFetch(tex, ivec2(coord.x, coord.y - ymod + quad - 1), 0); // bottom
            // usually accumulate horizontal pixels for interp, but not if left is unfilled
            vec4 left_colour = texelFetch(tex, ivec2(coord.x - xmod - 1, coord.y), 0);
            if (left_colour.rgb != vec3(0)) // needs to be coloured somewhat
            {
                // as long as left is good, use it for more data
                fragColor += weight_x * texelFetch(tex, ivec2(coord.x + quad - xmod, coord.y), 0); // right
                fragColor += (1.0 - weight_x) * left_colour;                                       // left
                fragColor /= 2; // average between x data and y data
            }
        }
        else
            discard;
    }
    else if (xmod >= quad && ymod < quad && d2 < sqr(thresh3)) // bottom left
    {
        if (d2 > sqr(thresh2))
        {
            weight_x = (xmod - quad) / quad; // positive is right
            weight_y = ymod / quad;          // positive is up
            fragColor = vec4(0.0);
            // always accumulate left/right data for interp
            fragColor += (1.0 - weight_x) * texelFetch(tex, ivec2(coord.x - xmod + quad - 1, coord.y), 0); // left
            fragColor += weight_x * texelFetch(tex, ivec2(coord.x + stride - xmod, coord.y), 0);           // right
            // usually accumulate vertical pixels for interp, but not if bottom is unfilled
            vec4 bottom_colour = texelFetch(tex, ivec2(coord.x, coord.y - ymod - 1), 0);
            if (bottom_colour.rgb != vec3(0)) // needs to be coloured somewhat
            {
                fragColor += weight_y * texelFetch(tex, ivec2(coord.x, coord.y + quad - ymod), 0); // top
                fragColor += (1.0 - weight_y) * bottom_colour;                                     // bottom
                fragColor /= 2; // average between x data and y data
            }
        }
        else
            discard;
    }
    else // bottom right
    {
        if (d2 > sqr(thresh3))
        {
            fragColor = vec4(0.0);
            // need to case for horizontal, vertical, or diagonal bilinear interp.
            if (xmod < quad)
            {
                // case 1: vertical bilinear interpolation
                weight_y = (ymod - quad) / quad; // positive is up

                fragColor += weight_y * texelFetch(tex, ivec2(coord.x, coord.y + stride - ymod), 0);           // top
                fragColor += (1.0 - weight_y) * texelFetch(tex, ivec2(coord.x, coord.y - ymod + quad - 1), 0); // bottom
            }
            else
            {
                weight_x = (xmod - quad) / quad; // positive is right
                if (ymod < quad)
                {
                    // case 2: horizontal bilinear interpolation
                    fragColor += weight_x * texelFetch(tex, ivec2(coord.x + stride - xmod, coord.y), 0);           // R
                    fragColor += (1.0 - weight_x) * texelFetch(tex, ivec2(coord.x - xmod + quad - 1, coord.y), 0); // L
                }
                else
                {
                    // case 3: diagonal trilinear interpolation
                    weight_y = (ymod - quad) / quad;                        // positive is up
                    float weight_xy1 = (1.0 - weight_y) * weight_x;         // positive is bottom right
                    float weight_xy2 = weight_y * (1.0 - weight_x);         // positive is top left
                    float weight_xy3 = weight_y * weight_x;                 // positive is top right
                    float weight_xy4 = (1.0 - weight_y) * (1.0 - weight_x); // positive is bottom left

                    fragColor += // bottom right
                        weight_xy1 * texelFetch(tex, ivec2(coord.x + stride - xmod, coord.y - ymod + quad - 1), 0);
                    fragColor += // top left
                        weight_xy2 * texelFetch(tex, ivec2(coord.x - xmod + quad - 1, coord.y + stride - ymod), 0);
                    fragColor += // top right
                        weight_xy3 * texelFetch(tex, ivec2(coord.x + stride - xmod, coord.y + stride - ymod), 0);
                    fragColor += // bottom left
                        weight_xy4 * texelFetch(tex, ivec2(coord.x - xmod + quad - 1, coord.y - ymod + quad - 1), 0);
                }
            }
        }
        else
            discard;
    }
}