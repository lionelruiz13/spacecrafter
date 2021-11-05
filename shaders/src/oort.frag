//
// oort
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=1) uniform uFrag {
	vec4 color;
};

layout (location=0) out vec4 Color;

void main(void)
{
	Color = color;
}
