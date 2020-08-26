//
// imageUnified
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec3 position;
layout (location=1)in vec2 texCoord;

#include <cam_block.glsl>

#include <fisheye.glsl>

uniform vec3 clipping_fov;
smooth out vec2 TexCoord;

//////////////////// PROJECTION FISHEYE ////////////////////////////////

void main()
{
	gl_Position = fisheyeProject(position, clipping_fov);
	TexCoord = texCoord;
}
