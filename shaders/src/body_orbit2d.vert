//
//	orbit 2D
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0) in vec4 position;

layout (location=0) out vec4 pos;

layout (push_constant) uniform uMat {
	layout (offset=16) mat4 ModelViewMatrix;
	float fov;
};

#include <fisheye2D.glsl>

void main()
{
	pos = position;
	gl_Position = fisheye2D(position, fov);
}

