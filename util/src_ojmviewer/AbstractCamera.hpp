#pragma once

#include "../../src/vecmath.hpp"

class CAbstractCamera
{
public:
	CAbstractCamera(void);
	~CAbstractCamera(void);
	 
	void SetupProjection(const float fovy=45.0f, const float aspectRatio=1.33333f);
	
	virtual void Update() = 0;
	virtual void Rotate(const float yaw, const float pitch, const float roll) = 0;
	//virtual void Translate(const float dx, const float dy, const float dz) = 0;

	const Mat4f GetViewMatrix() const;
	const Mat4f GetProjectionMatrix() const;

	void SetPosition(const Vec3f v);
	const Vec3f GetPosition() const;

	Mat4f GetMatrixUsingYawPitchRoll(const float yaw, const float pitch, const float roll);

	const float GetFOV() const;
	const float GetAspectRatio() const; 

	
protected:	 
	float fov, aspect_ratio;
	static Vec3f UP;
	Vec3f look;
	Vec3f up;
	Vec3f right; 
	Vec3f position;
	Mat4f V; //view matrix
	Mat4f P; //projection matrix
};

