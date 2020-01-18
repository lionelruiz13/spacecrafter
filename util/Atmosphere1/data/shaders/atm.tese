#version 430 core
#pragma debug(on)
#pragma optimize(off)

//layout (quads, equal_spacing) in;
layout (triangles, equal_spacing) in;
//layout (isolines, equal_spacing) in;
//layout (triangles, fractional_odd_spacing)in;

//uniform sampler2D texture;
//uniform sampler2D heightMap;
//uniform sampler2D normalMap;

//NEW UNIFORMS
uniform mat4 model;
uniform mat4 vp;

/*
in vec3 gl_TessCoord;
//in int gl_PatchVerticesIn;
in int gl_PrimitiveID;
patch in float gl_TessLevelOuter[4];
patch in float gl_TessLevelInner[2];*/
in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

// we'll change this with a uniform later.
/*const*/ vec3 lightPosition=vec3(0.0,0.0,0.0);

in TCS_OUT{
    in vec3 pos;
}tes_in[];

out TES_OUT{
    out vec3 pos;
}tes_out;


void main(void)
{

    tes_out.pos =   vec3(   vec4(tes_in[0].pos,0.0)*gl_TessCoord.x+
                            vec4(tes_in[1].pos,0.0)*gl_TessCoord.y+
                            vec4(tes_in[2].pos,0.0)*gl_TessCoord.z);



 // first we compute the position
    vec4 position=(gl_TessCoord.x * gl_in[0].gl_Position)+
                  (gl_TessCoord.y * gl_in[1].gl_Position)+
                  (gl_TessCoord.z * gl_in[2].gl_Position);
                  position.w=1.0;



    // then we use the heightmap to get the r
    // himalaya height: 8800 m.
    // earth ray      : 6356752 m.
    //position.xyz= position.xyz/length(position.xyz) *(1.0+texture2D(heightMap,uvCoordinates).x *(8800.0/(6356752.0+8800.0)));
    //position.xyz= position.xyz/length(position.xyz) *(1.0+texture(heightMap,uvCoordinates).x *0.055);
    position.xyz= position.xyz/length(position.xyz);


    // get the space coordinates for light calculations
    position=model*position;

    gl_Position = vp * position;
}









































