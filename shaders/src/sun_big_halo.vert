//
//	sun_big_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0) in vec2 position;

layout(location=0) out vec2 positionOut;

void main()
{
	positionOut = position;
}
