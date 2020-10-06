//
// illuminate
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location=0) in Data
{
    vec3 position;
	vec2 texCoord;
    vec3 texColor;
} data[];

layout (location=0) out DataFrag
{
	vec2 texCoord;
    vec3 texColor;
} dataFrag;


#include <cam_block.glsl>

layout (binding=1) uniform ubo {
    mat4 ModelViewMatrix;
};
#include <custom_project.glsl>


//////////////////// PROJECTION FISHEYE ////////////////////////////////


void main(void)
{
	vec4 pos1, pos2, pos3;
	pos1 = custom_project(data[0].position);
	pos2 = custom_project(data[1].position);
	pos3 = custom_project(data[2].position);

	if ( pos1.w==1.0 && pos2.w==1.0 && pos3.w==1.0) {

		gl_Position = MVP2D * pos1;
		dataFrag.texCoord = data[0].texCoord;
        dataFrag.texColor = data[0].texColor;
		EmitVertex();

		gl_Position = MVP2D * pos2;
		dataFrag.texCoord = data[1].texCoord;
		dataFrag.texColor = data[1].texColor;
		EmitVertex();

		gl_Position = MVP2D * pos3;
		dataFrag.texCoord = data[2].texCoord;
		dataFrag.texColor = data[2].texColor;
		EmitVertex();

		EndPrimitive();	
	}
}
