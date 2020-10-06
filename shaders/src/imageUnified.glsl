//
//	imageUnified
//
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;

layout (push_constant) uniform uFrag {
	layout (offset=76) float fader;
	layout (offset=80) vec4 noColor;
};

layout (location=0) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

