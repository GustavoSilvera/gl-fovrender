#version 330 core
// modified from https://www.shadertoy.com/view/WdsGRl

layout(location = 0) out vec4 fragColor;
uniform vec2 iResolution;
uniform float iTime;

#define t iTime


void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = ( gl_FragCoord.xy - .5*iResolution.xy) / iResolution.y;
    vec3 col = vec3(0.);
    float a = atan(uv.y,uv.x);
    float r = 0.5*length(uv);
    float counter = 100.;
    a = 4.*a+20.*r+50.*cos(r)*cos(.1*t)+abs(a*r);
    float f = 0.02*abs(cos(a))/(r*r);

    vec2 v = vec2(0.);
    for(float i=0.;i<counter;i++){
    	v = mat2(v,-v.y,v.x) * v + vec2(2.*f+cos(0.5*t*(exp(-0.2* r))),-cos(t*r*r)*cos(0.5*t));
        if(length(v)>2.){
        	counter = i;
            break;
        }
    }

    col=vec3(min(0.9,1.2*exp(-pow(f,0.45)*counter)));

	fragColor =    min(0.9,1.2*exp(-pow(f,0.45)*counter) )
                * ( 0.7 + 0.3* cos(10.*r - 2.*t -vec4(.7,1.4,2.1,0) ) );
}