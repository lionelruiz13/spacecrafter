//
//	body_Hints
//
#version 420
#pragma debug(on)
#pragma optimize(off)


uniform vec3 Color;
uniform float fader; 
 
out vec4 FragColor;

void main(void)
{
	//~ FragColor = Color;
	FragColor = vec4( Color , fader);
	//~ FragColor = vec4( 1.0,0.0,0.0,1.0);
}


