//
//	STARS
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


in vertexData
{
	float mag;
	vec3 color;
} vertexIn[];

out Interpolators
{
	vec2 TexCoord;
	vec3 TexColor;
} interData;


//on veut représenter une texture sur un carré pour cela on construit deux triangles
void main(void)
{
	
	//en bas à droite
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(vertexIn[0].mag/2, -vertexIn[0].mag/2, 0.0, 0.0));
    interData.TexCoord= vec2(1.0f, .0f);
    interData.TexColor= vertexIn[0].color;
    EmitVertex();

    // en haut à droite
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(vertexIn[0].mag/2, vertexIn[0].mag/2, 0.0, 0.0));
    interData.TexCoord= vec2(1.0f, 1.0f);
    interData.TexColor= vertexIn[0].color;
    EmitVertex();    

	// en Bas à gauche
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(-vertexIn[0].mag/2, -vertexIn[0].mag/2, 0.0,0.0));
    interData.TexCoord= vec2(0.0f, 0.0f);
    interData.TexColor= vertexIn[0].color;
    EmitVertex();

    // en haut à gauche
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(-vertexIn[0].mag/2, vertexIn[0].mag/2,0.0,0.0));
    interData.TexCoord= vec2(0.0f, 1.0f);
    interData.TexColor= vertexIn[0].color;
    EmitVertex();


    EndPrimitive();
}

 
