//
//	CONSTELLATION_BOUNDARY
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0) in vec2 position;
layout (location=1) in float intensity;

// for MVP2D
#include <cam_block_only.glsl>

layout (location=0) out float Intensity;

void main()
{
	Intensity = intensity;
	gl_Position = MVP2D * vec4(position,0.0,1.0);
}
