//nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (triangles) in;
layout (triangle_strip , max_vertices = 3) out;

layout (location=0) in vec2 texCoord[3];

layout (location=0) out vec2 texCoordOut;

#include <cam_block_only.glsl>

void main(void)
{
	if (gl_in[0].gl_Position.w + gl_in[1].gl_Position.w + gl_in[2].gl_Position.w > 2.5) {

		texCoordOut = texCoord[0];
		gl_Position = MVP2D * gl_in[0].gl_Position;
		EmitVertex();

		texCoordOut = texCoord[1];
		gl_Position = MVP2D * gl_in[1].gl_Position;
		EmitVertex();

		texCoordOut = texCoord[2];
		gl_Position = MVP2D * gl_in[2].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
