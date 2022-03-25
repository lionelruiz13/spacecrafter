//
//	STARS
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

//uniform mat4 ModelViewProjectionMatrix;

layout (set = 0, binding = 1) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
	bool allsphere;
};

layout (location=0) in float mag[];
layout (location=1) in vec3 color[];
layout (location=2) in vec2 position[];

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 TexColor;

//on veut représenter une texture sur un carré pour cela on construit deux triangles
void main(void)
{
    float mag1 = mag[0]/2;
    float mag2 = -mag1;
	vec2 pos = position[0];
    //en bas à droite
    gl_Position   = MVP2D * vec4(pos+vec2(mag1, mag2), 0.0, 1.0);
    TexCoord= vec2(1.0f, .0f);
    TexColor= color[0];
    EmitVertex();

    // en haut à droite
    gl_Position   = MVP2D * vec4(pos+vec2(mag1, mag1), 0.0, 1.0);
    TexCoord= vec2(1.0f, 1.0f);
    TexColor= color[0];
    EmitVertex();

    // en Bas à gauche
    gl_Position   = MVP2D * vec4(pos+vec2(mag2, mag2), 0.0,1.0);
    TexCoord= vec2(0.0f, 0.0f);
    TexColor= color[0];
    EmitVertex();

    // en haut à gauche
    gl_Position   = MVP2D * vec4(pos+vec2(mag2, mag1),0.0,1.0);
    TexCoord= vec2(0.0f, 1.0f);
    TexColor= color[0];
    EmitVertex();


    EndPrimitive();
}
