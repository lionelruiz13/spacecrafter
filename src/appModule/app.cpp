/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2021 of the LSS Team & Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <exception>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sstream>

#include "spacecrafter.hpp"
#include "appModule/app.hpp"
#include "appModule/appDraw.hpp"
#include "appModule/fps.hpp"
#include "tools/app_settings.hpp"
#include "appModule/save_screen_interface.hpp"
#include "appModule/space_date.hpp"
#include "appModule/screenFader.hpp"
#include "appModule/fontFactory.hpp"
#include "appModule/mkfifo.hpp"
#include "coreModule/callbacks.hpp"
#include "coreModule/core.hpp"
#include "coreModule/coreLink.hpp"
#include "executorModule/executor.hpp"
#include "eventModule/event_handler.hpp"
#include "eventModule/event_recorder.hpp"
#include "interfaceModule/app_command_interface.hpp"
#include "scriptModule/script_interface.hpp"
#include "scriptModule/script_mgr.hpp"
#include "mainModule/sdl_facade.hpp"
#include "mediaModule/media.hpp"
#include "tools/call_system.hpp"
#include "tools/io.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"
#include "uiModule/ui.hpp"
#include "coreModule/time_mgr.hpp"
#include "mainModule/define_key.hpp"
#include "starModule/hip_star_mgr.hpp"

#include "eventModule/EventScriptHandler.hpp"
#include "eventModule/AppCommandHandler.hpp"
#include "eventModule/EventScreenFaderHandler.hpp"
#include "eventModule/EventSaveScreenHandler.hpp"
#include "eventModule/EventFpsHandler.hpp"
#include "eventModule/EventVideoHandler.hpp"
#include "eventModule/CoreHandler.hpp"
#include "EntityCore/EntityCore.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "EntityCore/Resource/FrameSender.hpp"
#include "EntityCore/Resource/SetMgr.hpp"
#include "EntityCore/Resource/TileMap.hpp"
#include "tools/NDISender.hpp"

EventRecorder* EventRecorder::instance = nullptr;
Context *Context::instance = nullptr;

App::App( SDLFacade* const sdl )
{
	mSdl = sdl;
	flagMasterput =false;
	//mSdl->getResolution( &width, &height );

	settings = AppSettings::Instance();
	InitParser conf;
	settings->loadAppSettings( &conf );
	Pipeline::setDefaultLineWidth(conf.getDouble(SCS_RENDERING, SCK_LINE_WIDTH));

	initVulkan(conf);

	fontFactory = std::make_unique<FontFactory>();

	media = std::make_shared<Media>();
	saveScreenInterface = std::make_shared<SaveScreenInterface>(width, height);
	saveScreenInterface->setVideoBaseName(settings->getVframeDirectory() + APP_LOWER_NAME);
	saveScreenInterface->setSnapBaseName(settings->getScreenshotDirectory() + APP_LOWER_NAME);

	screenFader =  std::make_unique<ScreenFader>();

	observatory = std::make_shared<Observer>();
	core = std::make_shared<Core>(width, height, media, fontFactory, mBoost::callback<void, std::string>(this, &App::recordCommand), observatory);
	coreLink = std::make_unique<CoreLink>(core);
	coreBackup = std::make_unique<CoreBackup>(core);

	screenFader->createSC_context();

	ui = std::make_shared<UI>(core, coreLink.get(), this, mSdl, media);
	commander = std::make_shared<AppCommandInterface>(core, coreLink, coreBackup, this, ui.get(), media, fontFactory);
	scriptMgr = std::make_shared<ScriptMgr>(commander, settings->getUserDir(), media);
	scriptInterface = std::make_shared<ScriptInterface>(scriptMgr);
	internalFPS = std::make_unique<Fps>();
	spaceDate = std::make_shared<SpaceDate>();

	executor = std::make_unique<Executor>(core, observatory.get());

	// fixation interface
	ui->initInterfaces(scriptInterface,spaceDate);
	commander->initInterfaces(scriptInterface, spaceDate, saveScreenInterface);

	EventRecorder::Init();
	eventRecorder = EventRecorder::getInstance();
	eventHandler = new EventHandler(eventRecorder);
	eventHandler-> add(new EventScriptHandler(scriptInterface.get()), Event::E_SCRIPT);
	eventHandler-> add(new EventCommandHandler(commander.get()), Event::E_COMMAND);
	eventHandler-> add(new EventFlagHandler(commander.get()), Event::E_FLAG);
	eventHandler-> add(new EventScreenFaderHandler(screenFader.get()), Event::E_SCREEN_FADER);
	eventHandler-> add(new EventScreenFaderInterludeHandler(screenFader.get()), Event::E_SCREEN_FADER_INTERLUDE);
	eventHandler-> add(new EventSaveScreenHandler(saveScreenInterface.get()), Event::E_SAVESCREEN);
	eventHandler-> add(new EventFpsHandler(internalFPS.get()), Event::E_FPS);
	eventHandler-> add(new EventAltitudeHandler(core), Event::E_CHANGE_ALTITUDE);
	eventHandler-> add(new EventObserverHandler(core), Event::E_CHANGE_OBSERVER);
	eventHandler-> add(new EventVideoHandler(ui.get(), scriptInterface.get()), Event::E_VIDEO);

	#if LINUX
	mkfifo = std::make_unique<Mkfifo>();
	#endif

	enable_mkfifo= false;
	enable_tcp= false;
	flagColorInverse= false;

	appDraw = std::make_unique<AppDraw>();
	appDraw->init(width, height);
}

