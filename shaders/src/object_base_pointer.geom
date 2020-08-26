//
//	object_base pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

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


in int posC[];

out Interpolators
{
	vec2 TexCoord;
} interData;


void motif1()
{
	//en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    interData.TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    interData.TexCoord= vec2(.0f, 1.0f);
    EmitVertex();  

	// en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, -10 ,0,0));
    interData.TexCoord= vec2(1.0f, 0.0f);
    EmitVertex();

    // en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, 10 ,0,0));
    interData.TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();

}

void motif2()
{
	// en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    interData.TexCoord= vec2(1.0f, .0f);
    EmitVertex();

    // en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    interData.TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();

	//en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, -10 ,0,0));
    interData.TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, 10 ,0,0));
    interData.TexCoord= vec2(.0f, 1.0f);
    EmitVertex();  
}

void motif3()
{
    // en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    interData.TexCoord= vec2(1.0f, 1.0f);
    EmitVertex();  
    
    //en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    interData.TexCoord= vec2(1.0f, .0f);
    EmitVertex();

    // en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, -10 ,0,0));
    interData.TexCoord= vec2(0.0f, 1.0f);
    EmitVertex();

	// en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(10, 10 ,0,0));
    interData.TexCoord= vec2(.0f, .0f);
    EmitVertex();
}


void motif4()
{
    // en bas gauche
	gl_Position   = MVP2D * (gl_in[0].gl_Position + vec4( -10, -10 ,0,0));
    interData.TexCoord= vec2(.0f, 1.0f);
    EmitVertex();

	// en haut gauche
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4(-10, 10 ,0,0));
    interData.TexCoord= vec2(.0f, .0f);
    EmitVertex();

    // en bas droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, -10 ,0,0));
    interData.TexCoord= vec2(1.0f, 1.0f);
    EmitVertex(); 

	//en haut droit
	gl_Position   =  MVP2D * (gl_in[0].gl_Position + vec4( 10, 10 ,0,0));
    interData.TexCoord= vec2(1.0f, .0f);
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



