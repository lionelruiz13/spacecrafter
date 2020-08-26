//
// body artificial
//
#version 420
#pragma debug(on)
#pragma optimize(off)

#include <fisheye.glsl>

//layout
layout ( triangles ) in;
layout ( triangle_strip , max_vertices = 3) out;

#include <cam_block.glsl>

//externe
uniform bool useTexture;
uniform mat4 NormalMatrix;
uniform vec3 clipping_fov;

//in-out
layout (location=0) in vertexData
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
	float Ambient;
} vertexIn[];

layout (location=0) out colorFrag
{
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    float Ambient;
} cfOut;



void main(void)
{
	vec4 pos[3];

    for(int i=0; i<3; i++)
        pos[i] = fisheyeProject(vertexIn[i].Position, clipping_fov);
    
	if ( pos[0].w==1.0 && pos[1].w==1.0 && pos[2].w==1.0) {
        for(int i=0; i<3; i++) {    
            cfOut.Position = vertexIn[i].Position;
            cfOut.TexCoord = vertexIn[i].TexCoord;
            cfOut.Normal = vertexIn[i].Normal;
            cfOut.Ambient = vertexIn[i].Ambient;
            gl_Position = pos[i];
		    EmitVertex();
        }
	}
    EndPrimitive();
}
