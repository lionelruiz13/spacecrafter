//
// body normal tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#define M_PI 3.14159265358979323846

layout ( triangles ) in;
layout ( triangle_strip , max_vertices = 3) out;

layout (binding=5) uniform sampler2D heightmapTexture;

#include <cam_block.glsl>

//externe
layout (binding=0) uniform globalProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

#include <fisheye_noMV.glsl>

layout (binding=2) uniform globalTescGeom {
	ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};
/*
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
*/

//out
//smooth out vec2 TexCoord;
//~ out vec3 Normal;
//~ out vec3 Position;
//~ out vec3 TangentLight;
//~ out vec3 Light;
//~ out vec3 ViewDirection;

layout(location=0) in vec3 glPositionIn[];
layout(location=1) in vec2 TexCoordIn[];
layout(location=2) in vec3 NormalIn[];
    //~ in vec3 tangent;
    //in vec3 tessCoord;

//~ in VS_OUT{
    //~ //vec3 PositionL;
    //~ vec2 TexCoord;
    //~ vec3 Normal;
//~ } vs_in[];

layout (location=0) out vec3 PositionOut;
layout (location=1) out vec2 TexCoordOut;
layout (location=2) out vec3 NormalOut;
layout (location=3) out vec3 LightOut;
layout (location=4) out float NdotLOut;
layout (location=5) out float Ambient;

float coeffHeightMap = 0.05 * TesParam[2];

//////////////////// PROJECTION FISHEYE ////////////////////////////////
void main()
{
	vec3 glPosition;
	vec3 positionL, Position, Light, Normal;
	float NdotL;

	for(int i=0; i<3; i++) {

		glPosition = glPositionIn[i];
		positionL = planetRadius * glPosition;
		//~ sans normalMap
		//~ glPosition.xyz= glPosition.xyz/length(glPosition.xyz)*planetScaledRadius ;
		//~ avec normalMap
		glPosition = normalize(glPosition)*planetScaledRadius * (1.0+texture(heightmapTexture,TexCoordIn[i]).x * coeffHeightMap);

		gl_Position = fisheyeProject(glPosition, clipping_fov);

		positionL.z = positionL.z * planetOneMinusOblateness;

		//positionL = planetRadius * NormalIn[i];
		Position = vec3(ModelViewMatrix * vec4(positionL,1.0));
 		Light = normalize(LightPosition - Position);

		//Other
		Normal = normalize(mat3(NormalMatrix) * NormalIn[i]);
		NdotL = dot(Normal, Light);

		PositionOut = Position;
		LightOut = Light;
		NormalOut = Normal;
		NdotLOut = NdotL;
		TexCoordOut = TexCoordIn[i];
		Ambient = ambient;

		EmitVertex();
	}

	EndPrimitive();
}
