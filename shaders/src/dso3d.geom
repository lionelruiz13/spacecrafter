//
// dso3d
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#define M_PI 3.14159265358979323846

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

layout (location=0) in float scale[1];
layout (location=1) in float texture[1];

layout (location=0) out vec2 TexCoord;

layout (binding=0, set=1) uniform uGeom {
	mat4 Mat;
	vec3 camPos;
	int nbTextures;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
	// position de la nebuleuse
	vec4 pos = custom_project( gl_in[0].gl_Position );
	// todo : utiliser length
	vec4 dist = gl_in[0].gl_Position-vec4(camPos, 1.0); 
	float distance = sqrt(dist.x*dist.x + dist.y*dist.y + dist.z*dist.z );
	float radius = scale[0] * 60. / distance;

	if (pos.w == 1.0) {
		if (radius>=1.0) {
			// en bas à droite
			float tex = (texture[0]+1)/nbTextures;
			gl_Position   = MVP2D * ( pos +vec4( radius, -radius, 0.0, 0.0) );
			TexCoord= vec2(tex, .0f);
			EmitVertex();
	
			// en haut à droite
			gl_Position   = MVP2D * ( pos +vec4( radius, radius, 0.0, 0.0) );
			TexCoord= vec2(tex, 1.0f);
			EmitVertex();    

			tex = texture[0]/nbTextures;
			// en Bas à gauche
			gl_Position   = MVP2D * ( pos +vec4( -radius, -radius, 0.0, 0.0) );
			TexCoord= vec2(tex, 0.0f);
			EmitVertex();
	
			// en haut à gauche
			gl_Position   = MVP2D * ( pos +vec4( -radius, radius, 0.0, 0.0) );
			TexCoord= vec2(tex, 1.0f);
			EmitVertex();
			EndPrimitive();
		}
	}
}
