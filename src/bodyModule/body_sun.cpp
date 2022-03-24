/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
 * Copyright (C) 2017-2020 AssociationSirius
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "bodyModule/body_sun.hpp"

#include "bodyModule/axis.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/halo.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "bodyModule/body_color.hpp"
#include "navModule/observer.hpp"
#include "tools/sc_const.hpp"
#include "tools/s_font.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

Sun::Sun(std::shared_ptr<Body> parent,
         const std::string& englishName,
         bool flagHalo,
         double radius,
         double oblateness,
         std::unique_ptr<BodyColor> myColor,
         float _sol_local_day,
         float albedo,
         std::unique_ptr<Orbit> orbit,
         bool close_orbit,
         ObjL* _currentObj,
         double orbit_bounding_radius,
		 std::shared_ptr<BodyTexture> _bodyTexture):
	Body(parent,
	     englishName,
	     SUN,
	     flagHalo,
	     radius,
	     oblateness,
	     std::move(myColor),
	     _sol_local_day,
	     albedo,
	     std::move(orbit),
	     close_orbit,
	     _currentObj,
	     orbit_bounding_radius,
		_bodyTexture
        )
{
	//more adding could be placed here for the constructor of Sun
	createSunShader();
	createHaloShader(VulkanMgr::instance->getSwapChainExtent().height);
}

Sun::~Sun()
{
}


float Sun::computeMagnitude(Vec3d obs_pos) const
{
	float rval = 0;
	const double sq = obs_pos.lengthSquared();
	rval = -26.73f + 2.5f*log10f(sq);
	return rval;
}

void Sun::buildHaloCmd()
{
    Context &context = *Context::instance;
    descriptorSetBigHalo->bindTexture(tex_big_halo->getTexture(), 0);
    if (haloCmds[0] == VK_NULL_HANDLE) {
        context.cmdInfo.commandBufferCount = 3;
        vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, haloCmds);
    }
    for (int i = 0; i < 3; ++i) {
        VkCommandBuffer &cmd = haloCmds[i];
        context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
        pipelineBigHalo->bind(cmd);
        VertexArray::bind(cmd, haloBuffer->get());
        layoutBigHalo->bindSets(cmd, {*descriptorSetBigHalo->get(), *context.uboSet->get()});
        vkCmdDraw(cmd, 1, 1, 0, 0);
        context.frame[i]->compile(cmd);
    }
}

void Sun::setBigHalo(const std::string& halotexfile, const std::string &path)
{
	tex_big_halo = std::make_unique<s_texture>( path + halotexfile, TEX_LOAD_TYPE_PNG_SOLID);
    if (descriptorSetBigHalo) {
        buildHaloCmd();
    }
}

void Sun::createHaloShader(float viewport_y)
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

	m_bigHaloGL = std::make_unique<VertexArray>(vkmgr, sizeof(Vec2f));
    m_bigHaloGL->createBindingEntry(sizeof(Vec2f));
    m_bigHaloGL->addInput(VK_FORMAT_R32G32_SFLOAT);
    haloBuffer = m_bigHaloGL->createBuffer(0, 1, context.tinyMgr.get());
    screenPosF = static_cast<typeof(screenPosF)>(context.tinyMgr->getPtr(haloBuffer->get()));

    layoutBigHalo = std::make_unique<PipelineLayout>(vkmgr);
    layoutBigHalo->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 1); // Rmag
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2); // cmag
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 3); // radius
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 4); // color
    layoutBigHalo->buildLayout();
    layoutBigHalo->setGlobalPipelineLayout(context.layouts.front().get());
    layoutBigHalo->build();

    descriptorSetBigHalo = std::make_unique<Set>(vkmgr, *context.setMgr, layoutBigHalo.get());
    uRmag = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
    descriptorSetBigHalo->bindUniform(uRmag, 1);
    uCmag = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
    descriptorSetBigHalo->bindUniform(uCmag, 2);
    uRadius = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
    descriptorSetBigHalo->bindUniform(uRadius, 3);
    uColor = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
    descriptorSetBigHalo->bindUniform(uColor, 4);

    pipelineBigHalo = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layoutBigHalo.get());
    pipelineBigHalo->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    pipelineBigHalo->setDepthStencilMode(VK_FALSE, VK_FALSE);
    pipelineBigHalo->setBlendMode(BLEND_ADD);
    pipelineBigHalo->bindVertex(*m_bigHaloGL);
    pipelineBigHalo->bindShader("sun_big_halo.vert.spv");
    pipelineBigHalo->bindShader("sun_big_halo.geom.spv");
    pipelineBigHalo->bindShader("sun_big_halo.frag.spv");
    pipelineBigHalo->setSpecializedConstant(0, &viewport_y, sizeof(viewport_y));
    pipelineBigHalo->build();

    if (tex_big_halo) {
        buildHaloCmd();
    }
}

