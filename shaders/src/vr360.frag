//vr360

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D s_tex_y;
layout (binding=1, set=1) uniform sampler2D s_tex_u;
layout (binding=2, set=1) uniform sampler2D s_tex_v;

#include <convertToRGB.glsl>

layout (location=0) in vec2 TexCoord;

layout (location=0)out vec4 FragColor;
 
void main(void)
{
    vec3 tex_color = convertToRGB(s_tex_y, s_tex_u, s_tex_v, TexCoord);
    FragColor = vec4(tex_color,1);  
}
