//////////////////// PROJECTION FISHEYE ////////////////////////////////
#define M_PI 3.14159265358979323846

// note: win.w != 1 tell us that point is behind us
// note: win.w == 1 tell us that point is front us
vec4 fisheyeProject(vec3 invec, vec3 clipping_fov)
{
    vec4 win = ModelViewMatrix * vec4(invec, 1);
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

vec4 fisheyeProjectClamped(vec3 invec, vec3 clipping_fov)
{
	vec4 win = ModelViewMatrix * vec4(invec, 1);
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

    float f = asin(min(rq/depth, 1)); // min patch a driver bug were rq/depth > 1
	if (win.z > 0)
		f = M_PI - f;
	f /= rq * clipping_fov.z;
    depth = (depth - clipping_fov.x) / (clipping_fov.y - clipping_fov.x);
    return vec4(win.x * f, win.y * f, clamp(depth, 0, 1), 1.);
}
