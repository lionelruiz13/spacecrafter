//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (lines_adjacency) in;
layout (triangle_strip , max_vertices = 12) out;

// for MVP2D
#include <cam_block_only.glsl>

layout (location=0) in vec2 Position[4];
layout (location=1) in vec2 TexCoord[4];

layout (location=0) out vec2 TexCoordOut;

void main()
{
	vec4 v0 = vec4(Position[0], 0,1);
	vec4 v1 = vec4(Position[1], 0,1);
	vec4 v2 = vec4(Position[2], 0,1);
	vec4 v3 = vec4(Position[3], 0,1);

	//premier triangle_strip
	//en bas gauche
	gl_Position   = MVP2D * v0;
    TexCoordOut= TexCoord[0];
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v2)/2);
    TexCoordOut= (TexCoord[0]+TexCoord[2])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v1)/2);
    TexCoordOut= (TexCoord[0]+TexCoord[1])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v3)/2);
    TexCoordOut= (TexCoord[0]+TexCoord[3])/2;
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * v1;
    TexCoordOut= TexCoord[1];
    EmitVertex(); 

	gl_Position   = MVP2D * ((v1+v3)/2);
    TexCoordOut= (TexCoord[1]+TexCoord[3])/2;
    EmitVertex();
    EndPrimitive();

	//second triangle_strip
	gl_Position   = MVP2D * ((v0+v2)/2);
    TexCoordOut= (TexCoord[0]+TexCoord[2])/2;
    EmitVertex();

	// en bas droit
	gl_Position   =  MVP2D * v2;
    TexCoordOut= TexCoord[2];
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v3)/2);
    TexCoordOut= (TexCoord[0]+TexCoord[3])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v2+v3)/2);
    TexCoordOut= (TexCoord[2]+TexCoord[3])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v1+v3)/2);
    TexCoordOut= (TexCoord[1]+TexCoord[3])/2;
    EmitVertex();

    // en haut droit
	gl_Position   =  MVP2D * v3;
    TexCoordOut= TexCoord[3];
    EmitVertex();    
    EndPrimitive();
}
