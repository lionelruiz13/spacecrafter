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
    in vec3 pos;
}gs_in[];

//layout(stream=0)
out GS_OUT
{
    out vec3 pos;
}gs_out;

void main(void)
{

    //memoryBarrier();
    gl_Position     = gl_in[0].gl_Position;
    gs_out.pos      = gs_in[0].pos;
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();
    //EmitStreamVertex(0);

    gl_Position     = gl_in[1].gl_Position;
    gs_out.pos      = gs_in[1].pos;
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    gl_Position     = gl_in[2].gl_Position;
    gs_out.pos      = gs_in[2].pos;
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    EndPrimitive();
}











































