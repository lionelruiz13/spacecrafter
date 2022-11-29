#define M_PI 3.14159265358979323846

vec4 custom_project(vec4 invec)
{
	float zNear=main_clipping_fov[0];
	float zFar=main_clipping_fov[1];
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
		win.x = viewport_center_x;
		win.y = viewport_center_y;
		win.z = (win.z < 0.0) ? 1.0 : -1e30;
		win.w =-1.0;
		return win;
	}
        float oneoverh = 1.0/sqrt(rq1);
        float a = M_PI/2.0 + atan(win.z*oneoverh);
        float f = a * fisheye_scale_factor;

        f *= oneoverh;
		//	Realtime ALLSPHERE distorsion
		//	f = f *1200.f;
		//	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
		//	f = f/1200.f;
		f *= viewport_radius;

        win.x = viewport_center_x + win.x * f;
        win.y = viewport_center_y + win.y * f;

        win.z = (abs(depth) - zNear) / (zFar-zNear);
        if (a<0.9*M_PI)
			win.w = 1.0;
        else
			win.w = -1.0;
	if (win.z == 0.0)
		win.z = -1e30;
        return win;
}

vec4 custom_project(vec3 invec)
{
	return custom_project(vec4(invec, 1.));
}
