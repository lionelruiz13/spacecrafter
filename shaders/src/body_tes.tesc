//
// my earth tessellation
//

#version 430 core
#pragma debug(on)
#pragma optimize(off)

layout(vertices=3) out;

//NEW UNIFORMS
layout (binding=2) uniform globalTescGeom {
	uniform ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};

layout(location=0) in vec3 PositionIn[];
layout(location=1) in vec2 TexCoordIn[];
layout(location=2) in vec3 NormalIn[];
layout(location=3) in vec2 glPositionIn[];
layout(location=4) in int VisibleIn[];

layout(location=0) out vec3 PositionOut[];
layout(location=1) out vec2 TexCoordOut[];
layout(location=2) out vec3 NormalOut[];
    //~ out vec3 tangent;

#define ID gl_InvocationID

void main(void)
{
    TexCoordOut[ID] = TexCoordIn[ID];
    NormalOut[ID] = NormalIn[ID];
    PositionOut[ID] = PositionIn[ID];
    if (ID == 0) {
        vec3 tes = vec3(distance(glPositionIn[1], glPositionIn[2]),
                        distance(glPositionIn[0], glPositionIn[2]),
                        distance(glPositionIn[0], glPositionIn[1]));
        tes = clamp(tes * 256 * float((VisibleIn[0] | VisibleIn[1] | VisibleIn[2]) & int(tes.x * tes.y * tes.z != 0)), 1., 64.);

        gl_TessLevelOuter[0] = tes[0];
        gl_TessLevelOuter[1] = tes[1];
        gl_TessLevelOuter[2] = tes[2];

        gl_TessLevelInner[0]=(tes[0] + tes[1] + tes[2])/3;
    }
}
