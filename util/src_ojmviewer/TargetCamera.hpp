#pragma once
#include "AbstractCamera.hpp"

class CTargetCamera :
	public CAbstractCamera
{
public:
	CTargetCamera(void);
	~CTargetCamera(void);

	void Update();
	void Rotate(const float yaw, const float pitch, const float roll);
	 
	void SetTarget(const Vec3f tgt);
	const Vec3f GetTarget() const;

	void Pan(const float dx, const float dy);
	void Zoom(const float amount );
	void Move(const float dx, const float dz);

protected:
	Vec3f target;  
	float m_yaw, m_pitch, m_roll;
	 
	float minRy, maxRy;
	float distance;
	float minDistance, maxDistance;

};

