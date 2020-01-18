#define _USE_MATH_DEFINES
#include <cmath>
#include "TargetCamera.hpp"
#include <iostream>

CTargetCamera::CTargetCamera(void)
{  
	right = Vec3f(1,0,0);
	up = Vec3f(0,1,0);
	look = Vec3f(0,0,-1);
	minRy = -60;
	maxRy = 60;
	minDistance = 1;
	maxDistance = 10;
}


CTargetCamera::~CTargetCamera(void)
{
}
 
void CTargetCamera::Update() {
	 
	//~ Mat4f R = glm::yawPitchRoll(glm::radians(m_yaw),glm::radians(m_pitch),0.0f);
	Mat4f R = GetMatrixUsingYawPitchRoll(m_yaw,m_pitch,0.0f);
	Vec3f T = Vec3f(0,0,distance);
	T = Vec3f(R*Vec4f(T,0.0f));
	position = target + T;
	look = target-position;
	look.normalize();
	up = Vec3f(R*Vec4f(UP,0.0f));
	right = look ^ up;
	V = Mat4f::lookAt(position, target, up); 
}

void CTargetCamera::SetTarget(const Vec3f tgt) {
	target = tgt; 
	distance = (position - target).length();
	distance = std::max(minDistance, std::min(distance, maxDistance));
	V = Mat4f::lookAt(position, target, up);

	m_yaw = 0;
	m_pitch = 0;

	if(V[0][0] < 0)
		m_yaw = ((float)(M_PI - asinf(-V[2][0])) )/M_PI*180.;
	else
		m_yaw = (asinf(-V[2][0]))/M_PI*180.;

	m_pitch = (asinf(-V[1][2]))/M_PI*180.; 
}

const Vec3f CTargetCamera::GetTarget() const {
	return target;
} 

void CTargetCamera::Rotate(const float yaw, const float pitch, const float roll) {
 	m_yaw+=yaw;
	m_pitch+=pitch;
 	m_pitch=std::min( std::max(m_pitch, minRy), maxRy);
	Update();
}
 
void CTargetCamera::Pan(const float dx, const float dy) {
	Vec3f X = right*dx;
	Vec3f Y = up*dy;
	position += X + Y;
	  target += X + Y;
	Update();
}

 
void CTargetCamera::Zoom(const float amount) { 
	position += look * amount;
	distance = (position - target).length();
	distance=std::max(minDistance, std::min(distance, maxDistance));
	Update();
}
 
void CTargetCamera::Move(const float dx, const float dy) {
	Vec3f X = right*dx;
	Vec3f Y = look*dy;
	position += X + Y;
	  target += X + Y;
	Update();
}
