//
// moon bump
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#include <fisheye.glsl>

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
layout (location=2)in vec3 normal;

#include <cam_block.glsl>

//externe
uniform mat4 NormalMatrix;
uniform vec3 LightPosition;
uniform vec3 clipping_fov;

uniform float planetRadius;
uniform float planetScaledRadius;
uniform float planetOneMinusOblateness;


//out
smooth out vec2 TexCoord;
out float Ambient;

out vec3 Normal;
out vec3 Position;
out vec3 TangentLight;
out vec3 Light;

void main()
{
	//glPosition
	vec3 Position0;
	Position0.x =position.x * planetScaledRadius;
	Position0.y =position.y * planetScaledRadius;
	Position0.z =position.z * planetScaledRadius * planetOneMinusOblateness;
	gl_Position = fisheyeProject(Position0, clipping_fov);

    //Light
	vec3 positionL = planetRadius * position ;
	positionL.z = positionL.z * planetOneMinusOblateness;
	Position = vec3(ModelViewMatrix * vec4(positionL,1.0));  
	Light = normalize(LightPosition - Position);

	//Other
	Normal = normalize(mat3(NormalMatrix) * normal);
	vec3 binormal = vec3(0,-Normal.z,Normal.y);
	vec3 tangent = cross(Normal,binormal);
	TangentLight = vec3(dot(Light, tangent), dot(Light, binormal), dot(Light, Normal)); 
    TexCoord = texcoord;
    Ambient = ambient;
}
