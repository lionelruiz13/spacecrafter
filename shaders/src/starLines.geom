//
// starLines
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (lines) in;
layout (line_strip, max_vertices = 6) out;

layout (binding=0, set=1) uniform ubo {
	mat4 Mat;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
	vec4 pos1 = custom_project( gl_in[0].gl_Position );
	pos1.z=0.0;
	vec4 pos2 = custom_project( gl_in[1].gl_Position );
	pos2.z=0.0;
	if (pos1.w == 1.0 && pos2.w == 1.0) {
		for(int i=0; i<=4; i++){
			gl_Position   = MVP2D * custom_project( gl_in[0].gl_Position*i/4 + gl_in[1].gl_Position*(1-i/4)); 
			EmitVertex();
		}
	}
	EndPrimitive();
}
