//
// body artificial
//
#version 420
#pragma debug(on)
#pragma optimize(off)


//layout
layout ( triangles ) in;
layout ( triangle_strip , max_vertices = 3) out;

//#include <cam_block.glsl>

//externe
//uniform bool useTexture;
layout (binding=1, set=2) uniform artGeom {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
};

#include <fisheye.glsl>

//in-out
layout (location=0) in vec3 Position[3];
layout (location=1) in vec3 Normal[3];
layout (location=2) in vec2 TexCoord[3];
layout (location=3) in float Ambient[3];

layout (location=0) out vec3 cfOutPosition;
layout (location=1) out vec2 cfOutTexCoord;
layout (location=2) out vec3 cfOutNormal;
layout (location=3) out float cfOutAmbient;

#define TOLERANCE 0.6

void main(void)
{
    vec4 pos[3];

    for(int i=0; i<3; i++)
        pos[i] = fisheyeProject(Position[i], clipping_fov);

    vec2 pos0 = pos[0].xy;
    if (clipping_fov[2] < 300.0 || (length(pos0 - pos[1].xy) + length(pos0 - pos[2].xy)) < TOLERANCE) {
        for(int i=0; i<3; i++) {
            cfOutPosition = Position[i];
            cfOutTexCoord = TexCoord[i];
            cfOutNormal = Normal[i];
            cfOutAmbient = Ambient[i];
            gl_Position = pos[i];
            EmitVertex();
        }
    }
    EndPrimitive();
}
