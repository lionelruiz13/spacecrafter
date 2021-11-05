//
//	body_trace
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (lines) in;
layout (line_strip , max_vertices = 2) out;

layout (push_constant) uniform uMat {mat4 Mat;};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main(void)
{
	vec4 pos1, pos2;
	//~ for (int i=0; i < gl_in.length()-1; ++i)
	//~ {
    pos1 = custom_project(gl_in[0].gl_Position);
    pos2 = custom_project(gl_in[1].gl_Position);
    //~ pos1 = custom_project(gl_in[0].gl_Position);
    //~ pos2 = custom_project(gl_in[1].gl_Position);

    
		if ( pos1.w==1.0 && pos2.w==1.0 ) {
			pos1.z = 0.0;
			pos2.z = 0.0;
			gl_Position = MVP2D * pos1;
			EmitVertex();

			gl_Position = MVP2D * pos2;
			EmitVertex();
		}
    EndPrimitive();
}
