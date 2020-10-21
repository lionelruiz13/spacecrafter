//
// starLines
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (lines) in;
layout (line_strip, max_vertices = 5) out;

layout (binding=0, set=1) uniform ubo {
	mat4 Mat;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
	vec4 pos1 = custom_project( gl_in[0].gl_Position );
	vec4 pos2 = custom_project( gl_in[1].gl_Position );
	if (pos1.w + pos2.w == 2.0) {
		gl_Position = MVP2D * pos2;
		gl_Position.z = 0;
		EmitVertex();
		for(int i=1; i<4; i++){
			gl_Position = MVP2D * custom_project( gl_in[0].gl_Position*i/4 + gl_in[1].gl_Position*(1-i/4));
			gl_Position.z = 0;
			EmitVertex();
		}
		gl_Position = MVP2D * pos1;
		gl_Position.z = 0;
		EmitVertex();
	}
	EndPrimitive();
}
