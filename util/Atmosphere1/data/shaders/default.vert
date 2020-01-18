#version 430 core


layout(location=0) in vec3 position;
//layout(location=1) in vec2 texcoord;

layout (std140) uniform UBOData_t
{
    mat4  model;
    mat4  view;
    mat4  viewBeforeLookAt;
    mat4  projection;
    mat4  vp;
    mat4  mvp;
    mat4  normalMatrix;

    vec2  resolution;
    float time; // seconds... sometimes reset?
}UBOData;


out VS_OUT{
    out vec2 uv;
}vs_out;

void main(void)
{
    gl_Position = UBOData.vp*vec4(position,1.0);
    //vs_out.uv =   texcoord;
    vs_out.uv= vec2(0.0,0.0);
}
