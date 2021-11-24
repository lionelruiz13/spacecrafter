//
// sfontHorizontal
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec4 position;
layout (location=1)in vec2 texCoord;
layout (location=2)in vec2 texCoord2;

#include <cam_block_only.glsl>

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec2 TexCoord2;

void main()
{
	gl_Position = MVP2D * position;
	TexCoord = texCoord;
	TexCoord2 = texCoord2;
}
