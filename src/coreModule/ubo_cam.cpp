#include "coreModule/ubo_cam.hpp"

UBOCam::UBOCam(const std::string& UBOName_)
{
	UBOName = UBOName_;
	block_id = 0;//TODO a incrémenter à chaque ubo
	glGenBuffers(1, &block_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, block_buffer);
	glBufferData(GL_UNIFORM_BUFFER,	sizeof(UBOData), NULL, GL_DYNAMIC_DRAW);
	UBOdata.time = 0.00;
	UBOdata.ambient = 0.03;
}

UBOCam::~UBOCam()
{
	glDeleteBuffers(1,&block_buffer);
}

void UBOCam::IndexAndBinding(GLuint program)   //TODO � tester
{
	block_index = glGetUniformBlockIndex(program, UBOName.c_str());
	glUniformBlockBinding(program, block_index, block_id);
}

void UBOCam::update()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, block_id, block_buffer);
	float* cam_ubo_ptr = (float*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memcpy(cam_ubo_ptr, &UBOdata, sizeof(UBOData));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}
