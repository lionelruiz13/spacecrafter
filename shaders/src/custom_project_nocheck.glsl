#define M_PI 3.14159265358979323846

vec4 custom_project(vec4 win)
{
	float zNear=main_clipping_fov[0];
	float zFar=main_clipping_fov[1];
	float fov=main_clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0;
	float viewport_center_x=viewport_center[0];
	float viewport_center_y=viewport_center[1];
	float viewport_radius=viewport_center[2];

    win = Mat * win;

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		win.x = viewport_center_x;
		win.y = viewport_center_y;
		win.z = (win.z < 0.0) ? 1.0 : -1e30;
		return win;
	}
	float depth = sqrt(rq1 + win.z*win.z);

        float oneoverh = 1.0/sqrt(rq1);
        float a = M_PI/2.0 + atan(win.z*oneoverh);
        float f = a * fisheye_scale_factor;

        f *= viewport_radius * oneoverh;

        return vec4(viewport_center_x + win.x * f, viewport_center_y + win.y * f, (depth - zNear) / (zFar-zNear), 1.0);
}

vec4 custom_project(vec3 invec)
{
	return custom_project(vec4(invec, 1.));
}
