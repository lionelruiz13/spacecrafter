//
// landscape2T
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//fisheye projection inclusion
#include <fisheye.glsl>

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

//externe
uniform vec3 clipping_fov;

//entrées - sorties du pipeline
in V2G
{
	vec3 Position;
	vec2 TexCoord;
} v2g[];

out G2F
{
	vec2 TexCoord;
} g2f;


void main(void)
{
	vec4 pos1, pos2, pos3;

	pos1 = custom_project(v2g[0].Position, clipping_fov);
	pos2 = custom_project(v2g[1].Position, clipping_fov);
	pos3 = custom_project(v2g[2].Position, clipping_fov);

	if ( pos1.w * pos2.w * pos3.w == 1.0 ) {

		gl_Position = pos1;
		g2f.TexCoord = v2g[0].TexCoord;
		EmitVertex();

		gl_Position = pos2;
		g2f.TexCoord = v2g[1].TexCoord;
		EmitVertex();

		gl_Position = pos3;
		g2f.TexCoord = v2g[2].TexCoord;
		EmitVertex();
		EndPrimitive();	
	}
}
