//
//	pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

//uniform mat4 ModelViewProjectionMatrix;
uniform mat4 matRotation;
uniform float radius;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};


out Interpolators
{
	vec2 TexCoord;
} interData;


void main(void)
{
	//en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + matRotation *vec4(-radius, -radius,0,0));
    interData.TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + matRotation *vec4(-radius, radius,0,0));
    interData.TexCoord= vec2(.0f, 1.0f);
    EmitVertex();  

	// en bas droite
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + matRotation *vec4(radius, -radius,0,0));
    interData.TexCoord= vec2(1.0f, .0f);
    EmitVertex();

    // en haut droite
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + matRotation *vec4(radius, radius,0,0));
    interData.TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();

    EndPrimitive();
}

 