App::~App()
{
	eventHandler->remove(Event::E_VIDEO);
	eventHandler->remove(Event::E_CHANGE_OBSERVER);
	eventHandler->remove(Event::E_CHANGE_ALTITUDE);
	eventHandler->remove(Event::E_FPS);
	eventHandler->remove(Event::E_SAVESCREEN);
	eventHandler->remove(Event::E_SCREEN_FADER_INTERLUDE);
	eventHandler->remove(Event::E_SCREEN_FADER);
	eventHandler->remove(Event::E_FLAG);
	eventHandler->remove(Event::E_COMMAND);
	eventHandler->remove(Event::E_SCRIPT);

	EventRecorder::End();

	sender = nullptr;
	FrameMgr::stopHelper();
	VulkanMgr::instance->waitIdle();
	Landscape::destroySC_context();
	appDraw.reset();
	if (enable_tcp)
		tcp.reset();
	#if LINUX
		mkfifo.reset();
	#endif
	ui.reset();
	scriptInterface.reset();
	scriptMgr.reset();
	media.reset();
	commander.reset();
	coreLink.reset();
	coreBackup.reset();
	core.reset();
	observatory.reset();
	saveScreenInterface.reset();
	internalFPS.reset();
	screenFader.reset();
	spaceDate.reset();
	fontFactory.reset();
	s_texture::forceUnload();
}

