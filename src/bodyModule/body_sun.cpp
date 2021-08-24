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


#include "vulkanModule/VertexArray.hpp"

#include "bodyModule/axis.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/halo.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "bodyModule/body_color.hpp"
#include "navModule/observer.hpp"
#include "tools/sc_const.hpp"
#include "tools/s_font.hpp"

#include "vulkanModule/Vulkan.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Texture.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"
#include "vulkanModule/Set.hpp"

Sun::Sun(Body *parent,
         const std::string& englishName,
         bool flagHalo,
         double radius,
         double oblateness,
         std::shared_ptr<BodyColor> myColor,
         float _sol_local_day,
         float albedo,
         std::shared_ptr<Orbit> orbit,
         bool close_orbit,
         ObjL* _currentObj,
         double orbit_bounding_radius,
		 BodyTexture* _bodyTexture,
         ThreadContext *context):
	Body(parent,
	     englishName,
	     SUN,
	     flagHalo,
	     radius,
	     oblateness,
	     myColor,
	     _sol_local_day,
	     albedo,
	     orbit,
	     close_orbit,
	     _currentObj,
	     orbit_bounding_radius,
		_bodyTexture,
	    context
        )
{
	//more adding could be placed here for the constructor of Sun
	tex_big_halo = nullptr;
	createSunShader(context);
	createHaloShader(context, context->global->vulkan->getSwapChainExtent().height);
}

Sun::~Sun()
{
	if (tex_big_halo) delete tex_big_halo;
	tex_big_halo = nullptr;
}


float Sun::computeMagnitude(Vec3d obs_pos) const
{
	float rval = 0;
	const double sq = obs_pos.lengthSquared();
	rval = -26.73f + 2.5f*log10f(sq);
	return rval;
}

void Sun::setBigHalo(const std::string& halotexfile, const std::string &path)
{
	tex_big_halo = new s_texture( path + halotexfile, TEX_LOAD_TYPE_PNG_SOLID);
    if (descriptorSetBigHalo) {
        descriptorSetBigHalo->bindTexture(tex_big_halo->getTexture(), 0);

        commandIndexBigHalo = cmdMgr->getCommandIndex();
        cmdMgr->init(commandIndexBigHalo);
        cmdMgr->beginRenderPass(renderPassType::DEFAULT);
        cmdMgr->bindPipeline(pipelineBigHalo.get());
        cmdMgr->bindVertex(m_bigHaloGL.get());
        cmdMgr->bindSet(layoutBigHalo.get(), descriptorSetBigHalo.get());
        cmdMgr->bindSet(layoutBigHalo.get(), context->global->globalSet, 1);
        cmdMgr->draw(1);
        cmdMgr->compile();
    }
}

