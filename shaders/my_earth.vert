//
// my earth tessellation
//
#version 430
#pragma debug(on)
#pragma optimize(off)

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

uniform float planetRadius;
uniform float planetScaledRadius;
uniform float planetOneMinusOblateness;

out VS_OUT{
	vec4 glPosition;
    vec2 TexCoord;
    vec3 Normal;
} vs_out;


void main()
{
	vec3 Position;
	Position.x =position.x * planetScaledRadius;
	Position.y =position.y * planetScaledRadius;
	Position.z =position.z * planetScaledRadius * planetOneMinusOblateness;

	vs_out.glPosition = vec4(Position, 1.0);
	vs_out.TexCoord = texcoord;
	vs_out.Normal = normal;
}

