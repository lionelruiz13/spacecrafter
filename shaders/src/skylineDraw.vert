//
// skylineDraw
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec2 position;

#include <cam_block_only.glsl>

void main()
{
	gl_Position = MVP2D * vec4(position,0.0,1.0);
}