void Sun::createHaloShader(ThreadContext *context, float viewport_y)
{
    cmdMgr = context->commandMgr;
	// shaderBigHalo = std::make_unique<shaderProgram>();
	// shaderBigHalo->init("sun_big_halo.vert","sun_big_halo.geom","sun_big_halo.frag");
	// shaderBigHalo->setUniformLocation("Rmag");
	// shaderBigHalo->setUniformLocation("cmag");
	// shaderBigHalo->setUniformLocation("Center");
	// shaderBigHalo->setUniformLocation("radius");
	// shaderBigHalo->setUniformLocation("color");

	m_bigHaloGL = std::make_unique<VertexArray>(context->surface, context->commandMgr);
	m_bigHaloGL ->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
    m_bigHaloGL->build(1);

    layoutBigHalo = std::make_unique<PipelineLayout>(context->surface);
    layoutBigHalo->setTextureLocation(0);
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 1); // Rmag
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2); // cmag
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 3); // radius
    layoutBigHalo->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 4); // color
    layoutBigHalo->buildLayout();
    layoutBigHalo->setGlobalPipelineLayout(context->global->globalLayout);
    layoutBigHalo->build();

    descriptorSetBigHalo = std::make_unique<Set>(context->surface, context->setMgr, layoutBigHalo.get());
    if (tex_big_halo)
        descriptorSetBigHalo->bindTexture(tex_big_halo->getTexture(), 0);
    uRmag = std::make_unique<Uniform>(context->surface, sizeof(*pRmag));
    pRmag = static_cast<typeof(pRmag)>(uRmag->data);
    descriptorSetBigHalo->bindUniform(uRmag.get(), 1);
    uCmag = std::make_unique<Uniform>(context->surface, sizeof(*pCmag));
    pCmag = static_cast<typeof(pCmag)>(uCmag->data);
    descriptorSetBigHalo->bindUniform(uCmag.get(), 2);
    uRadius = std::make_unique<Uniform>(context->surface, sizeof(*pRadius));
    pRadius = static_cast<typeof(pRadius)>(uRadius->data);
    descriptorSetBigHalo->bindUniform(uRadius.get(), 3);
    uColor = std::make_unique<Uniform>(context->surface, sizeof(*pColor));
    pColor = static_cast<typeof(pColor)>(uColor->data);
    descriptorSetBigHalo->bindUniform(uColor.get(), 4);

    pipelineBigHalo = std::make_unique<Pipeline>(context->surface, layoutBigHalo.get());
    pipelineBigHalo->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    pipelineBigHalo->setDepthStencilMode(VK_FALSE, VK_FALSE);
    pipelineBigHalo->setBlendMode(BLEND_ADD);
    pipelineBigHalo->bindVertex(m_bigHaloGL.get());
    pipelineBigHalo->bindShader("sun_big_halo.vert.spv");
    pipelineBigHalo->bindShader("sun_big_halo.geom.spv");
    pipelineBigHalo->bindShader("sun_big_halo.frag.spv");
    pipelineBigHalo->setSpecializedConstant(0, &viewport_y, sizeof(viewport_y));
    pipelineBigHalo->build();

    if (tex_big_halo) {
        commandIndexBigHalo = cmdMgr->getCommandIndex();
        cmdMgr->init(commandIndexBigHalo);
        cmdMgr->beginRenderPass(renderPassType::DEFAULT);
        cmdMgr->bindPipeline(pipelineBigHalo.get());
        cmdMgr->bindVertex(m_bigHaloGL.get());
        cmdMgr->bindSet(layoutBigHalo.get(), descriptorSetBigHalo.get());
        cmdMgr->bindSet(layoutBigHalo.get(), context->global->globalSet, 1);
        cmdMgr->draw(1);
        cmdMgr->compile();
    }
}


void Sun::drawBigHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	Vec2f screenPosF ((float) screenPos[0], (float)screenPos[1]);


	float screen_r = getOnScreenSize(prj, nav);
	float rmag = big_halo_size/2/sqrt(nav->getObserverHelioPos().length());
	float cmag = rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r*2) {
		cmag*=rmag/(screen_r*2);
		rmag = screen_r*2;
	}

	if (rmag<32) rmag = 32;

	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_ONE, GL_ONE);

	// shaderBigHalo->use();
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, tex_big_halo->getID());

	// shaderBigHalo->setUniform("color", myColor->getHalo());
	// shaderBigHalo->setUniform("cmag", cmag);
	// shaderBigHalo->setUniform("Rmag", rmag);
	// shaderBigHalo->setUniform("radius", getOnScreenSize(prj, nav));
    *pColor = myColor->getHalo();
    *pCmag = cmag;
    *pRmag = rmag;
    *pRadius = getOnScreenSize(prj, nav);

	m_bigHaloGL->fillVertexBuffer(BufferType::POS2D, 2, screenPosF );
    m_bigHaloGL->update();

	//Renderer::drawArrays(shaderBigHalo.get(), m_bigHaloGL.get(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, 1);
    cmdMgr->setSubmission(commandIndexBigHalo);
}

