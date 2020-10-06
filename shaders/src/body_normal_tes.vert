//
// body normal tessellation
//
#version 430
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
layout (location=2)in vec3 normal;

//uniform float planetRadius;
layout (binding=2) uniform globalVertGeom {
    float planetScaledRadius;
    float planetOneMinusOblateness;
};

layout (location=0) out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

layout (location=1) out VS_OUT{
    //~ vec3 PositionL;
    vec2 TexCoord;
    vec3 Normal;
} vs_out;


void main()
{
	vec3 Position;
	Position.x =position.x * planetScaledRadius;
	Position.y =position.y * planetScaledRadius;
	Position.z =position.z * planetScaledRadius * planetOneMinusOblateness;
	gl_Position = vec4(Position, 1.0);

	vs_out.TexCoord = texcoord;
	vs_out.Normal = normal;
}
