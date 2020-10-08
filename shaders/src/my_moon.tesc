//
// my moon tessellation
//

#version 430 core
#pragma debug(on)
#pragma optimize(off)

layout(vertices=3) out;

//NEW UNIFORMS
layout (binding=4) uniform globalTesc {
	mat4 ViewProjection;
	mat4 Model;
};
layout (binding=3) uniform globalTescGeom {
	ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};

/*
in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];


out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_out[];
*/

layout (location=0) in vec2 TexCoordIn[gl_MaxPatchVertices];
layout (location=1) in vec3 NormalIn[gl_MaxPatchVertices];
    //~ in vec3 tangent;

layout (location=0) out vec2 TexCoord[3];
layout (location=1) out vec3 Normal[3];
    //~ out vec3 tangent;

#define ID gl_InvocationID

int MinTessLevel=TesParam[0];
int MaxTessLevel=TesParam[1];
const float MaxDepth=8.0;
const float MinDepth=1.0;

void main(void)
{
    // Position in camera coordinates
    vec4 p = ViewProjection*Model[3].xyzw;

    // 0.0=close 1.0=far
    float ratio=(abs(p.z-1.0)-MinDepth)/(MaxDepth-MinDepth);


    float depth = clamp (ratio,0.0,1.0);
    depth=1.0-depth;
    depth=depth*depth*depth*depth*depth*depth*depth*depth;
    depth=1.0-depth;

    float tessLevel= mix(MaxTessLevel,MinTessLevel,depth);

    if(ID==0)
    {
        gl_TessLevelInner[0]=float(tessLevel);

        gl_TessLevelOuter[0]=float(tessLevel);
        gl_TessLevelOuter[1]=float(tessLevel);
        gl_TessLevelOuter[2]=float(tessLevel);
    }


    TexCoord[ID] = TexCoordIn[ID];
    Normal[ID] = NormalIn[ID];
    //~ tcs_out[ID].tangent = tangentIn[ID];

    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
}
