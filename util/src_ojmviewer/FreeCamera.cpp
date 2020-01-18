#include "FreeCamera.hpp"


CFreeCamera::CFreeCamera()
{
	translation =v3fNull;
}


CFreeCamera::~CFreeCamera(void)
{
}
 
void CFreeCamera::Update() {
	Mat4f R = GetMatrixUsingYawPitchRoll(yaw,pitch,roll); 
	position+=translation;
	translation=v3fNull;

	look = Vec3f(R*Vec4f(0,0,1,0));
	Vec3f tgt  = position+look;
	up   = Vec3f(R*Vec4f(0,1,0,0));
	right =  look^up;
	V = Mat4f::lookAt(position, tgt, up);

	//normalize
	//look = glm::normalize(look);
	//up = glm::normalize(up);
	//right = glm::normalize(right);
}

void CFreeCamera::Rotate(const float y, const float p, const float r) {
	yaw+=y;
	pitch+=p;
	roll+=r;
}


void CFreeCamera::Walk(const float amount) {
	translation += (look*amount);
}

void CFreeCamera::Strafe(const float amount) {
	translation += (right*amount);
}

void CFreeCamera::Lift(const float amount) {
	translation += (up*amount);
}