void App::initVulkan(InitParser &conf)
{
	cLog::get()->write("Initializing Vulkan...", LOG_TYPE::L_INFO);
	int antialiasing = 1 << static_cast<int>(std::log2(conf.getInt(SCS_RENDERING, SCK_ANTIALIASING)|1));
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	width = vkmgr.getSwapChainExtent().width;
	height = vkmgr.getSwapChainExtent().height;
	context.stagingMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, 512*1024*1024, "Staging BufferMgr");
	context.texStagingMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, 1*1024*1024*1024, "Texture staging BufferMgr");
	context.asyncTexStagingMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, BIG_TEXTURE_SIZE, "Async texture upload buffer"); // Support up to 16k x 8k big texture, around 682 Mo
	context.readbackMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_CACHED_BIT, 3*4*width*height, "readback BufferMgr");
	context.globalBuffer = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, 64*1024*1024, "global BufferMgr");
	context.uniformMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1*1024*1024, "uniform BufferMgr", true);
	context.tinyMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1*1024*1024, "tiny BufferMgr");
	context.ojmBufferMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, 128*1024*1024, "OJM BufferMgr");
	context.ojmVertexArray = std::make_unique<VertexArray>(vkmgr, context.ojmAlignment);
	context.ojmVertexArray->createBindingEntry(8*sizeof(float));
	context.ojmVertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	context.ojmVertexArray->addInput(VK_FORMAT_R32G32_SFLOAT);
	context.ojmVertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	context.indexBufferMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, 64*1024*1024, "indexBuffer BufferMgr");
	context.multiVertexArray = std::make_unique<VertexArray>(vkmgr, 6*sizeof(float));
	context.multiVertexMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context.multiVertexArray->alignment*32*1024, "draw_helper BufferMgr");
	context.setMgr = std::make_unique<SetMgr>(vkmgr, 1024, 512, 1024, 1, 64, true);
	context.starColorAttachment = std::make_unique<Texture>(vkmgr, vkmgr.getScreenRect().extent.width, vkmgr.getScreenRect().extent.height, VK_SAMPLE_COUNT_1_BIT, "star FBO", VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	context.starColorAttachment->use();
	sampleCount = static_cast<VkSampleCountFlagBits>(antialiasing);
	depthBuffer = std::make_unique<Texture>(vkmgr, width, height, sampleCount);
	depthBuffer->use();
	if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
		for (int i = 0; i < 3; ++i) {
			multisampleImage.push_back(std::make_unique<Texture>(vkmgr, width, height, sampleCount, "multisample color " + std::to_string(i), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT));
			multisampleImage.back()->use();

		}
	}
	context.graphicFamily = vkmgr.acquireQueue(context.graphicQueue, VulkanMgr::QueueType::GRAPHIC_COMPUTE, "main");
	if (context.graphicFamily) {
		context.computeQueue = context.graphicQueue;
	} else {
		context.graphicFamily = vkmgr.acquireQueue(context.graphicQueue, VulkanMgr::QueueType::GRAPHIC, "main compute");
		vkmgr.acquireQueue(context.computeQueue, VulkanMgr::QueueType::COMPUTE, "main graphic");
	}
	VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, context.graphicFamily->id};
	vkCreateCommandPool(vkmgr.refDevice, &poolInfo, nullptr, &context.cmdPool);
	context.cmdInfo.commandPool = context.cmdPool;
	// ========== DEFINE RENDERING ========== //
	context.render = std::make_unique<RenderMgr>(vkmgr);
	int colorID = context.render->attach(VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, vkmgr.getSwapchainView().empty() ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	int depthID = context.render->attach(VK_FORMAT_D24_UNORM_S8_UINT, sampleCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, false);
	int multiColorID = colorID;
	if (sampleCount != VK_SAMPLE_COUNT_1_BIT)
		multiColorID = context.render->attach(VK_FORMAT_B8G8R8A8_UNORM, sampleCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, false);
	// PASS_BACKGROUND
	context.render->setupClear(multiColorID, {0.f, 0.f, 0.f, 0.f});
	context.render->bindColor(multiColorID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	// Sync with host writes to uniform and tiny vertexBuffer using tinyMgr
	context.render->addDependency(VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT_KHR, false);
	if (multiColorID == colorID) {
		// Sync with semaphore
		context.render->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false);
	} else {
		// Sync with load op
		context.render->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	}
	// Sync with texture update
	context.render->addDependency(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, VK_ACCESS_SHADER_READ_BIT, false);
	// For hip star fbo sync with event
	context.render->addDependencyFrom(-1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, false);
	context.render->pushLayer();
	// PASS_MULTISAMPLE_DEPTH
	context.render->bindColor(multiColorID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	context.render->setupClear(depthID, 1.f);
	context.render->bindDepth(depthID, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	if (multiColorID != colorID) {
		context.render->bindResolveDst(colorID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		// Sync with semaphore
		context.render->addDependencyFrom(-1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false);
	}
	// Sync with use in previous subPass
	context.render->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
	// Sync with depth loadOp
	context.render->addDependencyFrom(-1, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
	// Sync with host writes to uniform and tiny vertexBuffer using tinyMgr
	context.render->addDependencyFrom(-1, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT_KHR, false);
	context.render->pushLayer();
	// PASS_FOREGROUND
	context.render->bindColor(colorID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	// Sync with resolve
	context.render->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	// Sync with host writes to uniform and tiny vertexBuffer using tinyMgr
	context.render->addDependencyFrom(-1, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT_KHR, false);
	// For video texture sync with event
	context.render->addDependencyFrom(-1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, false);
	context.render->pushLayer();
	// For video texture sync with event
	context.render->addDependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, false);
	// For hip star fbo sync with event
	context.render->addDependencyFrom(PASS_BACKGROUND, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false);
	// Sync with screen capture
	context.render->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, false);
	// Sync with presentation engine
	context.render->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, false);
	context.render->build(3);
	// ========== END DEFINE RENDERING ========== //
	context.transferSync = std::make_unique<SyncEvent>();
	context.transferSync->bufferBarrier(*context.globalBuffer, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR);
	context.transferSync->bufferBarrier(*context.multiVertexMgr, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR);
	context.transferSync->bufferBarrier(*context.ojmBufferMgr, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR);
	context.transferSync->bufferBarrier(*context.indexBufferMgr, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_INDEX_READ_BIT_KHR);
	context.transferSync->build();
	context.transfers.resize(3);
	context.fences.resize(3);
	context.debugFences.resize(3);
	context.semaphores.resize(6);
	context.graphicTransferCmd.resize(3);
	context.starUsed.resize(3);
	VkFenceCreateInfo fenceInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
	VkSemaphoreCreateInfo semaphoreInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
	// FrameMgr::startHelper();
	for (uint8_t i = 0; i < 3; ++i) {
		context.transfers[i] = std::make_unique<TransferMgr>(*context.stagingMgr, 64*1024*1024);
		// context.starSync[i] = std::make_unique<SyncEvent>(&vkmgr);
		// context.starSync[i]->imageBarrier(*context.starColorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT_KHR | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
		// context.starSync[i]->build();
		context.frame.push_back(std::make_unique<FrameMgr>(vkmgr, *context.render, i, width, height, "main " + std::to_string(i), (void (*)(void *, int)) &App::submitFrame, (void *) this));
		if (vkmgr.getSwapchainView().empty()) {
			senderImage.push_back(std::make_unique<Texture>(vkmgr, width, height, VK_SAMPLE_COUNT_1_BIT, "main color " + std::to_string(i), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT));
			context.frame.back()->bind(colorID, *senderImage.back());
		} else {
			context.frame.back()->bind(colorID, vkmgr.getSwapchainView()[i]);
		}
		context.frame.back()->bind(depthID, *depthBuffer);
		// context.frame.back()->bind(starID, *context.starColorAttachment);
		if (multiColorID != colorID)
			context.frame.back()->bind(multiColorID, *multisampleImage[i]);
		context.frame.back()->build(context.graphicFamily->id, true, true);
		vkCreateFence(vkmgr.refDevice, &fenceInfo, nullptr, context.fences.data() + i);
		fenceInfo.flags = 0;
		vkCreateFence(vkmgr.refDevice, &fenceInfo, nullptr, context.debugFences.data() + i);
		vkCreateSemaphore(vkmgr.refDevice, &semaphoreInfo, nullptr, context.semaphores.data() + i);
		vkCreateSemaphore(vkmgr.refDevice, &semaphoreInfo, nullptr, context.semaphores.data() + i + 3);
		context.graphicTransferCmd[i] = context.frame.back()->createMain();
	}
	context.transfer = context.transfers[0].get(); // Assume the first frame is the frame 0
	context.helper = std::make_unique<DrawHelper>();
	if (vkmgr.getSwapchainView().empty()) {
		sender = std::make_unique<NDISender>(vkmgr, senderImage, context.fences.data());
	}
	cLog::get()->write("Vulkan initialization completed", LOG_TYPE::L_INFO);
}

int App::getFpsClock() const {
 	return internalFPS->getFps();
}

void App::setLineWidth(float w) const {
	appDraw->setLineWidth(w);
}

float App::getLineWidth() const {
	return appDraw->getLineWidth();
}

float App::getFlagAntialiasLines() const{
	return appDraw->getFlagAntialiasLines();
}

void App::flag(APP_FLAG layerValue, bool _value) {
	switch(layerValue) {
		case APP_FLAG::VISIBLE :
				flagVisible = _value; break;
		case APP_FLAG::ALIVE :
				flagAlive = _value; break;
		case APP_FLAG::COLOR_INVERSE :
				flagColorInverse = _value; break;
		case APP_FLAG::ANTIALIAS :
				appDraw->setFlagAntialiasLines(_value); break;
		default: break;
	}
}

void App::toggle(APP_FLAG layerValue)
{
		switch(layerValue) {
		case APP_FLAG::VISIBLE :
				flagVisible = !flagVisible; break;
		case APP_FLAG::ALIVE :
				flagAlive = !flagAlive; break;
		case APP_FLAG::COLOR_INVERSE :
				flagColorInverse = !flagColorInverse; break;
		case APP_FLAG::ANTIALIAS :
				appDraw->flipFlagAntialiasLines(); break;
		default: break;
	}
}


std::string App::getAppLanguage() {
	return Translator::globalTranslator.getLocaleName();
}


//! Load configuration from disk
void App::init()
{
	// Initialize video device and other sdl parameters
	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	appDraw->setFlagAntialiasLines(conf.getBoolean(SCS_RENDERING, SCK_FLAG_ANTIALIAS_LINES));

	internalFPS->setMaxFps(conf.getDouble (SCS_VIDEO,SCK_MAXIMUM_FPS));
	internalFPS->setVideoFps(conf.getDouble(SCS_VIDEO,SCK_REC_VIDEO_FPS));

	std::string appLocaleName = conf.getStr(SCS_LOCALIZATION, SCK_APP_LOCALE); //, "system");
	spaceDate->setTimeFormat(spaceDate->stringToSTimeFormat(conf.getStr(SCS_LOCALIZATION, SCK_TIME_DISPLAY_FORMAT)));
	spaceDate->setDateFormat(spaceDate->stringToSDateFormat(conf.getStr(SCS_LOCALIZATION, SCK_DATE_DISPLAY_FORMAT)));
	setAppLanguage(appLocaleName);

	// time_zone used to be in init_location section of config, so use that as fallback when reading config - Rob
	std::string tzstr = conf.getStr(SCS_LOCALIZATION, SCK_TIME_ZONE);
	#if LINUX
	if (tzstr == "system_default") {
		spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT);
		// Set the program global intern timezones variables from the system locale
		tzset();
	} else {
	#endif
		if (tzstr == "gmt+x") {
			spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_GMT_SHIFT);
		} else {
			// We have a custom time zone name
			spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT);
			spaceDate->setCustomTzName(tzstr);
		}
	#if LINUX
	}
	#endif

	core->init(conf);

	// Navigation section
	PresetSkyTime 		= conf.getDouble (SCS_NAVIGATION, SCK_PRESET_SKY_TIME); //,2451545.);
	StartupTimeMode 	= conf.getStr(SCS_NAVIGATION, SCK_STARTUP_TIME_MODE);	// Can be "now" or "preset"
	DayKeyMode 			= conf.getStr(SCS_NAVIGATION, SCK_DAY_KEY_MODE); //,"calendar");  // calendar or sidereal
	cLog::get()->write("Read daykeymode as <" + DayKeyMode + ">", LOG_TYPE::L_INFO);

	if (StartupTimeMode=="preset" || StartupTimeMode=="Preset")
		coreLink->setJDay(PresetSkyTime - spaceDate->getGMTShift(PresetSkyTime) * JD_HOUR);
	else core->setTimeNow();

	// initialisation of the User Interface
	ui->init(conf);
	ui->localizeTui();

	//set all color
	core->setColorScheme(settings->getConfigFile(), SCS_COLOR);

	// play startup script
	scriptMgr->playStartupScript();
	// on sauvegarde ici l'état des composants du logiciel.
	coreBackup->saveGridState();
	coreBackup->saveDisplayState();
	coreBackup->saveLineState();

	fontFactory->reloadAllFont();
	commander->deleteVar();
}

