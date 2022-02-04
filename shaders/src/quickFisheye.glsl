#define M_PI 3.14159265358979323846

vec4 quickFisheye(in vec4 win, in vec3 clipping_fov)
{
	vec4 pos = ModelViewMatrix * win;
	float rq1 = pos.x*pos.x + pos.y*pos.y;
	float depth = (sqrt(rq1 + pos.z*pos.z) - clipping_fov[0]) / (clipping_fov[1] - clipping_fov[0]);
	pos /= sqrt(rq1) + 1e-30; // Don't divide by zero
	float f = (atan(pos.z) / M_PI + 0.5f) * 360./clipping_fov[2];
	return vec4(pos.x * f, pos.y * f, depth, 1);
}
