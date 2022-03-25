//
//	sun_big_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0) in vec2 pos;

layout(location=0) out vec2 posOut;

#include <cam_block.glsl>

void main()
{
	posOut = pos;
}
