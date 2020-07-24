//
// landscape2T
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;

out V2G
{
	vec3 Position;
	vec2 TexCoord;
} v2g;

void main()
{
	v2g.Position = position;
    v2g.TexCoord = texcoord;
}
