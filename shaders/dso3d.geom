//
// dso3d
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

//fisheye projection inclusion
#include <fisheye.glsl>

// for main_clipping_fov
#include <cam_block.glsl>

in V2G
{
	vec3 Position;
	float scale;
	float texture;
} v2g[];


out G2F
{
	vec2 TexCoord;
} g2f;

uniform vec3 camPos;
uniform int nbTextures;

void main()
{
	// position de la nebuleuse
	vec4 pos = fisheyeProject(v2g[0].Position, vec3(main_clipping_fov));

	if (pos.w == 1.0) {
		vec3 dist = v2g[0].Position-camPos; 
		float distance = dist.length();
		float radius = v2g[0].scale * 60. / distance;

		if (radius>=1.0) {
			// en bas à droite
			gl_Position   = MVP2D * ( pos +vec4( radius, -radius, 0.0, 0.0) );
			g2f.TexCoord= vec2((v2g[0].texture+1)/nbTextures, .0f);
			EmitVertex();
	
			// en haut à droite
			gl_Position   = MVP2D * ( pos +vec4( radius, radius, 0.0, 0.0) );
			g2f.TexCoord= vec2((v2g[0].texture+1)/nbTextures, 1.0f);
			EmitVertex();    
	
			// en Bas à gauche
			gl_Position   = MVP2D * ( pos +vec4( -radius, -radius, 0.0, 0.0) );
			g2f.TexCoord= vec2(v2g[0].texture/nbTextures, 0.0f);
			EmitVertex();
	
			// en haut à gauche
			gl_Position   = MVP2D * ( pos +vec4( -radius, radius, 0.0, 0.0) );
			g2f.TexCoord= vec2(v2g[0].texture/nbTextures, 1.0f);
			EmitVertex();
			EndPrimitive();
		}
	}
}
