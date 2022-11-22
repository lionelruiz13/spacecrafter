//
//	orbit 3D
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)


layout (push_constant) uniform uMat {
	layout (offset=16) mat4 ModelViewMatrix;
	vec3 clipping_fov;
};

layout (location=0) in vec3 position;
layout (location=0) out vec3 pos;

#include <fisheye.glsl>

void main()
{
	pos = position;
	gl_Position = fisheyeProjectClamped(position, clipping_fov);
}

