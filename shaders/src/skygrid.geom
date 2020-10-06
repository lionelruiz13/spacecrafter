//skygrid

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (lines) in;
layout (line_strip , max_vertices = 2) out;

layout (binding=0, set=1) uniform ubo {
	mat4 Mat;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

layout (location=0) in float intensityColor[2];

layout (location=0) out float intensityColorOut;

void main(void)
{
	vec4 pos0, pos1;

    pos0 = custom_project(gl_in[0].gl_Position);
    pos1 = custom_project(gl_in[1].gl_Position);
    
	if ( pos0.w==1.0 && pos1.w==1.0 ) {
		pos0.z = 0.0;
		pos1.z = 0.0;
		gl_Position = MVP2D * pos0;
		intensityColorOut = intensityColor[0];
		EmitVertex();

		intensityColorOut = intensityColor[1];
		gl_Position = MVP2D * pos1;
		EmitVertex();
	}

    EndPrimitive();
}
