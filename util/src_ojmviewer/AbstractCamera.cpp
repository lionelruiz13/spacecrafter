#include "AbstractCamera.hpp"  
 

glm::vec3 CAbstractCamera::UP = glm::vec3(0,1,0);

CAbstractCamera::CAbstractCamera(void) 
{ 
	Znear = 0.1f;
	Zfar  = 1000;
}


CAbstractCamera::~CAbstractCamera(void)
{
}

void CAbstractCamera::SetupProjection(const float fovy, const float aspRatio, const float nr, const float fr) {
	P = glm::perspective(glm::radians(fovy), aspRatio, nr, fr); 
	Znear = nr;
	Zfar = fr;
	fov = fovy;
	aspect_ratio = aspRatio; 
} 

const glm::mat4 CAbstractCamera::GetViewMatrix() const {
	return V;
}

const glm::mat4 CAbstractCamera::GetProjectionMatrix() const {
	return P;
}

const glm::vec3 CAbstractCamera::GetPosition() const {
	return position;
}

void CAbstractCamera::SetPosition(const glm::vec3& p) {
	position = p;
}
  
const float CAbstractCamera::GetFOV() const {
	return fov;
} 
void CAbstractCamera::SetFOV(const float fovInDegrees) {
	fov = fovInDegrees;
	P = glm::perspective(glm::radians(fovInDegrees), aspect_ratio, Znear, Zfar); 
}
const float CAbstractCamera::GetAspectRatio() const {
	return aspect_ratio;
}

void CAbstractCamera::Rotate(const float y, const float p, const float r) {
	  yaw=glm::radians(y);
	pitch=glm::radians(p);
	 roll=glm::radians(r);
	Update();
}
