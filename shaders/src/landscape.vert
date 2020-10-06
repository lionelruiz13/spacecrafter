//
// landscape
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 Position;

void main()
{
	Position = position;
    TexCoord = texcoord;
}
