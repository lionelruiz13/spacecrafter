//skygrid

#version 420
#pragma debug(on)
#pragma optimize(off)

#define M_PI   3.14159265358979323846

layout (lines) in;
layout (line_strip , max_vertices = 2) out;

uniform mat4 Mat;

#define M_PI   3.14159265358979323846

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};


vec4 custom_project(vec4 invec)
{
	float zNear=main_clipping_fov[0];
	float zFar=main_clipping_fov[1];
	float fov=main_clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0;
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

in ColorVertex
{
	float intensityColor;
} cvIn[];


out colorFrag
{
	float intensityColor;
} cfOut;

void main(void)
{
	vec4 pos0, pos1;

    pos0 = custom_project(gl_in[0].gl_Position);
    pos1 = custom_project(gl_in[1].gl_Position);
    
	if ( pos0.w==1.0 && pos1.w==1.0 ) {
		pos0.z = 0.0;
		pos1.z = 0.0;
		gl_Position = MVP2D * pos0;
		cfOut.intensityColor = cvIn[0].intensityColor;
		EmitVertex();

		cfOut.intensityColor = cvIn[1].intensityColor;
		gl_Position = MVP2D * pos1;
		EmitVertex();
	}

    EndPrimitive();
}
