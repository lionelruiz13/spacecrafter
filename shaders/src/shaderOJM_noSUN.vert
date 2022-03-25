//
// OJM vertex with no SUN
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;
layout (location = 2) in vec3 VertexNormal;

#include <cam_block_only.glsl>

//externe
//uniform bool useTexture;
layout (binding=0, set=2) uniform custom {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
};

#include <fisheye.glsl>

//uniform vec3 clipping_fov;
//uniform mat4 ProjectionMatrix;
//uniform mat4 MVP;

//out
layout (location=0) out vec3 Position;
layout (location=1) out vec2 TexCoord;
layout (location=2) out vec3 Normal;

void main()
{
    TexCoord = VertexTexCoord;
    Normal = normalize( mat3(NormalMatrix) * VertexNormal);
    Position = vec3( ModelViewMatrix * vec4(VertexPosition,1.0) );

    gl_Position = fisheyeProject(VertexPosition, vec3(main_clipping_fov));
}
