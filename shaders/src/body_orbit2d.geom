//
//	orbit 2D
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (lines) in;
layout (line_strip, max_vertices = 2) out;

//externe
layout (binding=0) uniform uMat {mat4 Mat;};

#include <cam_block.glsl>
#include <custom_project.glsl>

void main(void)
{
	vec4 pos1, pos2, pos3, pos4;

	pos1 = custom_project(gl_in[0].gl_Position);
	pos2 = custom_project(gl_in[1].gl_Position);

	if ((pos1.w + pos2.w)==2.0) {
		pos1.z = 0.0; pos2.z = 0.0;

		//first
		gl_Position = MVP2D * pos1;
		EmitVertex();

		//second
		gl_Position = MVP2D * pos2;
		EmitVertex();

		EndPrimitive();
	}
}
