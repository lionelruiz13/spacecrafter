//
// starNav
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#define M_PI 3.14159265358979323846

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

layout (location=0) in float mag[1];
layout (location=1) in vec3 color[1];

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 TexColor;

layout (binding=1, set=1) uniform uMat {
	mat4 Mat;
};

#include <cam_block_only.glsl>

vec4 custom_project(vec4 invec)
{
	float zNear=main_clipping_fov[0]; // 1.f
	float zFar=main_clipping_fov[1] * 8.f; // 16000.f
	float fov=main_clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov;
	float viewport_center_x=viewport_center[0];
	float viewport_center_y=viewport_center[1];
	float viewport_radius=viewport_center[2];

	vec4 win = invec;
    win = Mat * win;
    win.w = 0.0;

	float depth = length(win);

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		if (win.z < 0.0) {
			win.x = viewport_center_x;
			win.y = viewport_center_y;
			win.z = 1.0;
			win.w =-1.0;
			return win;
		}
		win.x = viewport_center_x;
		win.y = viewport_center_y;
		win.z = -1e30;
		win.w = -1.0;
		return win;
	}
	else{
        float oneoverh = 1.0/sqrt(rq1);
        float a = M_PI/2.0 + atan(win.z*oneoverh);
        float f = a * fisheye_scale_factor;

        f *= viewport_radius * oneoverh;

        win.x = viewport_center_x + win.x * f;
        win.y = viewport_center_y + win.y * f;

        win.z = (abs(depth) - zNear) / (zFar-zNear);
        if (a<0.9*M_PI)
			win.w = 1.0;
        else
			win.w = -1.0;
        return win;
	}
}

void main()
{
	//test sur le centre afin d'écarter les stars invisibles ou hors caméra
	vec4 pos = custom_project( gl_in[0].gl_Position );
	if (pos.w == 1.0) {
		// en Bas à droite
		gl_Position   = MVP2D * ( pos +vec4( mag[0], -mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(1.0f, .0f);
		TexColor= color[0];
		EmitVertex();

		// en haut à droite
		gl_Position   = MVP2D * ( pos +vec4( mag[0], mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(1.0f, 1.0f);
		TexColor= color[0];
		EmitVertex();

		// en Bas à gauche
		gl_Position   = MVP2D * ( pos +vec4( -mag[0], -mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(0.0f, 0.0f);
		TexColor= color[0];
		EmitVertex();

		// en haut à gauche
		gl_Position   = MVP2D * ( pos +vec4( -mag[0], mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(0.0f, 1.0f);
		TexColor= color[0];
		EmitVertex();
	}
}
