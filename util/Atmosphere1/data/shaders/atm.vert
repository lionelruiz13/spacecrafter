#version 430 core
#pragma debug(on)
#pragma optimize(off)


layout(location=0) in vec3 position;
layout(location=1) in vec2 texcoord;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;
//layout(location=2) in mat3 TBN;

//NEW UNIFORMS
uniform mat4  model;

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
    out vec3 pos;
}vs_out;

void main(void)
{
    gl_Position = vec4(position,1.0);
    vs_out.pos  = (model*vec4(position,1.0)).xyz;
}