//! Load configuration from disk
void App::firstInit()
{
	appDraw->initSplash();

	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	fontFactory->init(std::min(width,height), conf);
	fontFactory->initMediaFont(media.get());
	fontFactory->buildAllFont();

	ui->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_UI));

	core->init(conf);
	ui->init(conf);
	ui->localizeTui();
	ui->initTui();

	appDraw->createSC_context();
	media->initVR360();
	media->createSC_context();

	enable_tcp=conf.getBoolean(SCS_IO, SCK_ENABLE_TCP);
	enable_mkfifo=conf.getBoolean(SCS_IO, SCK_ENABLE_MKFIFO);
	flagAlwaysVisible = conf.getBoolean(SCS_MAIN,SCK_FLAG_ALWAYS_VISIBLE);
	flagMasterput=conf.getBoolean(SCS_IO, SCK_FLAG_MASTERPUT);

	if (enable_tcp) {
		int port = conf.getInt(SCS_IO, SCK_TCP_PORT_IN);
		int buffer_in_size=conf.getInt(SCS_IO, SCK_TCP_BUFFER_IN_SIZE);
		cLog::get()->write("buffer TCP taille " + std::to_string(buffer_in_size));
		tcp = std::make_unique<ServerSocket>(port, 16, buffer_in_size);
		tcp->open();
		commander->setTcp(tcp.get());
	}

	#if LINUX // special mkfifo
	if (enable_mkfifo) {
		std::string mplayerMkfifoName = conf.getStr(SCS_IO, SCK_MPLAYER_MKFIFO_NAME);
		std::string mkfifo_file_in = conf.getStr(SCS_IO, SCK_MKFIFO_FILE_IN);
		int buffer_in_size=conf.getInt(SCS_IO, SCK_MKFIFO_BUFFER_IN_SIZE);
		cLog::get()->write("buffer MKFIFO taille "+ std::to_string(buffer_in_size));
		mkfifo->init(mkfifo_file_in, buffer_in_size);
	}
	#endif

	cLog::get()->write(CallSystem::getRamInfo());
	cLog::get()->mark();
	cLog::get()->write("End of loading SC");
	cLog::get()->write("End of loading SC",LOG_TYPE::L_INFO,LOG_FILE::SCRIPT);
	cLog::get()->mark();

	this->init();
}


