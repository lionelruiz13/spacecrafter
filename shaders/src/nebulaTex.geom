//nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (triangles) in;
layout (triangle_strip , max_vertices = 3) out;

layout (push_constant) uniform ubo {
    mat4 Mat;
};

#include <cam_block.glsl>
#include <custom_project.glsl>

layout (location=0) in vec2 texCoord[3];

layout (location=0) out vec2 texCoordOut;

void main(void)
{
	vec4 pos1, pos2, pos3;

    pos1 = custom_project(gl_in[0].gl_Position);
    pos2 = custom_project(gl_in[1].gl_Position);
    pos3 = custom_project(gl_in[2].gl_Position);

	if ( pos1.w==1.0 && pos2.w==1.0 && pos3.w==1.0) {
			pos1.z = 0.0;
			pos2.z = 0.0;
			pos3.z = 0.0;

			texCoordOut = texCoord[0];
			gl_Position = MVP2D * pos1;
			EmitVertex();

			texCoordOut = texCoord[1];
			gl_Position = MVP2D * pos2;
			EmitVertex();

			texCoordOut = texCoord[2];
			gl_Position = MVP2D * pos3;
			EmitVertex();
		}
    EndPrimitive();
}
