//
// my earth tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)


layout ( triangles ) in;
layout ( triangle_strip , max_vertices = 3) out;

layout (binding=11) uniform sampler2D heightmapTexture;

layout (binding=0) uniform globalVertProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
};

#include <cam_block.glsl>
#include <fisheye_noMV.glsl>

layout (binding=3) uniform globalTescGeom {
	ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};


float coeffHeightMap = 0.05 * float(TesParam[2]);

//out
//layout (location=0) out vec2 TexCoord;

layout (location=0) in TES_OUT
{
    in vec3 glPosition; 	
    in vec2 TexCoord;
    in vec3 Normal;
    //~ in vec3 tangent;
    in vec3 tessCoord;
}gs_in[];

layout (location=0) out GS_OUT {
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    vec3 TangentLight;
    vec3 Light;
    vec3 ViewDirection;
} gs_out;

void main()
{
	vec3 binormal, Normal, tangent;
	vec3 glPosition;
	vec3 positionL, Position, Light, ViewDirection, TangentLight;

	for(int i=0; i<3; i++) {

		glPosition = gs_in[i].glPosition;
		//~ sans normalMap
		//~ glPosition.xyz= glPosition.xyz/length(glPosition.xyz)*planetScaledRadius ;
		//~ avec normalMap
		glPosition= glPosition/length(glPosition)*planetScaledRadius * (1.0+texture(heightmapTexture,gs_in[i].TexCoord).x * coeffHeightMap);

		gl_Position = fisheyeProject(glPosition, clipping_fov);

		positionL = planetRadius * gs_in[i].Normal ;
		positionL.z = positionL.z * planetOneMinusOblateness;

		Position = vec3(ModelViewMatrix * vec4(positionL,1.0));
 		Light = normalize(LightPosition - Position);
		ViewDirection = normalize(-Position);

		//Other
		Normal = normalize(mat3(NormalMatrix) * gs_in[i].Normal);
		binormal = vec3(0,-Normal.z,Normal.y);
		tangent = cross(Normal,binormal);

		TangentLight = vec3(dot(Light, tangent), dot(Light, binormal), dot(Light, Normal)); 

		gs_out.Position = Position;
		gs_out.Light = Light;
		gs_out.ViewDirection = ViewDirection;
		gs_out.Normal = Normal;
		gs_out.TangentLight = TangentLight;
		gs_out.TexCoord = gs_in[i].TexCoord;
		
		EmitVertex();
	}

	EndPrimitive();
}
