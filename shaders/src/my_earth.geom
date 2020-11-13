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

layout (binding=0) uniform globalProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

#include <cam_block.glsl>
#include <fisheye_noMV.glsl>

layout (binding=2) uniform globalTescGeom {
	ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};


float coeffHeightMap = 0.02 * float(TesParam[2]);

//out
//layout (location=0) out vec2 TexCoord;

layout(location=0) in vec3 glPositionIn[];
layout(location=1) in vec2 TexCoordIn[];
layout(location=2) in vec3 NormalIn[];
    //~ in vec3 tangent;
    //in vec3 tessCoord;

layout (location=0) out vec3 PositionOut;
layout (location=1) out vec2 TexCoordOut;
layout (location=2) out vec3 NormalOut;
layout (location=3) out vec3 TangentLightOut;
layout (location=4) out vec3 LightOut;
layout (location=5) out vec3 ViewDirectionOut;

void main()
{
	vec3 binormal, Normal, tangent;
	vec3 glPosition;
	vec3 positionL, Position, Light, ViewDirection, TangentLight;

	for(int i=0; i<3; i++) {
		glPosition= normalize(glPositionIn[i])*planetScaledRadius * (1.0+texture(heightmapTexture,TexCoordIn[i]).x * coeffHeightMap);

		gl_Position = fisheyeProject(glPosition, clipping_fov);

		positionL = planetRadius * glPositionIn[i];
		positionL.z = positionL.z * planetOneMinusOblateness;

		Position = vec3(ModelViewMatrix * vec4(positionL,1.0));
 		Light = normalize(LightPosition - Position);
		ViewDirection = normalize(-Position);

		//Other
		Normal = normalize(mat3(NormalMatrix) * NormalIn[i]);
		binormal = vec3(0,-Normal.z,Normal.y);
		tangent = cross(Normal,binormal);

		TangentLight = vec3(dot(Light, tangent), dot(Light, binormal), dot(Light, Normal)); 

		PositionOut = Position;
		LightOut = Light;
		ViewDirectionOut = ViewDirection;
		NormalOut = Normal;
		TangentLightOut = TangentLight;
		TexCoordOut = TexCoordIn[i];
		
		EmitVertex();
	}

	EndPrimitive();
}
