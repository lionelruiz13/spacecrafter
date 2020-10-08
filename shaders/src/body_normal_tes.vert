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
/*
layout (location=0) out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};
*/

    //~ vec3 PositionL;
layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 Normal;


void main()
{
	vec3 Position;
	Position.x =position.x * planetScaledRadius;
	Position.y =position.y * planetScaledRadius;
	Position.z =position.z * planetScaledRadius * planetOneMinusOblateness;
	gl_Position = vec4(Position, 1.0);

	TexCoord = texcoord;
	Normal = normal;
}
