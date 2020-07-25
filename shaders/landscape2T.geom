//
// landscape2T
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//fisheye projection inclusion
#include <fisheye.glsl>

// for main_clipping_fov
#include <cam_block.glsl>

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

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
	vec4 pos[3];

	for (int i =0; i<3; i++)
		pos[i] = fisheyeProject(v2g[i].Position, vec3(main_clipping_fov));

	// note: pos[i].w != 1 tell us that point is behind us
	if ( pos[0].w * pos[1].w * pos[2].w == 1.0 ) {

		for (int i =0; i<3; i++) {
			gl_Position = pos[i];
			g2f.TexCoord = v2g[i].TexCoord;
			EmitVertex();
		}
		EndPrimitive();	
	}
}
