//
//	CONSTELLATION_BOUNDARY
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//entrée
layout (location=0) in float Intensity;
layout (binding=0, set=1) uniform ubo {
	vec3 Color;
};

//sortie
layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = vec4(Color, Intensity);
}
