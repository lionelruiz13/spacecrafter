//
//	atmosphere
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec2 position;
layout (location=3)in vec3 color;

//out
layout (location=0) out vec3 Color;

// for MVP2D
#include <cam_block_only.glsl>

void main()
{
	gl_Position = MVP2D * vec4(position,0.0,1.0);
    Color = color;
}
