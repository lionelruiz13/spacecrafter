//
//	SUN_BIG_HALO
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

#include <cam_block.glsl>

uniform float Rmag;

in VtG
{
	vec2 position;
} vtg[];

out GtF
{
	vec2 Center;
	vec2 TexCoord;
} gtf;


//on veut représenter une texture sur un carré pour cela on construit deux triangles
void main(void)
{
	vec2 center = vtg[0].position;
	//en bas à droite
	gl_Position   =  MVP2D * vec4(center + vec2(-Rmag, -Rmag), 0, 1);
    gtf.TexCoord= vec2(0, 0);
	gtf.Center = center;
    EmitVertex();

    // en haut à droite
	gl_Position   =  MVP2D * vec4(center + vec2(-Rmag, Rmag), 0, 1);
    gtf.TexCoord= vec2(0, 1);
	gtf.Center = center;
    EmitVertex();  

	// en Bas à gauche
	gl_Position   =  MVP2D * vec4(center + vec2(Rmag, -Rmag), 0, 1);
    gtf.TexCoord= vec2(1, 0);
	gtf.Center = center;
    EmitVertex();

    // en haut à gauche
	gl_Position   =  MVP2D * vec4(center + vec2(Rmag, Rmag), 0, 1);
    gtf.TexCoord= vec2(1, 1);
	gtf.Center = center;
    EmitVertex();

    EndPrimitive();
}

 
