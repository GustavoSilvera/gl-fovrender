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

/////////////////////////////////////////////////////
//////////////////////BEGIN SHADER///////////////////
/////////////////////////////////////////////////////

vec4 expensive_main()
{
    // implement from a fancy shader's main
    return clear;
}
/////////////////////////////////////////////////////
//////////////////////END SHADER/////////////////////
/////////////////////////////////////////////////////

void main()
{
    vec2 coord = gl_FragCoord.xy;
    float d = length(coord - 2 * vec2(iMouse.x, -iMouse.y + iResolution.y / 2));

    // which quad am on?
    float xmod = mod(coord.x - 0.5, stride);
    float ymod = mod(coord.y - 0.5, stride);

    if (xmod < quad && ymod < quad) // top left
    {
        fragColor = expensive_main();
    }
    else if (xmod < quad && ymod >= quad)
    {
        fragColor = (d > 200) ? clear : expensive_main();
    }
    else if (xmod >= quad && ymod < quad)
    {
        fragColor = (d > 400) ? clear : expensive_main();
    }
    else
    {
        fragColor = (d > 800) ? clear : expensive_main();
    }
}