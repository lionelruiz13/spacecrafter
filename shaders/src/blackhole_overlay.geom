//
// black hole screen overlay
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

#include <cam_block.glsl>

layout(binding=0) uniform ubo {float Rmag;};

layout (location=0) in vec2 position[1];

layout (location=0) out vec2 Center;
layout (location=1) out vec2 TexCoord;

void main(void)
{
    vec2 center = position[0];

    gl_Position = MVP2D * vec4(center + vec2(-Rmag, -Rmag), 0, 1);
    Center = center;
    TexCoord = vec2(0, 0);
    EmitVertex();

    gl_Position = MVP2D * vec4(center + vec2(-Rmag, Rmag), 0, 1);
    Center = center;
    TexCoord = vec2(0, 1);
    EmitVertex();

    gl_Position = MVP2D * vec4(center + vec2(Rmag, -Rmag), 0, 1);
    Center = center;
    TexCoord = vec2(1, 0);
    EmitVertex();

    gl_Position = MVP2D * vec4(center + vec2(Rmag, Rmag), 0, 1);
    Center = center;
    TexCoord = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}
