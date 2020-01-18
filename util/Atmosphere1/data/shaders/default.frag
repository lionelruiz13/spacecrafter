#version 430 core

out vec4 color;

uniform sampler2D texunit0;


 in VS_OUT{
    in vec2 uv;
}frag_in;



void main(void)
{
	vec4 fcolor = texture2D(texunit0,frag_in.uv).rgba;
	color = fcolor;
}

