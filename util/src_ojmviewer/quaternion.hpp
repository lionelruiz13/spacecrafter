#ifndef _QUATERNIONS_HPP_
#define _QUATERNIONS_HPP_

// const used to convert degrees into radians
#define TAU 2.0 * M_PI
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * M_PI) //57.2957795

/*---------------------------- QUATERNIONS ---------------------------*/

/* create a unit quaternion q from an angle in degrees a, and an axis x,y,z */
void create_versor(float* q, float a, float x, float y, float z)
{
	float rad = ONE_DEG_IN_RAD * a;
	q[0] = cosf (rad / 2.0f);
	q[1] = sinf (rad / 2.0f) * x;
	q[2] = sinf (rad / 2.0f) * y;
	q[3] = sinf (rad / 2.0f) * z;
}

/* convert a unit quaternion q to a 4x4 matrix m */
void quat_to_mat4(float* m, float* q)
{
	float w = q[0];
	float x = q[1];
	float y = q[2];
	float z = q[3];
	m[0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
	m[1] = 2.0f * x * y + 2.0f * w * z;
	m[2] = 2.0f * x * z - 2.0f * w * y;
	m[3] = 0.0f;
	m[4] = 2.0f * x * y - 2.0f * w * z;
	m[5] = 1.0f - 2.0f * x * x - 2.0f * z * z;
	m[6] = 2.0f * y * z + 2.0f * w * x;
	m[7] = 0.0f;
	m[8] = 2.0f * x * z + 2.0f * w * y;
	m[9] = 2.0f * y * z - 2.0f * w * x;
	m[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}

/* normalise a quaternion in case it got a bit mangled */
void normalise_quat(float* q)
{
	// norm(q) = q / magnitude (q)
	// magnitude (q) = sqrt (w*w + x*x...)
	// only compute sqrt if interior sum != 1.0
	float sum = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
	// NB: floats have min 6 digits of precision
	const float thresh = 0.0001f;
	if (fabs (1.0f - sum) < thresh) {
		return;
	}
	float mag = sqrt (sum);
	for (int i = 0; i < 4; i++) {
		q[i] = q[i] / mag;
	}
}


/* multiply quaternions to get another one. result=R*S */
void mult_quat_quat(float* result, float* r, float* s)
{
	float tmp[4];
	tmp[0] = s[0] * r[0] - s[1] * r[1] - s[2] * r[2] - s[3] * r[3];
	tmp[1] = s[0] * r[1] + s[1] * r[0] - s[2] * r[3] + s[3] * r[2];
	tmp[2] = s[0] * r[2] + s[1] * r[3] + s[2] * r[0] - s[3] * r[1];
	tmp[3] = s[0] * r[3] - s[1] * r[2] + s[2] * r[1] + s[3] * r[0];
	result[0]=tmp[0];
	result[1]=tmp[1];
	result[2]=tmp[2];
	result[3]=tmp[3];
	// re-normalise in case of mangling
	normalise_quat(result);
}


#endif
