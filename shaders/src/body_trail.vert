//
//	body_trail
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
//~ layout (location=1)in vec3 color;
layout (location=0) in vec4 position;

layout (push_constant) uniform ubo {
	layout (offset=12) int nbPoints;
	mat4 ModelViewMatrix;
	float fader;
};

layout (location=0) out float indice;

#include <cam_block_only.glsl>
#include <fisheye2D.glsl>

void main()
{
	gl_Position = fisheye2D(position, main_clipping_fov[2]);
	indice = (1.0-0.9*gl_VertexIndex/nbPoints)*fader;
}
