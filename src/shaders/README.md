# Custom fragment shaders
## From ShaderToy

I am including this README so I don't directly distribute others' works, but I will point out some necessary tweaks to get this playground repo to work with them:

# 0) Prerequisites

Please note this is not an extremely robust solution, many things are not fully functional. Notably including iChannels (often used for audio/textures). But for relatively simple shaders this works well enough.

# 1) Copy source from shadertoy 

Here are some great shaders that I have successfully managed to port to this codebase:
| Title | Creator | Link |
| --- | --- | --- |
| Geomechanical | [Bers](https://www.shadertoy.com/user/Bers) | https://www.shadertoy.com/view/MdcXzn |
| simple 2d animation | [naxius](https://www.shadertoy.com/user/naxius) | https://www.shadertoy.com/view/WdsGRl |
| fractal pyramid | [bradjamesgrant](https://www.shadertoy.com/user/bradjamesgrant) | https://www.shadertoy.com/view/tsXBzS |
| Mandelbulb | [EvilRyu](https://www.shadertoy.com/user/EvilRyu) | https://www.shadertoy.com/view/MdXSWn |
| SeaScape | [TDM](https://www.shadertoy.com/user/TDM) | https://www.shadertoy.com/view/Ms2SD1 |
| Happy Jumping | [iq](https://www.shadertoy.com/user/iq) | https://www.shadertoy.com/view/3lsSzf |
| Raymarching - Primitives  | [iq](https://www.shadertoy.com/user/iq) | https://www.shadertoy.com/view/Xds3zN |

You can simply paste the content into a new `.glsl` file in this `shaders/main/` directory.

# 2) Add to the beginning of the file

```glsl
#version 330 core

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform int iFrame;
```

This respects the primary input/output mechanisms that ShaderToy uses in their API. 

# 3) Replace `mainImage`

```glsl
# replace
void mainImage( out vec4 fragColor, in vec2 fragCoord )

# with this
vec4 expensive_main()

# and add at the bottom of the function:
return fragColor;
```

(Note, the `fragColor` variable was newly declared at the top of the file as described in #2)

# 4) Replace fragCoord with `gl_FragCoord.xy`

Replace all instances of `fragCoord` (a `vec2`) with `gl_FragCoord.xy` (originally a `vec4`), note that the `.xy` is swizzling (accessing the `xy` components of the vector) the `vec4` to convert it to a `vec2`. In cases where `fragCoord.x` is used, you can instead replace with `gl_FragCoord.x` to follow the sizzling. 

# 5) [If necessary] Refactor `#if`

In a lot of cases I ran into issues with `#if` declarations in the `.glsl` files, so I simply commented them out if they casued problems. 

# 6) Point to the new file in the params

Finally, make sure the `fragment_shader` variable in `params/params.ini` (or whatever param file you are using) correctly points to the new `.glsl` file you added. 

```ini
fragment_shaders=../src/shaders/new_shader_here.glsl
```
