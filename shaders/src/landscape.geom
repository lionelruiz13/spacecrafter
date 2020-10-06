//
// landscape
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//fisheye projection inclusion
layout (binding=2, set=1) uniform uModelViewMatrix {mat4 ModelViewMatrix;};
#include <fisheye_noMV.glsl>

// for main_clipping_fov
#include <cam_block_only.glsl>

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

//entrées - sorties du pipeline
layout (location=0) in vec2 TexCoordIn[3];
layout (location=1) in vec3 Position[3];

layout (location=0) out vec2 TexCoord;

void main(void)
{
	vec4 pos[3];

	for (int i =0; i<3; i++)
		pos[i] = fisheyeProject(Position[i], vec3(main_clipping_fov));

	// note: pos[i].w != 1 tell us that point is behind us
	if ( pos[0].w * pos[1].w * pos[2].w == 1.0 ) {

		for (int i =0; i<3; i++) {
			gl_Position = pos[i];
			TexCoord = TexCoordIn[i];
			EmitVertex();
		}
		EndPrimitive();	
	}
}
