//
// body normal
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#include <fisheyeDouble.glsl>

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
layout (location=2)in vec3 normal;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};

//externe
//uniform mat4 ModelViewProjectionMatrix;
//uniform mat4 ModelViewMatrixInverse;
uniform mat4 NormalMatrix;
uniform vec3 LightPosition;

//uniform mat4 ModelViewMatrix; // in fisheyeDouble

uniform float planetRadius;
uniform float planetScaledRadius;
uniform float planetOneMinusOblateness;


//out
smooth out vec2 TexCoord;
out float Ambient;

out vec3 Normal;
out vec3 Position;
out float NdotL;
out vec3 Light;
out vec3 ModelLight;

void main()
{
	//glPosition
	//~ gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
	vec3 Position0;
	Position0.x =position.x * planetScaledRadius;
	Position0.y =position.y * planetScaledRadius;
	Position0.z =position.z * planetScaledRadius * planetOneMinusOblateness;
	gl_Position = custom_project(vec4(Position0, 1.));

    //Light
	vec3 positionL = planetRadius * position ;
	positionL.z = positionL.z * planetOneMinusOblateness;
	Position = vec3(ModelViewMatrix * vec4(positionL,1.0));  

	Light = normalize(LightPosition - Position);

	//Other
	Normal = normalize(mat3(NormalMatrix) * normal);
	//~ ModelLight = vec3(ModelViewMatrixInverse * vec4(Light,1.0)); 
	NdotL = dot(Normal, Light);
	TexCoord = texcoord;
	Ambient = ambient;
}



//~ //old
//~ varying vec2 TexCoord; 
//~ uniform vec3 LightPosition;
//~ varying vec3 Normal; 
//~ varying vec3 Position; 
//~ varying float NdotL;
//~ varying vec3 Light;
//~ varying vec3 ModelLight;
//~ varying vec3 OriginalPosition; 

//~ void main() 
//~ {
	//~ OriginalPosition = vec3(gl_Color);
	//~ Position = vec3(gl_ModelViewMatrix * gl_Color); 
	//~ Normal = normalize(gl_NormalMatrix * gl_Normal); 
	//~ TexCoord = gl_MultiTexCoord0.st; 
	//~ Light = normalize(LightPosition - Position);
	//~ NdotL = dot(Normal, Light);
	//~ ModelLight = vec3(gl_ModelViewMatrixInverse * vec4(Light,1.0));
	//~ gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; 
//~ }