void App::updateFromSharedData()
{
	if (enable_mkfifo) {
		std::string out;
		#if LINUX
		if (mkfifo->update(out)) {
			cLog::get()->write("get mkfifo: " + out);
			commander->executeCommand(out);
		}
		#endif
	}
	if (enable_tcp) {
		std::string out;
		do {
			out = tcp->getInput();
			if (!out.empty()) {
				cLog::get()->write("get tcp : " + out);
				commander->executeCommand(out);
			}
		} while (!out.empty());
	}
	if (flagMasterput==true)
		masterput();
}


void App::update(int delta_time)
{
	internalFPS->addFrame();
	// change time rate if needed to fast forward scripts
	delta_time *= scriptMgr->getMuliplierRate();
	// run command from a running script
	scriptMgr->update(delta_time);
	if (!scriptMgr->isPaused() || !scriptMgr->isFaster() )	media->audioUpdate(delta_time);
	// run any incoming command from shared memory interface
	updateFromSharedData();

	ui->updateTimeouts(delta_time);
	ui->tuiUpdateWidgets();

	if (!scriptMgr->isPaused()) media->imageUpdate(delta_time);

	media->playerUpdate();
	screenFader->update(delta_time);
	media->faderUpdate(delta_time);

	executor->update(delta_time);
}


