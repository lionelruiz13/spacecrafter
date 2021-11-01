//
// strating splash
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//out
layout(location=0) out vec2 TexCoord;

const vec2 pos[4] = {{-1.0,1.0}, {1.0, 1.0}, {-1.0, -1.0}, {1.0, -1.0}};
const vec2 tex[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main()
{
	gl_Position = vec4(pos[gl_VertexIndex], 0, 1);
	TexCoord = tex[gl_VertexIndex];
}
