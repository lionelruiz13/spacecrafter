//
// landscape
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// for main_clipping_fov
#include <cam_block_only.glsl>

//entrées - sorties du pipeline
layout (location=0) in vec2 TexCoordIn[3];
layout (location=1) in vec4 pos[3];

layout (location=0) out vec2 TexCoord;

// maximal distance accepted
#define TOLERANCE 0.6

void main(void)
{
	vec2 pos0 = pos[0].xy;
	if (main_clipping_fov[2] < 2.7 || (length(pos0 - pos[1].xy) + length(pos0 - pos[2].xy)) < TOLERANCE) {
		// delete triangles with too many spaced points

		for (int i =0; i<3; i++) {
			gl_Position = pos[i];
			TexCoord = TexCoordIn[i];
			EmitVertex();
		}
		EndPrimitive();	
	}
}
