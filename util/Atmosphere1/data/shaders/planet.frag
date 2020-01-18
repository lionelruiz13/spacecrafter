#version 430 core
#pragma debug(on)
#pragma optimize(off)

out vec4 color;

uniform sampler2D diffuseMap;
uniform sampler2D heightMap;
uniform sampler2D normalMap;


/*
//in vec4 gl_FragCoord;
//in bool gl_FrontFacing;
in vec2 gl_PointCoord;
layout(origin_upper_left) in vec4 gl_FragCoord;
//layout(pixel_center_integer​) in vec4 gl_FragCoord;

//in int gl_SampleID;
//in vec2 gl_SamplePosition;
//in int gl_SampleMaskIn[];

//in float gl_ClipDistance[];
//in int gl_PrimitiveID;

//in int gl_Layer;
//in int gl_ViewportIndex;


out float gl_FragDepth;

out int gl_SampleMask[];
*/

 in GS_OUT{
    in vec2 uv;
    in vec3 normal;
    in vec3 tangent;
    in vec3 lightRay;
    in vec3 colorMix;
    //in mat3 TBN;
}fs_in;



void main(void)
{
	vec4 fcolor = texture(diffuseMap,fs_in.uv);
	vec3 normal = texture(normalMap,fs_in.uv).rgb*2.0-vec3(1.0);

	fcolor.xyz = fcolor.xyz*dot(normal,fs_in.lightRay);

	color=fcolor;
	//color = vec4(texture2D(normalMap,fs_in.uv).rgb,1.0);
}























