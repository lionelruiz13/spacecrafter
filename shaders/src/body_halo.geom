//
//	body_halo
//
#version 420

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

#include <cam_block_only.glsl>

layout(location=0) in vec2 pos[];
layout(location=1) in vec4 Color[];
layout(location=2) in float rmag[];

layout(location=0) out vec2 TexCoord;
layout(location=1) flat out vec4 ColorOut;

void main(void)
{
	gl_Position = MVP2D * vec4(pos[0].x-rmag[0], pos[0].y-rmag[0], 0, 1);
	TexCoord = vec2(0, 0);
	ColorOut = Color[0];
	EmitVertex();

	gl_Position = MVP2D * vec4(pos[0].x-rmag[0], pos[0].y+rmag[0], 0, 1);
	TexCoord = vec2(0, 1);
	ColorOut = Color[0];
	EmitVertex();

	gl_Position = MVP2D * vec4(pos[0].x+rmag[0], pos[0].y-rmag[0], 0, 1);
	TexCoord = vec2(1, 0);
	ColorOut = Color[0];
	EmitVertex();

	gl_Position = MVP2D * vec4(pos[0].x+rmag[0], pos[0].y+rmag[0], 0, 1);
	TexCoord = vec2(1, 1);
	ColorOut = Color[0];
	EmitVertex();

	EndPrimitive();
}
