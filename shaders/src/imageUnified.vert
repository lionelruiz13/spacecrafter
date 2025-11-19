//
// imageUnified
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec3 position;
layout (location=1)in vec2 texCoord;

// #include <cam_block_only.glsl>

layout (push_constant) uniform uVert {
	mat4 ModelViewMatrix;
	layout (offset=64) vec3 clipping_fov;
};
#include <cam_block.glsl>
#include <custom_project.glsl>

layout (location=0) out vec2 TexCoord;

//////////////////// PROJECTION FISHEYE ////////////////////////////////

void main()
{
	gl_Position = custom_project(position, ModelViewMatrix, clipping_fov);
	TexCoord = texCoord;
}
