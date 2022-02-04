#define M_PI 3.14159265358979323846

vec4 fisheye2D(in vec4 win, in float fov)
{
	vec4 pos = ModelViewMatrix * win;
	pos /= sqrt(pos.x*pos.x + pos.y*pos.y) + 1e-30; // Don't divide by zero
	float f = (atan(pos.z) / M_PI + 0.5f) * 360./fov;
	return vec4(pos.x * f, pos.y * f, 0, 1);
}