//! Main drawinf function called at each frame
void App::draw(int delta_time)
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	context.lastFrameIdx = context.frameIdx;
	// Acquire a frame
	if (sender) {
		sender->acquireFrame(context.frameIdx);
	} else {
		auto res = vkAcquireNextImageKHR(vkmgr.refDevice, vkmgr.getSwapchain(), 1000000, context.semaphores[context.lastFrameIdx + 3], VK_NULL_HANDLE, &context.frameIdx);
		switch (res) {
			case VK_SUCCESS:
				break;
			case VK_SUBOPTIMAL_KHR:
				vkmgr.putLog("Suboptimal swapchain", LogType::WARNING);
				break;
			case VK_TIMEOUT:
				vkmgr.putLog("Timeout for swapchain acquire", LogType::WARNING);
				return;
			default:
				vkmgr.putLog("Invalid swapchain", LogType::ERROR);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				return;
		}
		context.helper->waitFrame(context.lastFrameIdx);
		res = vkWaitForFences(vkmgr.refDevice, 1, &context.fences[context.lastFrameIdx], VK_TRUE, 30L*1000*1000*1000);
		if (res != VK_SUCCESS) {
			vkmgr.putLog("CRITICAL : Frame not completed after 30s (timeout)", LogType::ERROR);
			// Ok, but... what to do then ?
			// Can't leave, as VulkanMgr internally call vkDeviceWaitIdle on destroy, which would never end in this case
			exit(2);
		}
		vkResetFences(vkmgr.refDevice, 1, &context.fences[context.frameIdx]);
	}
	s_texture::update();
	context.frame[context.frameIdx]->discardRecord();
	context.setMgr->update();
	context.transfer = context.transfers[context.frameIdx].get();
	for (auto &buffer : context.transientBuffer[context.frameIdx]) {
		context.stagingMgr->releaseBuffer(buffer);
	}
	context.transientBuffer[context.frameIdx].clear();
	s_font::beginPrint();

	executor->draw(delta_time);
	context.helper->nextDraw(PASS_FOREGROUND);
	// Draw the Graphical ui and the Text ui
	ui->draw(executor->getExecutorModule());
	//inversion des couleurs pour un ciel blanc
	if (flagColorInverse)
		appDraw->drawColorInverse();

	//draw video frame to classical viewport
	media->drawViewPort();
	//draw text user
	media->textDraw();
	context.helper->endDraw();

	// Fill with black around the circle
	appDraw->drawViewportShape();

	screenFader->draw();
	// auto mainCmd = context.frame[context.frameIdx]->preBegin();
	context.helper->submitFrame(context.frameIdx);
	// context.frame[context.frameIdx]->submitInline();
	context.transfer = context.transfers[(context.frameIdx + 1) % 3].get(); // Assume the next frame follow the previous one
}

