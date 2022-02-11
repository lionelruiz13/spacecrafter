//
// Volumetric 3D object
//
#version 420

layout(vertices=3) out;

#define ID gl_InvocationID

layout (location=0) in vec3 pos[];
layout (location=1) in vec3 tex[];

layout (location=0) out vec3 posOut[];
layout (location=1) out vec3 texOut[];

void main()
{
    if (ID == 0) {
        vec2 v0 = gl_in[0].gl_Position.xy;
        vec2 v1 = gl_in[1].gl_Position.xy;
        vec2 v2 = gl_in[2].gl_Position.xy;
	float d0 = distance(v1, v2);
        v1 -= v0;
        v2 -= v0;
        vec3 tes = vec3(d0, length(v2), length(v1));
        float counter_clockwise = float(v1.x * v2.y < v1.y * v2.x);
        float texCoef = clamp((tes[0] + tes[1] + tes[2])/3 * 64., 1., 64.) * counter_clockwise;

        gl_TessLevelOuter[0] = texCoef;
        gl_TessLevelOuter[1] = texCoef;
        gl_TessLevelOuter[2] = texCoef;

        gl_TessLevelInner[0]= texCoef;
    }
    texOut[ID] = tex[ID];
    posOut[ID] = pos[ID];
}

