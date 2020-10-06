//
// tully
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

layout (location=0) in vertexData
{
	float texture;
	float radius;
} vertexIn[];

layout (location=0) out Interpolators
{
	vec2 TexCoord;
	float intensity;
} interData;

layout (binding=0, set=1) ubo {
	mat4 Mat;
	vec3 camPos;
	int nbTextures;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
	// position de la galaxie
	vec4 pos = custom_project( gl_in[0].gl_Position );
	// distance de la galaxie à la caméra correspond anciennement à d=sqrt((x-a)*(x-a)+(y-b)*(y-b)+(z-c)*(z-c));
	//~ float distance = length(gl_in[0].gl_Position-vec4(camPos, 1.0)); 
	// taille apparente de la galaxie correspond à radiusTully.push_back(.3/(d*scaleTully[i]));
	//~ float radius = 0.3 / (vertexIn[0].scale * distance);
	float radius = vertexIn[0].radius;

	if ((pos.w == 1.0) && (radius>=2)) {
		// TODO : ici intensity fixé à 0.8 car radius >1.0
		//~ float intensity = max(min(radius,0.8), 0.2);
		float intensity = 1.0;
		//~ // en bas à droite
		gl_Position   = MVP2D * ( pos +vec4( radius, -radius, 0.0, 0.0) );
		interData.TexCoord= vec2((vertexIn[0].texture+1)/nbTextures, .0f);
		interData.intensity = intensity;
		EmitVertex();

		//~ // en haut à droite
		gl_Position   = MVP2D * ( pos +vec4( radius, radius, 0.0, 0.0) );
		interData.TexCoord= vec2((vertexIn[0].texture+1)/nbTextures, 1.0f);
		interData.intensity = intensity;
		EmitVertex();    

		// en Bas à gauche
		gl_Position   = MVP2D * ( pos +vec4( -radius, -radius, 0.0, 0.0) );
		interData.TexCoord= vec2(vertexIn[0].texture/nbTextures, 0.0f);
		interData.intensity = intensity;
		EmitVertex();

		// en haut à gauche
		gl_Position   = MVP2D * ( pos +vec4( -radius, radius, 0.0, 0.0) );
		interData.TexCoord= vec2(vertexIn[0].texture/nbTextures, 1.0f);
		interData.intensity = intensity;
		EmitVertex();
		EndPrimitive();
	}
}