//! @brief Set the application locale. This apply to GUI, console messages etc..
void App::setAppLanguage(const std::string& newAppLocaleName)
{
	// Update the translator with new locale name
	Translator::globalTranslator = Translator(settings->getLanguageDir(), newAppLocaleName);
	cLog::get()->write("Application locale is " + Translator::globalTranslator.getLocaleName(), LOG_TYPE::L_INFO);
	ui->localizeTui();
}

//! For use by TUI - saves all current settings
void App::saveCurrentConfig(const std::string& confFile)
{
	// No longer resaves everything, just settings user can change through UI
	cLog::get()->write("Saving configuration file " + confFile + " ...", LOG_TYPE::L_INFO);
	InitParser conf;
	conf.load(confFile);

	// Main section
	conf.setStr	(SCS_MAIN,SCK_VERSION, VERSION);
	// localization section
	conf.setStr(SCS_LOCALIZATION, SCK_APP_LOCALE, getAppLanguage());
	conf.setStr(SCS_LOCALIZATION, SCK_TIME_DISPLAY_FORMAT, spaceDate->getTimeFormatStr());
	conf.setStr(SCS_LOCALIZATION, SCK_DATE_DISPLAY_FORMAT, spaceDate->getDateFormatStr());
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_CUSTOM) {
		conf.setStr(SCS_LOCALIZATION, SCK_TIME_ZONE, spaceDate->getCustomTzName());
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT) {
		conf.setStr(SCS_LOCALIZATION, SCK_TIME_ZONE, "system_default");
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_GMT_SHIFT) {
		conf.setStr(SCS_LOCALIZATION, SCK_TIME_ZONE, "gmt+x");
	}
	conf.setDouble (SCS_NAVIGATION, SCK_PRESET_SKY_TIME, PresetSkyTime);
	conf.setStr	(SCS_NAVIGATION, SCK_STARTUP_TIME_MODE, StartupTimeMode);
	conf.setStr	(SCS_NAVIGATION, SCK_DAY_KEY_MODE, DayKeyMode);
	conf.setDouble(SCS_RENDERING, SCK_LINE_WIDTH, appDraw->getLineWidth());
	conf.setBoolean(SCS_RENDERING, SCK_FLAG_ANTIALIAS_LINES, appDraw->getFlagAntialiasLines());

	ui->saveCurrentConfig(conf);
	core->saveCurrentConfig(conf);

	// Get landscape and other observatory info
	coreLink->observerSetConf(conf, SCS_INIT_LOCATION);
	conf.save(confFile);
}

