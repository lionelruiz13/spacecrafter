#define M_PI 3.14159265358979323846

vec4 fisheye2D(vec4 win, in float fov)
{
	win = ModelViewMatrix * win;
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(rq/depth);
	if (win.z > 0)
		f = M_PI - f;
	f /= rq * fov;
    return vec4(win.x * f, win.y * f, 0, 1.);
}
