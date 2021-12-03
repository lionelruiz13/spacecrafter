//
// tully
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (points) in;
layout (points , max_vertices = 1) out;

layout (location=0) in float scale[1];
layout (location=1) in float texture[1];
layout (location=2) in vec3 color[1];

layout (location=0) out vec3 TexColor;
layout (location=1) out float intensityOut;

layout (binding=0, set=1) uniform ubo {
	mat4 Mat;
	vec3 camPos;
	//int nbTextures;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
	// position de la galaxie
	vec4 pos = custom_project( gl_in[0].gl_Position );
	// distance de la galaxie à la caméra correspond anciennement à d=sqrt((x-a)*(x-a)+(y-b)*(y-b)+(z-c)*(z-c));
	float distance = length(gl_in[0].gl_Position-vec4(camPos, 1.0)); 
	// taille apparente de la galaxie correspond à radiusTully.push_back(.3/(d*scaleTully[i]));
	float radius = 0.3 / (scale[0] * distance);

	if (pos.w == 1.0) {
		if (radius<2.0) {
			float intensity = max(min(radius,0.9), 0.3);
			gl_Position   = MVP2D * ( pos );
			gl_PointSize = radius;
			TexColor= color[0];
			intensityOut = intensity;
			EmitVertex();
			EndPrimitive();
		}
	}
}
