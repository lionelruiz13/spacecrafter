//
// illuminate
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location=0) in vec3 position[3];
layout (location=1) in vec2 texCoord[3];
layout (location=2) in vec3 texColor[3];

layout (location=0) out vec2 texCoordOut;
layout (location=1) out vec3 texColorOut;


#include <cam_block.glsl>

layout (push_constant) uniform ubo {
    mat4 Mat; // ModelViewMatrix
};
#include <custom_project_nocheck.glsl>

// maximal squared distance accepted
#define TOLERANCE 60000.0
//////////////////// PROJECTION FISHEYE ////////////////////////////////


void main(void)
{
	vec4 pos1, pos2, pos3;
	pos1 = custom_project(position[0]);
	pos2 = custom_project(position[1]);
	pos3 = custom_project(position[2]);
	vec2 dist1 = pos1.xy;
	vec2 dist2 = dist1 - pos3.xy;
	dist1 -= pos2.xy;

	if (main_clipping_fov[2] < 1 || ((dot(dist1, dist1) + dot(dist2, dist2)) < TOLERANCE)) {

		gl_Position = MVP2D * pos1;
		texCoordOut = texCoord[0];
		texColorOut = texColor[0];
		EmitVertex();

		gl_Position = MVP2D * pos2;
		texCoordOut = texCoord[1];
		texColorOut = texColor[1];
		EmitVertex();

		gl_Position = MVP2D * pos3;
		texCoordOut = texCoord[2];
		texColorOut = texColor[2];
		EmitVertex();

		EndPrimitive();	
	}
}
