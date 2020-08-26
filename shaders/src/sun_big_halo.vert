//
//	sun_big_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0)in vec2 position;

out VtG
{
	vec2 position;
} vtg;

void main()
{
	vtg.position = position;
}
