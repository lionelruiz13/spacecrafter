//////////////////// PROJECTION FISHEYE ////////////////////////////////
#define M_PI 3.14159265358979323846

// fisheyeProject without ModelView projection
vec4 fisheyeProjectNoMV(vec3 win, in vec3 clipping_fov)
{
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(min(rq/depth, 1)); // min patch a driver bug were rq/depth > 1
	if (win.z > 0)
		f = M_PI - f;
	f /= rq * clipping_fov.z;
    depth = (depth - clipping_fov.x) / (clipping_fov.y - clipping_fov.x);
    return vec4(win.x * f, win.y * f, depth, 1.);
}
