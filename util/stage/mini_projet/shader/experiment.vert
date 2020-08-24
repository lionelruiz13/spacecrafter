#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texel;

layout(location = 0) out vec3 Color;

layout (binding = 0) uniform UniformBufferObject {
	mat4 MV;
	vec3 clipping_fov;
} ubo;

#define M_PI 3.14159265358979323846

vec4 custom_project(vec4 invec)
{
	float zNear= ubo.clipping_fov[0];
	float zFar= ubo.clipping_fov[1];
	float fov = ubo.clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0;

    vec4 win = invec;
    win=ubo.MV*win;

    float rq1 = win.x*win.x+win.y*win.y;

    float depth = rq1 + win.z * win.z;

	if (rq1 <= 0.0 ) {
		if (win.z < 0.0) {
			win.x = 0.;
			win.y = 0.;
			win.z = 1.0;
			win.w=0.0;
			return win;
		}
		win.x = 0.;
		win.y = 0.;
		win.z = -1e30;
		win.w = 0.0;
		return win;
	}
	else{
        float oneoverh = 1.0/sqrt(rq1);
        float a = M_PI/2.0 + atan(win.z*oneoverh);
        float f = a * fisheye_scale_factor;

        f *= oneoverh;

        win.x = win.x * f;
        win.y = win.y * f;

        win.z = (abs(depth) - zNear) / (zFar-zNear);

		win.w = (a<0.9*M_PI) ? 1. : 0.;
        return win;
	}
}

vec4 custom_unproject(vec4 pos)
{
	vec4 unproj_vec=vec4(pos.x,
						-pos.y,
						  1.0-2.0*pos.z,
						  pos.w);

	return unproj_vec;
}

void main()
{
	gl_Position = custom_unproject(custom_project(vec4(vertexPos, 1.0)));
	Color = color;
}
