//
//	meteor
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec2 position;
layout (location=3)in vec4 color;

//externe
//~ uniform mat4 ModelViewProjectionMatrix;

#include <cam_block_only.glsl>

//out
layout (location=0) out vec4 Color;


void main()
{
	gl_Position = MVP2D * vec4(position,0.0,1.0);
    Color = color;
}

