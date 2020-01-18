//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (lines_adjacency) in;
layout (triangle_strip , max_vertices = 12) out;

//uniform mat4 ModelViewProjectionMatrix;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};


//~ in float TexCoord[];
in vec2 TexCoord[];


out Interpolators
{
	//~ float intensity;
	vec2 TexCoord;
} interData;


void main()
{
	vec4 v0 = gl_in[0].gl_Position;
	vec4 v1 = gl_in[1].gl_Position;
	vec4 v2 = gl_in[2].gl_Position;
	vec4 v3 = gl_in[3].gl_Position;

	//premier triangle_strip
	//en bas gauche
	gl_Position   = MVP2D * v0;
    interData.TexCoord= TexCoord[0];
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v2)/2);
    interData.TexCoord= (TexCoord[0]+TexCoord[2])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v1)/2);
    interData.TexCoord= (TexCoord[0]+TexCoord[1])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v3)/2);
    interData.TexCoord= (TexCoord[0]+TexCoord[3])/2;
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * v1;
    interData.TexCoord= TexCoord[1];
    EmitVertex(); 

	gl_Position   = MVP2D * ((v1+v3)/2);
    interData.TexCoord= (TexCoord[1]+TexCoord[3])/2;
    EmitVertex();
    EndPrimitive();

	//second triangle_strip
	gl_Position   = MVP2D * ((v0+v2)/2);
    interData.TexCoord= (TexCoord[0]+TexCoord[2])/2;
    EmitVertex();

	// en bas droit
	gl_Position   =  MVP2D * v2;
    interData.TexCoord= TexCoord[2];
    EmitVertex();

	gl_Position   = MVP2D * ((v0+v3)/2);
    interData.TexCoord= (TexCoord[0]+TexCoord[3])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v2+v3)/2);
    interData.TexCoord= (TexCoord[2]+TexCoord[3])/2;
    EmitVertex();

	gl_Position   = MVP2D * ((v1+v3)/2);
    interData.TexCoord= (TexCoord[1]+TexCoord[3])/2;
    EmitVertex();

    // en haut droit
	gl_Position   =  MVP2D * v3;
    interData.TexCoord= TexCoord[3];
    EmitVertex();    
    EndPrimitive();
}



