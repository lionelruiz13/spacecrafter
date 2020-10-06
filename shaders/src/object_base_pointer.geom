//
//	object_base pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

//uniform mat4 ModelViewProjectionMatrix;

#include <cam_block_only.glsl>

layout (location=0) in int posC[];

layout (location=0) out vec2 TexCoord;

void motif1()
{
	//en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    TexCoord= vec2(.0f, 1.0f);
    EmitVertex();  

	// en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, -10 ,0,0));
    TexCoord= vec2(1.0f, 0.0f);
    EmitVertex();

    // en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, 10 ,0,0));
    TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();

}

void motif2()
{
	// en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    TexCoord= vec2(1.0f, .0f);
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();

	//en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, -10 ,0,0));
    TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, 10 ,0,0));
    TexCoord= vec2(.0f, 1.0f);
    EmitVertex();  
}

void motif3()
{
    // en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();  
    
    //en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    TexCoord= vec2(1.0f, .0f);
    EmitVertex();

    // en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, -10 ,0,0));
    TexCoord= vec2(0.0f, 1.0f);
    EmitVertex();

	// en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, 10 ,0,0));
    TexCoord= vec2(.0f, .0f);
    EmitVertex();
}


void motif4()
{
    // en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    TexCoord= vec2(.0f, 1.0f);
    EmitVertex();

	// en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, -10 ,0,0));
    TexCoord= vec2(1.0f, 1.0f);
    EmitVertex(); 

	//en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, 10 ,0,0));
    TexCoord= vec2(1.0f, .0f);
    EmitVertex();
}


void main()
{
	switch(posC[0])
	{
	case 1:
		motif1();
		break;
	case 2:
		motif2();
		break;
	case 3:
		motif3();
		break;
	case 4:
		motif4();
		break;
	}
}



