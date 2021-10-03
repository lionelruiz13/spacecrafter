#include "coreModule/ubo_cam.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/PipelineLayout.hpp"

UBOCam::UBOCam(ThreadContext *context, const std::string& UBOName_) : uniform(new Uniform(context->surface, sizeof(UBOData))), UBOdata(*static_cast<struct UBOData*>(uniform->data))
{
	UBOName = UBOName_;
	UBOdata.time = 0.00;
	UBOdata.ambient = 0.03;
	globalLayout = context->global->globalLayout = new PipelineLayout(context->surface);
	globalLayout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0);
	globalLayout->buildLayout();
	globalLayout->build();
	globalSet = context->global->globalSet = new Set(context->surface, context->setMgr, globalLayout);
	globalSet->bindUniform(uniform, 0);
}

UBOCam::~UBOCam()
{
	delete uniform;
	delete globalLayout;
	delete globalSet;
	//glDeleteBuffers(1,&block_buffer);
}

// void UBOCam::IndexAndBinding(uint32_t program)   //TODO � tester
// {
	/*
	block_index = glGetUniformBlockIndex(program, UBOName.c_str());
	glUniformBlockBinding(program, block_index, block_id);
	*/
// }

void UBOCam::update()
{
	UBOdata.time += 0.01/60.;
	/*
	glBindBufferBase(GL_UNIFORM_BUFFER, block_id, block_buffer);
	float* cam_ubo_ptr = (float*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memcpy(cam_ubo_ptr, &UBOdata, sizeof(UBOData));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	*/
}
