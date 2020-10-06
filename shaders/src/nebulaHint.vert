//
//	nebulaHint
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec2 position;
layout (location=1)in vec2 texCoord;
layout (location=3)in vec3 color;

//externe
//~ uniform mat4 ModelViewProjectionMatrix;

#include <cam_block.glsl>

//out
layout (location=1) out vec2 TexCoord;
layout (location=0) out vec3 Color;

void main()
{
	gl_Position = MVP2D * vec4(position,0.0,1.0);
    TexCoord = texCoord;
    Color = color;
}