void Sun::drawBigHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	float screen_r = getOnScreenSize(prj, nav);
	float rmag = big_halo_size/2/sqrt(nav->getObserverHelioPos().length());
	float cmag = rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r*2) {
		cmag*=rmag/(screen_r*2);
		rmag = screen_r*2;
	}

	if (rmag<32) rmag = 32;

    *uColor = myColor->getHalo();
    *uCmag = cmag;
    *uRmag = rmag;
    *uRadius = getOnScreenSize(prj, nav);

    *screenPosF = Vec2f((float) screenPos[0], (float)screenPos[1]);

    Context::instance->frame[Context::instance->frameIdx]->toExecute(haloCmds[Context::instance->frameIdx], PASS_MULTISAMPLE_DEPTH);
}

void Sun::createSunShader()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
	myShader = SHADER_SUN;
    layoutSun = std::make_unique<PipelineLayout>(vkmgr);
    layoutSun->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // ModelViewMatrix
    auto tmp = PipelineLayout::DEFAULT_SAMPLER;
    tmp.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    layoutSun->setTextureLocation(1, &tmp);
    layoutSun->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 2); // clipping_fov
    layoutSun->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 3); // planetScaledRadius
    layoutSun->buildLayout();
    layoutSun->setGlobalPipelineLayout(context.layouts.front().get());
    layoutSun->build();

    pipelineSun = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layoutSun.get());
    pipelineSun->setBlendMode(BLEND_NONE);
    pipelineSun->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineSun->setCullMode(true);
    currentObj->bind(*pipelineSun);
    pipelineSun->removeVertexEntry(2);
    pipelineSun->bindShader("body_sun.vert.spv");
    pipelineSun->bindShader("body_sun.frag.spv");
    pipelineSunNoDepth = std::unique_ptr<Pipeline>(pipelineSun->clone("Body Sun noDepth"));
    pipelineSunNoDepth->setDepthStencilMode();
    pipelineSun->build("Body Sun");
}

// Draw the Sun and all the related infos : name, circle etc..
void Sun::computeDraw(const Projector* prj, const Navigator * nav)
{
	eye_sun = nav->getHelioToEyeMat() * v3fNull;

	mat = mat_local_to_parent;
	parent_mat = Mat4d::identity();

	// This removed totally the Body shaking bug!!!
	mat = nav->getHelioToEyeMat() * mat;
	parent_mat = nav->getHelioToEyeMat() * parent_mat;

	eye_planet = mat * v3fNull;

	lightDirection = eye_sun - eye_planet;
	sun_half_angle = atan(696000.0/AU/lightDirection.length());  // hard coded Sun radius!
	//	cout << sun_half_angle << " sun angle on " << englishName << endl;
	lightDirection.normalize();

	// Compute the 2D position and check if in the screen
	screen_sz = getOnScreenSize(prj, nav);

	float screen_size_with_halo = screen_sz;
	if (big_halo_size > screen_sz)
		screen_size_with_halo = big_halo_size;

	isVisible = prj->projectCustomCheck(v3fNull, screenPos, mat, (int)(screen_size_with_halo/2));

	visibilityFader = isVisible;

	// Do not draw anything else if was not visible
	// Draw the name, and the circle if it's not too close from the body it's turning around
	// this prevents name overlaping (ie for jupiter satellites)
	ang_dist = 300.f*atan(get_ecliptic_pos().length()/getEarthEquPos(nav).length())/prj->getFov();
}