void App::recordCommand(const std::string& commandline)
{
	scriptMgr->recordCommand(commandline);
}

//! Masterput script launch
void App::masterput()
{
	std::string action = settings->getFtpDir()+"pub/masterput.launch";
	FILE * tempFile = fopen(action.c_str(),"r");
	if (tempFile) {
		fclose(tempFile);
		//cLog::get()->write("MASTERPUT is in action", LOG_TYPE::L_INFO);
		unlink(action.c_str());
		scriptMgr->playScript(settings->getFtpDir()+"pub/script.sts");
	}
}

void App::startMainLoop()
{
	flagVisible = true;		// At The Beginning, Our App Is Visible
	flagAlive = true; 		// au debut, on veut que l'application ne s'arrete pas :)

	//center mouse in middle screen
	mSdl->warpMouseInCenter();

	internalFPS->init();
	internalFPS->selectMaxFps();

	SDL_TimerID my_timer_id = SDL_AddTimer(1000, internalFPS->callbackfunc, nullptr);

	// Start the main loop
	while (flagAlive) {
		while (SDL_PollEvent(&E)) {	// Fetch all Event Of The Queue
			ui->handleInputs(E);
		}

		//analyse le joystick au cas ou des events ont été accumulés pour le joystick
		ui->handleDeal();

		// on applique toutes les modifications faites dans ui etc
		eventHandler->handleEvents(executor.get());

		// If the application is not visible
		if (!flagVisible && !flagAlwaysVisible) {
			// Reduce GPU memory consumption
			s_texture::releaseUnusedMemory();
			// Leave the CPU alone, don't waste time, simply wait for an event
			SDL_WaitEvent(NULL);
		} else {
			internalFPS->setTickCount();
			// Wait a while if drawing a frame right now would exceed our preferred framerate.
			internalFPS->wait();
			internalFPS->setTickCount();

			deltaTime = internalFPS->getDeltaTime();

			this->update(deltaTime);		// And update the motions and data
			this->draw(deltaTime);			// Do the drawings!

			internalFPS->setLastCount();
		}
	}

	SDL_RemoveTimer(my_timer_id);
	CallSystem::killAllPidFrom("vlc");
	CallSystem::killAllPidFrom("mplayer");
}

void App::switchMode(const std::string setValue) {
		executor->switchMode(setValue);
}

void App::submitFrame(App *self, int id)
{
	VkCommandBuffer mainCmd = self->context.frame[id]->getMainHandle();
	vkCmdEndRenderPass(mainCmd);
	self->media->playerRecordUpdateDependency(mainCmd);
	if (self->context.starUsed[id]) {
		self->context.starUsed[id]->syncFramebuffer(mainCmd);
		self->context.starUsed[id] = nullptr;
	}
	self->saveScreenInterface->readScreenShot(mainCmd);
	if (self->sender) {
		self->sender->setupReadback(mainCmd, id);
	}
	vkEndCommandBuffer(mainCmd);

	if (self->sender) {
		VkSubmitInfo submit {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &mainCmd, 0, nullptr};
		vkQueueSubmit(self->context.graphicQueue, 1, &submit, self->context.fences[id]);
		self->sender->presentFrame(id);
	} else {
		const int lastId = (id + 2) % 3;
		VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &self->context.semaphores[3 + lastId], &stage, 1, &mainCmd, 1, &self->context.semaphores[id]};
		vkQueueSubmit(self->context.graphicQueue, 1, &submit, self->context.fences[id]);
		VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1, &self->context.semaphores[id], 1, &VulkanMgr::instance->getSwapchain(), (uint32_t *) &id, nullptr};
		vkQueuePresentKHR(self->context.graphicQueue, &presentInfo);
	}
}
