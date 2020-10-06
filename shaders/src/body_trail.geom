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

//externe
//uniform mat4 ModelViewProjectionMatrix;
layout (binding=0) uniform uMat {mat4 Mat;};

#include <cam_block.glsl>
#include <custom_project.glsl>

layout (location=0) in ValueFader
{
	smooth float indice;
} valueFader[];

layout (location=0) out FaderColor
{
	smooth float indice;
} faderColor;

void main(void)
{
	vec4 pos1, pos2;

	pos1 = custom_project(gl_in[0].gl_Position);
	pos2 = custom_project(gl_in[1].gl_Position);

	if ( pos1.w==1.0 && pos2.w==1.0) {
		pos1.z = 0.0; pos2.z==1.0;

		gl_Position = MVP2D * pos1;
		faderColor.indice = valueFader[0].indice;
		EmitVertex();

		gl_Position = MVP2D * pos2;
		faderColor.indice = valueFader[1].indice;
		EmitVertex();

		EndPrimitive();	
	}
}
