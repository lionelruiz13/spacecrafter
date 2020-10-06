//
//	pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

//uniform mat4 ModelViewProjectionMatrix;
layout (binding=2, set=1) uniform uboGeom {
	mat4 matRotation;
	float radius;
};

#include <cam_block_only.glsl>

layout (location=0) out vec2 TexCoord;

void main(void)
{
	//en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + matRotation *vec4(-radius, -radius,0,0));
    TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + matRotation *vec4(-radius, radius,0,0));
    TexCoord= vec2(.0f, 1.0f);
    EmitVertex();  

	// en bas droite
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + matRotation *vec4(radius, -radius,0,0));
    TexCoord= vec2(1.0f, .0f);
    EmitVertex();

    // en haut droite
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + matRotation *vec4(radius, radius,0,0));
    TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();

    EndPrimitive();
}

