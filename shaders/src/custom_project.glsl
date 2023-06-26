#define M_PI 3.14159265358979323846

vec4 custom_project(vec4 invec)
{
	float zNear=main_clipping_fov[0];
	float zFar=main_clipping_fov[1];
	float fov=main_clipping_fov[2];

	vec4 win = invec;
    win = Mat * win;

    float rq1 = win.x*win.x+win.y*win.y;
	float depth = sqrt(rq1 + win.z*win.z);

	if (rq1 > 0) {
	        rq1 = sqrt(rq1);
			float f = asin(min(rq1/depth, 1)); // min patch a driver bug were rq/depth > 1
			if (win.z > 0)
				f = M_PI - f;
			win.w = mix(-1.0, 1.0, f<0.9*M_PI);
	        f /= fov * rq1;

			//	Realtime ALLSPHERE distorsion
			//	f = f *1200.f;
			//	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
			//	f = f/1200.f;
			f *= viewport_center[2];

			win.x = win.x * f + viewport_center[0];
			win.y = win.y * f + viewport_center[1];
	} else {
		win.x = viewport_center[0];
		win.y = viewport_center[1];
		win.w = mix(-1.0, 1.0, win.z < 0);
	}
	win.z = (abs(depth) - zNear) / (zFar-zNear);
	if (win.z == 0.0)
		win.z = -1e30;
	return win;
}

vec4 custom_project(vec3 invec)
{
	return custom_project(vec4(invec, 1.));
}