void Sun::createSunShader(ThreadContext *context)
{
	myShader = SHADER_SUN;
	// shaderSun = std::make_unique<shaderProgram>();
	// shaderSun->init( "body_sun.vert", "body_sun.frag");
	// shaderSun->setUniformLocation("ModelViewProjectionMatrix");
    //
	// //fisheye
	// shaderSun->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderSun->setUniformLocation("ModelViewMatrix");
	// shaderSun->setUniformLocation("planetScaledRadius");
    layoutSun = std::make_unique<PipelineLayout>(context->surface);
    layoutSun->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // ModelViewMatrix
    layoutSun->setTextureLocation(1);
    layoutSun->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 2); // clipping_fov
    layoutSun->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 3); // planetScaledRadius
    layoutSun->buildLayout();
    layoutSun->setGlobalPipelineLayout(context->global->globalLayout);
    layoutSun->build();

    pipelineSun = std::make_unique<Pipeline>(context->surface, layoutSun.get());
    pipelineSun->setBlendMode(BLEND_NONE);
    //pipelineSun->setDepthStencilMode();
    pipelineSun->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineSun->setCullMode(true);
    currentObj->bind(pipelineSun.get());
    pipelineSun->removeVertexEntry(2);
    pipelineSun->bindShader("body_sun.vert.spv");
    pipelineSun->bindShader("body_sun.frag.spv");
    pipelineSun->build();

    descriptorSetSun = std::make_unique<Set>(context->surface, context->setMgr, layoutSun.get());
    uModelViewMatrix = std::make_unique<Uniform>(context->surface, sizeof(*pModelViewMatrix));
    pModelViewMatrix = static_cast<typeof(pModelViewMatrix)>(uModelViewMatrix->data);
    descriptorSetSun->bindUniform(uModelViewMatrix.get(), 0);
    uclipping_fov = std::make_unique<Uniform>(context->surface, sizeof(*pclipping_fov));
    pclipping_fov = static_cast<typeof(pclipping_fov)>(uclipping_fov->data);
    descriptorSetSun->bindUniform(uclipping_fov.get(), 2);
    uPlanetScaledRadius = std::make_unique<Uniform>(context->surface, sizeof(*pPlanetScaledRadius));
    pPlanetScaledRadius = static_cast<typeof(pPlanetScaledRadius)>(uPlanetScaledRadius->data);
    descriptorSetSun->bindUniform(uPlanetScaledRadius.get(), 3);

    drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
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

bool Sun::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet)
{
	bool drawn = false;

	//on ne dessine pas une planete sur laquel on se trouve
	if (!drawHomePlanet && observatory->isOnBody(this)) {
		return drawn;
	}

	hints->drawHints(nav, prj);

	if (isVisible && tex_big_halo)
		drawBigHalo(nav, prj, eye);

	if (screen_sz > 1 && isVisible) {  // huge improvement in performance
        if (screen_sz > 3) {
            s_font::nextPrint(true);
            Halo::nextDraw();
        }
		axis->drawAxis(prj, mat);
		drawBody(prj, nav, mat, screen_sz);
		drawn = true;
	}

	return drawn;
}


void Sun::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	// StateGL::enable(GL_CULL_FACE);
	// StateGL::disable(GL_BLEND);

	// shaderSun->use();

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, tex_current->getID());

    if (tex_current != last_tex_current) {
        descriptorSetSun->bindTexture(tex_current->getTexture(), 1);
        last_tex_current = tex_current;
    }

    if (commandIndexSun == -2) {
        commandIndexSun = -1;
        if (!cmdMgr->isRecording()) {
            commandIndexSun = cmdMgr->getCommandIndex();
            cmdMgr->init(commandIndexSun);
            cmdMgr->beginRenderPass(renderPassType::CLEAR_DEPTH_BUFFER_DONT_SAVE);
        }
        cmdMgr->bindPipeline(pipelineSun.get());
        currentObj->bind(cmdMgr);
        cmdMgr->bindSet(layoutSun.get(), descriptorSetSun.get());
        cmdMgr->bindSet(layoutSun.get(), context->global->globalSet, 1);
        cmdMgr->indirectDrawIndexed(drawData.get());

        cmdMgr->compile();
        return;
    } else if (commandIndexSun >= 0) {
        cmdMgr->setSubmission(commandIndexSun);
    }

	//paramétrage des matrices pour opengl4
	// Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));

	// shaderSun->setUniform("ModelViewProjectionMatrix",proj*matrix);
	// shaderSun->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	// shaderSun->setUniform("ModelViewMatrix",matrix);
	// shaderSun->setUniform("planetScaledRadius",radius);
    *pModelViewMatrix = matrix;
    *pclipping_fov = prj->getClippingFov();
    *pPlanetScaledRadius = radius;

	currentObj->draw(screen_sz, drawData->data);
    drawData->update();

	// shaderSun->unuse();
	// glActiveTexture(GL_TEXTURE0);
	// StateGL::disable(GL_CULL_FACE);
}
