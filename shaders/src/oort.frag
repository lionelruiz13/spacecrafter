//
// oort
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=1) uniform uFrag {
	vec3 color;
	float intensity;
};

layout (location=0) out vec4 Color;

void main(void)
{
	Color = vec4 (color, intensity);
}
