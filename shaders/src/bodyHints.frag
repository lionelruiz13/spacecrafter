//
//	body_Hints
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform uColor {vec3 Color;};
layout (binding=1, set=1) uniform uFader {uniform float fader;};
 
layout (location=0) out vec4 FragColor;

void main(void)
{
	//~ FragColor = Color;
	FragColor = vec4( Color , fader);
	//~ FragColor = vec4( 1.0,0.0,0.0,1.0);
}


