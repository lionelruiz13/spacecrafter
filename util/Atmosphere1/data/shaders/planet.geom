#version 430 core
#pragma debug(on)
#pragma optimize(off)

//layout(invocations=1)in;
layout ( triangles ,invocations=1) in;
layout ( triangle_strip , max_vertices = 3) out;
//layout ( line_strip , max_vertices = 3) out;


in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];
//in int gl_PrimitiveIDIn;
//in int gl_InvocationID;  //Requires GLSL 4.0 or ARB_gpu_shader5

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};
//out int gl_PrimitiveID;

//layout(stream=0)
in TES_OUT
{
    in vec2 uv;
    in vec3 normal;
    in vec3 tangent;
    in vec3 lightRay;
    in vec3 tessCoord;
    //in mat3 TBN;
}gs_in[];

//layout(stream=0)
out GS_OUT
{
    out vec2 uv;
    out vec3 normal;
    out vec3 tangent;
    out vec3 lightRay;
    out vec3 colorMix;
    //out mat3 TBN; // tangent binormal normal matrix
}gs_out;

void main(void)
{

    //memoryBarrier();
    gl_Position     = gl_in[0].gl_Position/*+vec4(gs_in[0].normal,1.0)*/;
    gs_out.uv       = gs_in[0].uv;
    gs_out.normal   = gs_in[0].normal;
    //gs_out.tangent  = gs_in[0].tangent;
    gs_out.lightRay = gs_in[0].lightRay;
    //float xx=gs_in[0].tessCoord.x* gs_in[0].tessCoord.x;
    //gs_out.colorMix=vec3(1.0,xx,xx);
    gs_out.colorMix=vec3(1.0,0.0,0.0);
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();
    //EmitStreamVertex(0);

    gl_Position     = gl_in[1].gl_Position/*+vec4(gs_in[1].normal,1.0)*/;
    gs_out.uv       = gs_in[1].uv;
    gs_out.normal   = gs_in[1].normal;
    //gs_out.tangent  = gs_in[1].tangent;
    gs_out.lightRay = gs_in[1].lightRay;
    //float yy=gs_in[1].tessCoord.y* gs_in[1].tessCoord.y;
    //gs_out.colorMix=vec3(yy,1.0,yy);
    gs_out.colorMix=vec3(0.0,1.0,0.0);
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    gl_Position     = gl_in[2].gl_Position/*+vec4(gs_in[2].normal,1.0)*/;
    gs_out.uv       = gs_in[2].uv;
    gs_out.normal   = gs_in[2].normal;
    //gs_out.tangent  = gs_in[2].tangent;
    gs_out.lightRay = gs_in[2].lightRay;
    //float zz=gs_in[2].tessCoord.z* gs_in[2].tessCoord.z;
    //gs_out.colorMix=vec3(zz,zz,1.0);
    gs_out.colorMix=vec3(0.0,0.0,1.0);
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    EndPrimitive();
}











































