#include "coreModule/ubo_cam.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"

SharedBuffer<UBOData> *UBOCam::ubo = nullptr;

UBOCam::UBOCam() : UBOdata(*Context::instance->uniformMgr)
{
	UBOdata->time = 0.00;
	UBOdata->ambient = 0.03;
	Context::instance->layouts.insert(Context::instance->layouts.begin(), std::make_unique<PipelineLayout>(*VulkanMgr::instance));
	globalLayout = Context::instance->layouts.front().get();
	globalLayout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 0);
	globalLayout->buildLayout();
	globalLayout->build();
	Context::instance->uboSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, globalLayout);
	globalSet = Context::instance->uboSet.get();
	globalSet->bindUniform(UBOdata, 0, sizeof(UBOData));
	ubo = &UBOdata;
}

UBOCam::~UBOCam()
{
	ubo = nullptr;
}

void UBOCam::update()
{
	time += 0.01/60.;
	UBOdata->time = time;
}
