//
//	body_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout(location=0) in vec2 pos;
layout(location=1) in vec4 Color;
layout(location=2) in float rmag;

//out
layout(location=0) out vec2 posOut;
layout(location=1) out vec4 ColorOut;
layout(location=2) out float rmagOut;

void main()
{
	posOut = pos;
	ColorOut = Color;
	rmagOut = rmag;
}

