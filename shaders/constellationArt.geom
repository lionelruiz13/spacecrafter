//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (lines_adjacency) in;
layout (triangle_strip , max_vertices = 12) out;


// for MVP2D
#include <cam_block.glsl>

in V2G {
	vec2 Position;
	vec2 TexCoord;
} v2g[];

out G2F
{
	vec2 TexCoord;
} g2f;


void main()
{
	vec4 v0 = vec4(v2g[0].Position, 0,1);
	vec4 v1 = vec4(v2g[1].Position, 0,1);
	vec4 v2 = vec4(v2g[2].Position, 0,1);
	vec4 v3 = vec4(v2g[3].Position, 0,1);

	//premier triangle_strip
	//en bas gauche
	gl_Position   = MVP2D * v0;
    g2f.TexCoord= v2g[0].TexCoord;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v2)/2);
    g2f.TexCoord= (v2g[0].TexCoord+v2g[2].TexCoord)/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v1)/2);
    g2f.TexCoord= (v2g[0].TexCoord+v2g[1].TexCoord)/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v3)/2);
    g2f.TexCoord= (v2g[0].TexCoord+v2g[3].TexCoord)/2;
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * v1;
    g2f.TexCoord= v2g[1].TexCoord;
    EmitVertex(); 

	gl_Position   = MVP2D * ((v1+v3)/2);
    g2f.TexCoord= (v2g[1].TexCoord+v2g[3].TexCoord)/2;
    EmitVertex();
    EndPrimitive();

	//second triangle_strip
	gl_Position   = MVP2D * ((v0+v2)/2);
    g2f.TexCoord= (v2g[0].TexCoord+v2g[2].TexCoord)/2;
    EmitVertex();

	// en bas droit
	gl_Position   =  MVP2D * v2;
    g2f.TexCoord= v2g[2].TexCoord;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v3)/2);
    g2f.TexCoord= (v2g[0].TexCoord+v2g[3].TexCoord)/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v2+v3)/2);
    g2f.TexCoord= (v2g[2].TexCoord+v2g[3].TexCoord)/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v1+v3)/2);
    g2f.TexCoord= (v2g[1].TexCoord+v2g[3].TexCoord)/2;
    EmitVertex();

    // en haut droit
	gl_Position   =  MVP2D * v3;
    g2f.TexCoord= v2g[3].TexCoord;
    EmitVertex();    
    EndPrimitive();
}
