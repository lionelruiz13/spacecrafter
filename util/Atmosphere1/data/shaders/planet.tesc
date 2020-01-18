#version 430 core
#pragma debug(on)
#pragma optimize(off)

//layout(vertices=4)out;
layout(vertices=3) out;
/*
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;*/

//NEW UNIFORMS
uniform mat4 vp;
uniform mat4 model;

//in int gl_PatchVerticesIn;
//in int gl_PrimitiveID;

/*in int gl_InvocationID;*/
in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];

/*
patch out float gl_TessLevelOuter[4];
patch out float gl_TessLevelInner[2];*/
out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_out[];


in VS_OUT{
    in vec2 uv;
    in vec3 normal;
    in vec3 tangent;
    //in vec3 bitangent;
    //in mat3 TBN;
}tcs_in[];

out TCS_OUT{
    out vec2 uv;
    out vec3 normal;
    out vec3 tangent;
    //out mat3 TBN;
}tcs_out[];

#define ID gl_InvocationID

const int   MinTessLevel=1;
const int   MaxTessLevel=16;
const float MaxDepth=8.0;
//const float MaxDepth=20.0;
const float MinDepth=1.0;

void main(void)
{

    // Position in camera coordinates
    //vec4 p= projection*view*model*gl_in[ID].gl_Position;
    vec4 p = vp*model[3].xyzw;

    // 0.0=close 1.0=far
    float ratio=(abs(p.z-1.0)-MinDepth)/(MaxDepth-MinDepth);


    float depth = clamp (ratio,0.0,1.0);
    depth=1.0-depth;
    depth=depth*depth*depth*depth*depth*depth*depth*depth;

    depth=1.0-depth;

    float tessLevel= mix(MaxTessLevel,MinTessLevel,depth);
    //float tessLevel=1.0;

    if(ID==0)
    {
        gl_TessLevelInner[0]=float(tessLevel);
        //gl_TessLevelInner[0]=32;
        //gl_TessLevelInner[1]=2.0;

        gl_TessLevelOuter[0]=float(tessLevel);
        gl_TessLevelOuter[1]=float(tessLevel);
        gl_TessLevelOuter[2]=float(tessLevel);
        //gl_TessLevelOuter[3]=2.0;
    }


    tcs_out[ID].uv      = tcs_in[ID].uv;
    tcs_out[ID].normal  = tcs_in[ID].normal;
    tcs_out[ID].tangent = tcs_in[ID].tangent;
    //tcs_out[ID].TBN     = tcs_in[ID].TBN;
    //tes_out[ID].uv= vec2(0.5,0.5);

    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
    //memoryBarrier();
}
