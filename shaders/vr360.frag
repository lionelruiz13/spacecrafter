//vr360

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D s_texture_y;
layout (binding=1) uniform sampler2D s_texture_u;
layout (binding=2) uniform sampler2D s_texture_v;
uniform float intensity;

vec3 tex_color;
vec3 yuv_color;
smooth in vec2 TexCoord;
 
out vec4 FragColor;
 
void main(void)
{
    highp float y = texture2D(s_texture_y, TexCoord).r;  
    highp float u = texture2D(s_texture_u, TexCoord).r - 0.5;  
    highp float v = texture2D(s_texture_v, TexCoord).r - 0.5;  
    highp float r = y +             1.402 * v;  
    highp float g = y - 0.344 * u - 0.714 * v;  
    highp float b = y + 1.772 * u;  
    FragColor = vec4(r,g,b,1.0);  
}