bool Sun::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet, bool needClearDepthBuffer)
{
	bool drawn = false;

	//on ne dessine pas une planete sur laquel on se trouve
	if (!drawHomePlanet && observatory->isOnBody(shared_from_this())) {
		return drawn;
	}

	hints->drawHints(nav, prj);

	if (isVisible && tex_big_halo)
		drawBigHalo(nav, prj, eye);

	if (screen_sz > 1 && isVisible) {  // huge improvement in performance
        Context &context = *Context::instance;
        FrameMgr &frame = *context.frame[context.frameIdx];
        if (cmds[context.frameIdx] == -1) {
            cmds[context.frameIdx] = frame.create(1);
            frame.setName(cmds[context.frameIdx], englishName + " " + std::to_string(context.frameIdx));
        }
        VkCommandBuffer &cmd = frame.begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
        if (needClearDepthBuffer) {
            VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
            VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
            vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
        }
        if (screen_sz > 3) {
            context.helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
            Halo::nextDraw(cmd);
        }
        if (depthTest)
		      axis->drawAxis(cmd, prj, mat);
		drawBody(cmd, prj, nav, mat, screen_sz, depthTest);
        frame.compile(cmd);
        frame.toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
		drawn = true;
	}

	return drawn;
}

void Sun::defineSunSet()
{
    Context &context = *Context::instance;
    if (!uModelViewMatrix) {
        uModelViewMatrix = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
        uclipping_fov = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
        uPlanetScaledRadius = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
    }
    descriptorSetSun = std::make_unique<Set>(*VulkanMgr::instance, *context.setMgr, layoutSun.get(), -1, false, true);
    descriptorSetSun->bindUniform(uModelViewMatrix, 0);
    descriptorSetSun->bindTexture(tex_current->getTexture(), 1);
    descriptorSetSun->bindUniform(uclipping_fov, 2);
    descriptorSetSun->bindUniform(uPlanetScaledRadius, 3);
    bigSet.reset();
    changed = false;
}

Set &Sun::getSet(float screen_sz)
{
    if (screen_sz < 180)
        return *descriptorSetSun;
    if (changed)
        defineSunSet();
    auto tex0 = tex_current->getBigTexture();
    if (bigSet) {
        if (!tex0)
            bigSet.reset();
    } else {
        if (tex0) {
            bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, layoutSun.get(), -1, true, true);
            bigSet->bindUniform(uModelViewMatrix, 0);
            bigSet->bindTexture(*tex0, 1);
            bigSet->bindUniform(uclipping_fov, 2);
            bigSet->bindUniform(uPlanetScaledRadius, 3);
        }
    }
    return (bigSet && screen_sz > 512) ? *bigSet : *descriptorSetSun;
}

void Sun::drawBody(VkCommandBuffer &cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest)
{
    Context &context = *Context::instance;
    if (changed)
        defineSunSet();

    *uModelViewMatrix = mat.convert() * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));
    *uclipping_fov = prj->getClippingFov();
    *uPlanetScaledRadius = radius;

    if (depthTest)
        pipelineSun->bind(cmd);
    else
        pipelineSunNoDepth->bind(cmd);
    currentObj->bind(cmd);
    layoutSun->bindSets(cmd, {getSet(screen_sz), *context.uboSet});
	currentObj->draw(cmd, screen_sz);
}
