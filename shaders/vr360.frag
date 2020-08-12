//vr360

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D s_tex_y;
layout (binding=1) uniform sampler2D s_tex_u;
layout (binding=2) uniform sampler2D s_tex_v;

#include <convertToRGB.glsl>

smooth in vec2 TexCoord;

out vec4 FragColor;
 
void main(void)
{
    vec3 tex_color = convertToRGB(s_tex_y, s_tex_u, s_tex_v, TexCoord);
    FragColor = vec4(tex_color,1);  
}
