//
// body normal 
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#include <fisheye.glsl>

layout ( triangles ) in;
layout ( triangle_strip , max_vertices = 3) out;

in vertexData
{
	vec2 texcoord;
	vec3 normal;
	vec3 position;
} vertexVtG[];

out colorFrag
{
    vec2 TexCoord;
    vec3 Position;
    float NdotL;
    vec3 Light; 
} cfOut;


//externe
uniform mat4 NormalMatrix;
uniform vec3 LightPosition;
uniform vec3 clipping_fov;

uniform float planetRadius;
uniform float planetScaledRadius;
uniform float planetOneMinusOblateness;

void main()
{
    for(int i=0; i<3; i++) {
    	//glPosition
    	vec3 Position0;
    	Position0 = vertexVtG[i].position * planetScaledRadius;
    	Position0.z *= planetOneMinusOblateness;
    	gl_Position = fisheyeProject(Position0, clipping_fov);

        //Light
    	vec3 positionL = planetRadius * vertexVtG[i].position;
    	positionL.z = positionL.z * planetOneMinusOblateness;
    	cfOut.Position = vec3(ModelViewMatrix * vec4(positionL,1.0));  
    	cfOut.Light = normalize(LightPosition - cfOut.Position);

    	//Other
    	vec3 normal = normalize(mat3(NormalMatrix) * vertexVtG[i].normal);
    	cfOut.NdotL = dot(normal, cfOut.Light);
    	cfOut.TexCoord = vertexVtG[i].texcoord;
    	EmitVertex();
    }
	EndPrimitive();
}
