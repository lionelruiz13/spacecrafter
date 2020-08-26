//
// OJM vertex with no SUN
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#include <fisheye.glsl>

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;
layout (location = 2) in vec3 VertexNormal;

#include <cam_block.glsl>

//externe
uniform bool useTexture;
uniform mat4 NormalMatrix;
uniform vec3 clipping_fov;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

//out
out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    if (useTexture)
		TexCoord = VertexTexCoord;

    Normal = normalize( mat3(NormalMatrix) * VertexNormal);
    Position = vec3( ModelViewMatrix * vec4(VertexPosition,1.0) );

    gl_Position = fisheyeProject(VertexPosition, clipping_fov);
}
