#version 430 core
#pragma debug(on)
#pragma optimize(off)


layout(location=0) in vec3 position;
layout(location=1) in vec2 texcoord;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;
//layout(location=2) in mat3 TBN;

/*
in int gl_VertexID;
in int gl_InstanceID;*/
out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};


out VS_OUT{
    out vec2 uv;
    out vec3 normal;
    out vec3 tangent;
    //out vec3 bitangent;
    //out mat3 TBN;
}vs_out;

void main(void)
{
    gl_Position = vec4(position,1.0);

    vs_out.uv        = texcoord;
    vs_out.normal    = normal;
    //vs_out.normal    = TBN[2];
    vs_out.tangent   = tangent;
    //vs_out.tangent   = TBN[0];
    //vs_out.bitangent = bitangent;
    //vs_out.TBN     = TBN;
    //vs_out.TBN= mat3(tangent,bitangent,normal);
}
