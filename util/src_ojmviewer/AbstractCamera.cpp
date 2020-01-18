#include "AbstractCamera.hpp"

 
Vec3f CAbstractCamera::UP = Vec3f(0,1,0);

CAbstractCamera::CAbstractCamera(void) 
{  
}


CAbstractCamera::~CAbstractCamera(void)
{
}

void CAbstractCamera::SetupProjection(const float fovy, const float aspRatio) {
	P = Mat4f::perspective(fovy, aspRatio, 0.1f, 1000.0f);
	 
	fov = fovy;
	aspect_ratio = aspRatio;
} 

const Mat4f CAbstractCamera::GetViewMatrix() const {
	return V;
}

const Mat4f CAbstractCamera::GetProjectionMatrix() const {
	return P;
}

const Vec3f CAbstractCamera::GetPosition() const {
	return position;
}

void CAbstractCamera::SetPosition(const Vec3f p) {
	position = p;
}
 
Mat4f CAbstractCamera::GetMatrixUsingYawPitchRoll(const float yaw, const float pitch, const float roll) {
	 
	return Mat4f::yawPitchRoll(yaw, pitch, roll);  
}

const float CAbstractCamera::GetFOV() const {
	return fov;
} 

const float CAbstractCamera::GetAspectRatio() const {
	return aspect_ratio;
}

 
