//
//	SUN_BIG_HALO
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

#include <cam_block.glsl>

layout(binding=1) uniform ubo {float Rmag;};

layout (location=0) in vec2 position[1];

layout (location=0) out vec2 Center;
layout (location=1) out vec2 TexCoord;
layout (location=2) out float viewport_y;

//on veut représenter une texture sur un carré pour cela on construit deux triangles
void main(void)
{
	vec2 center = position[0];
	//en bas à droite
	gl_Position   =  MVP2D * vec4(center + vec2(-Rmag, -Rmag), 0, 1);
    TexCoord= vec2(0, 0);
	Center = center;
	viewport_y = viewport.y;
    EmitVertex();

    // en haut à droite
	gl_Position   =  MVP2D * vec4(center + vec2(-Rmag, Rmag), 0, 1);
    TexCoord= vec2(0, 1);
	Center = center;
	viewport_y = viewport.y;
    EmitVertex();  

	// en Bas à gauche
	gl_Position   =  MVP2D * vec4(center + vec2(Rmag, -Rmag), 0, 1);
    TexCoord= vec2(1, 0);
	Center = center;
	viewport_y = viewport.y;
    EmitVertex();

    // en haut à gauche
	gl_Position   =  MVP2D * vec4(center + vec2(Rmag, Rmag), 0, 1);
    TexCoord= vec2(1, 1);
	Center = center;
	viewport_y = viewport.y;
    EmitVertex();

    EndPrimitive();
}

 
