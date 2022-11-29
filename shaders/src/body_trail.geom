//
//	body_trail
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (lines) in;
layout (line_strip, max_vertices = 2) out;

layout (location=0) in float indice[2];
layout (location=0) out float indiceOut;

#include <cam_block_only.glsl>

#define SQUARED_TOLERANCE 0.16

void main(void)
{
	vec2 dist = gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy;

	if (main_clipping_fov[2] < 2.7 || dot(dist, dist) < SQUARED_TOLERANCE) {
		gl_Position = gl_in[0].gl_Position;
		indiceOut = indice[0];
		EmitVertex();

		gl_Position = gl_in[1].gl_Position;
		indiceOut = indice[1];
		EmitVertex();

		EndPrimitive();
	}
}
