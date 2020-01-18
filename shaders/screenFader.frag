//
//	screen_fader
//
#version 420
#pragma debug(on)
#pragma optimize(off)

uniform float intensity;

out vec4 FragColor;
 
void main(void)
{
	FragColor = vec4(0.0,0.0,0.0,intensity);
}
