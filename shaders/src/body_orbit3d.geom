//
//	orbit 3D
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)


//#define M_PI 3.14159265358979323846

//layout uniform mat4 ModelViewProjectionMatrix;

//#include <cam_block.glsl>

//////////////////// PROJECTION FISHEYE ////////////////////////////////

// uniform mat4 inverseModelViewProjectionMatrix;
// uniform mat4 ModelViewMatrix;
//~ uniform ivec4 viewport; 
//~ uniform vec3 viewport_center;

layout (binding=2) uniform ubo {vec3 clipping_fov;};

//~ uniform float zNear;
//~ uniform float zFar;
//~ uniform float fov;

#include <fisheye.glsl>

//////////////////// PROJECTION FISHEYE ////////////////////////////////

//layout
layout (lines) in;
layout (line_strip, max_vertices = 2) out;

#define TOLERANCE 0.4

void main(void)
{
	vec4 pos1, pos2; //, pos3, pos4;

	pos1 = fisheyeProject(vec3(gl_in[0].gl_Position), clipping_fov);
	pos2 = fisheyeProject(vec3(gl_in[1].gl_Position), clipping_fov);
	//~ pos3 = gl_in[2].gl_Position;
	//~ pos4 = gl_in[2].gl_Position;

	if (clipping_fov[2] < 300. || length(pos1.xy - pos2.xy) < TOLERANCE) { // && pos3.w==1.0 && pos4.w==1.0 ) {
		//~ pos1.z = 0.0; pos2.z = 0.0;

		//first
		gl_Position = pos1;
		EmitVertex();

		//second
		gl_Position = pos2;
		EmitVertex();

		//~ //first
		//~ gl_Position = pos3;
		//~ EmitVertex();

		//~ //second
		//~ gl_Position = pos4;
		//~ EmitVertex();


		EndPrimitive();
	}
}
