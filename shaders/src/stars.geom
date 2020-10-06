//
//	STARS
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

//uniform mat4 ModelViewProjectionMatrix;

#include <cam_block.glsl>

layout (location=0) in float mag[];
layout (location=1) in vec3 color[];

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 TexColor;

//on veut représenter une texture sur un carré pour cela on construit deux triangles
void main(void)
{
    float mag1 = mag[0]/2;
    float mag2 = -mag1;
    //en bas à droite
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(mag1, mag2, 0.0, 0.0));
    TexCoord= vec2(1.0f, .0f);
    TexColor= color[0];
    EmitVertex();

    // en haut à droite
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(mag1, mag1, 0.0, 0.0));
    TexCoord= vec2(1.0f, 1.0f);
    TexColor= color[0];
    EmitVertex();    

    // en Bas à gauche
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(mag2, mag2, 0.0,0.0));
    TexCoord= vec2(0.0f, 0.0f);
    TexColor= color[0];
    EmitVertex();

    // en haut à gauche
    gl_Position   = MVP2D * (gl_in[0].gl_Position+vec4(mag2, mag1,0.0,0.0));
    TexCoord= vec2(0.0f, 1.0f);
    TexColor= color[0];
    EmitVertex();


    EndPrimitive();
}

 
