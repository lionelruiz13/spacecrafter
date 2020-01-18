//
//	sun_big_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0)in vec2 position;

//externe
//~ uniform mat4 ModelViewProjectionMatrix;
uniform float Rmag;

out vertexData
{
	vec2 center;
	float rmag;
} vertexOut;

void main()
{
	vertexOut.center = position;
	vertexOut.rmag = Rmag;
	gl_Position = vec4(position,0.0,1.0);
}





//~ //
//~ //	sun_big_halo
//~ //
//~ #version 420
//~ #pragma debug(on)
//~ #pragma optimize(off)

//~ //layout
//~ layout (location=0)in vec2 position;
//~ layout (location=1)in vec2 texcoord;

//~ //externe
//~ uniform sampler2D texunit0;
//~ uniform mat4 ModelViewProjectionMatrix;

//~ //out
//~ smooth out vec2 TexCoord;


//~ void main()
//~ {
	//~ gl_Position = ModelViewProjectionMatrix * vec4(position,0.0,1.0);
    //~ TexCoord = texcoord;
//~ }
