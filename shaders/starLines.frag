//
// starLines
//
#version 420
#pragma debug(on)
#pragma optimize(off)

uniform vec3 Color;
uniform float Fader;
out vec4 FragColor;

void main(void)
{
	FragColor = vec4 (Color, Fader);
}

