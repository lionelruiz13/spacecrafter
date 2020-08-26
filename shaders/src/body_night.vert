//
// body night
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

out vec3 Normal;
out vec3 Position;
out vec3 TangentLight;
out vec3 Light;
out vec3 ViewDirection;


void main()
{
	//glPosition
	//~ gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
	vec3 Position0;
	Position0.x =position.x * planetScaledRadius;
	Position0.y =position.y * planetScaledRadius;
	Position0.z =position.z * planetScaledRadius * planetOneMinusOblateness;
	gl_Position = fisheyeProject(Position0, clipping_fov);

    //Light
	vec3 positionL = planetRadius * normal ;
	positionL.z = positionL.z * planetOneMinusOblateness;
	Position = vec3(ModelViewMatrix * vec4(positionL,1.0));  
	Light = normalize(LightPosition - Position);
	ViewDirection = normalize(-Position);
	
	//Other
	Normal = normalize(mat3(NormalMatrix) * normal);
	vec3 binormal = vec3(0,-Normal.z,Normal.y);
	vec3 tangent = cross(Normal,binormal);
	TangentLight = vec3(dot(Light, tangent), dot(Light, binormal), dot(Light, Normal)); 
    TexCoord = texcoord;
}


//~ uniform vec3 LightPosition;
//~ varying vec2 TexCoord; 
//~ varying vec3 TangentLight;
//~ varying vec3 Normal; 
//~ varying vec3 Position; 
//~ varying vec3 Light;
//~ varying vec3 ViewDirection;
//~ void main() 
//~ { 
	//~ Position = vec3(gl_ModelViewMatrix * gl_Color); 
	//~ Normal = normalize(gl_NormalMatrix * gl_Normal); 
	//~ TexCoord = gl_MultiTexCoord0.st; 
	//~ Light = normalize(LightPosition - Position);
	//~ ViewDirection = normalize(-Position);
	//~ vec3 binormal = vec3(0,-Normal.z,Normal.y);
	//~ vec3 tangent = cross(Normal,binormal);
	//~ TangentLight = vec3(dot(Light, tangent), dot(Light, binormal), dot(Light, Normal)); 
	//~ gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; 
//~ }
