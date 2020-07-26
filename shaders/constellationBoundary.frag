//
//	CONSTELLATION_BOUNDARY
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//entrée
in float Intensity;
uniform vec3 Color;

//sortie
out vec4 FragColor;

void main(void)
{
	FragColor = vec4(Color, Intensity);
}
