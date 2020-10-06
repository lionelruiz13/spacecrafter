//
// my moon tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#define M_PI 3.14159265358979323846

layout ( triangles ) in;
layout ( triangle_strip , max_vertices = 3) out;

layout (binding=8) uniform sampler2D heightmapTexture;

#include <cam_block.glsl>

//externe
//uniform mat4 ModelViewProjectionMatrix;

layout (binding=0) uniform globalVertProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
};

#include <fisheye_noMV.glsl>

layout (binding=3) uniform globalTescGeom {
    ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};

layout (location=0) in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];


layout (location=0) out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};


//out
//smooth out vec2 TexCoord;
// out float Ambient;
//~ out vec3 Normal;
//~ out vec3 Position;
//~ out vec3 TangentLight;
//~ out vec3 Light;
//~ out vec3 ViewDirection;

layout (location=1) in TES_OUT
{
    in vec2 TexCoord;
    in vec3 Normal;
    //~ in vec3 tangent;
    in vec3 tessCoord;
}gs_in[];

//~ in VS_OUT{
    //~ //vec3 PositionL;
    //~ vec2 TexCoord;
    //~ vec3 Normal;
//~ } vs_in[];

layout (location=1) out GS_OUT {
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    vec3 Light;
	vec3 TangentLight;
} gs_out;

//////////////////// PROJECTION FISHEYE ////////////////////////////////
void main()
{
	vec3 glPosition;
	vec3 positionL, Position, Light, Normal;
	float NdotL;

	for(int i=0; i<3; i++) {

		glPosition = vec3(gl_in[i].gl_Position);
		//~ sans normalMap
		//~ glPosition.xyz= glPosition.xyz/length(glPosition.xyz)*planetScaledRadius ;
		//~ avec normalMap
		glPosition.xyz= glPosition.xyz/length(glPosition.xyz)*planetScaledRadius * (1.0+texture(heightmapTexture,gs_in[i].TexCoord).x * coeffHeightMap);

		//gl_Position = ModelViewProjectionMatrix * posToFisheye(glPosition);
		gl_Position = fisheyeProject(glPosition, clipping_fov);

		positionL = planetRadius * gs_in[i].Normal ;
		positionL.z = positionL.z * planetOneMinusOblateness;

		Position = vec3(ModelViewMatrix * vec4(positionL,1.0));
 		Light = normalize(LightPosition - Position);

		//Other
		Normal = normalize(mat3(NormalMatrix) * gs_in[i].Normal);
		//NdotL = dot(Normal, Light);
		vec3 binormal = vec3(0,-Normal.z,Normal.y);
		vec3 tangent = cross(Normal,binormal);
		vec3 TangentLight = vec3(dot(Light, tangent), dot(Light, binormal), dot(Light, Normal));

		gs_out.Position = Position;
		gs_out.Light = Light;
		gs_out.Normal = Normal;
		gs_out.TangentLight=TangentLight;
		//gs_out.NdotL = NdotL;
		gs_out.TexCoord = gs_in[i].TexCoord;

		EmitVertex();
	}

	EndPrimitive();
}
