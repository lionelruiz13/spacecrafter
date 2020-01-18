#pragma once
#include "AbstractCamera.hpp"

class CFreeCamera :
	public CAbstractCamera
{
public:
	CFreeCamera(void);
	~CFreeCamera(void);

	void Update();
	void Rotate(const float yaw, const float pitch, const float roll);
	void Walk(const float amount);
	void Strafe(const float amount);
	void Lift(const float amount);
	 

protected:
	float yaw=0, pitch=0, roll=0;
	
	Vec3f translation;
};

