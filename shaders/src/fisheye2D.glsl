#define M_PI 3.14159265358979323846

vec4 fisheye2D(in vec4 win, in float fov)
{
	vec4 pos = ModelViewMatrix * win;
	float tmp = sqrt(pos.x*pos.x + pos.y*pos.y);
	if (tmp < 1e-30)
		return vec4(0, 0, 0, 1);
	float f = (atan(pos.z, tmp) / M_PI + 0.5f) * 360./fov/tmp;
	return vec4(pos.x * f, pos.y * f, 0, 1);
}

