/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
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
#include "bodyModule/orbit_2d.hpp"
#include "tools/app_settings.hpp"
#include "bodyModule/body_artificial.hpp"

#include "bodyModule/axis.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/hints.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "ojmModule/ojm.hpp"

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"

Artificial::Artificial(Body *parent,
                       const std::string& englishName,
                       bool flagHalo,
                       double radius,
                       std::shared_ptr<BodyColor> _myColor,
                       float _sol_local_day,
                       float albedo,
                       std::shared_ptr<Orbit> orbit,
                       bool close_orbit,
                       const std::string& model_name,
                       bool _deleteable,
                       double orbit_bounding_radius,
					   std::shared_ptr<BodyTexture> _bodyTexture,
                       ThreadContext *context
                      ):
	Body(parent,
	     englishName,
	     BODY_TYPE::ARTIFICIAL,
	     flagHalo,
	     radius,
	     1.0,
	     _myColor,
	     _sol_local_day,
	     albedo,
	     orbit,
	     close_orbit,
	     nullptr,
	     orbit_bounding_radius,
	     _bodyTexture,
         context)
{
	selectShader();
    createSC_context(context);
	obj3D = new Ojm(AppSettings::Instance()->getModel3DDir() + model_name+"/" + model_name+".ojm", AppSettings::Instance()->getModel3DDir() + model_name+"/", radius, context->surface);
	if (!obj3D -> getOk())
		std::cout << "Error with " << englishName << " " << model_name << std::endl;
	orbitPlot = new Orbit2D(this);

}

Artificial::~Artificial()
{
	if (obj3D) delete obj3D;
}

void Artificial::selectShader ()
{
	myShader = SHADER_ARTIFICIAL;
	drawState = BodyShader::getShaderArtificial();
    pushSet = BodyShader::getPushSetShaderArtificial();
}

void Artificial::createSC_context(ThreadContext *context)
{
    set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout, 2);
    uNormalMatrix = std::make_unique<Uniform>(context->surface, sizeof(*pNormalMatrix));
    pNormalMatrix = static_cast<typeof(pNormalMatrix)>(uNormalMatrix->data);
    set->bindUniform(uNormalMatrix.get(), 0);
    uProj = std::make_unique<Uniform>(context->surface, sizeof(*pProj));
    pProj = static_cast<typeof(pProj)>(uProj->data);
    set->bindUniform(uProj.get(), 1);
    uLight = std::make_unique<Uniform>(context->surface, sizeof(*pLight));
    pLight = static_cast<typeof(pLight)>(uLight->data);
    set->bindUniform(uLight.get(), 2);
}

void Artificial::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	//StateGL::enable(GL_CULL_FACE);
	//StateGL::disable(GL_BLEND);

    switch (commandIndex) {
        case -2: // Command not builded
            commandIndex = -1;
            if (!context->commandMgr->isRecording()) {
                commandIndex = context->commandMgr->getCommandIndex();
                context->commandMgr->init(commandIndex);
                context->commandMgr->beginRenderPass(renderPassType::CLEAR_DEPTH_BUFFER_DONT_SAVE);
            }
            pLight->Intensity =Vec3f(1.0, 1.0, 1.0);
            context->commandMgr->bindSet(drawState->layout, context->global->globalSet, 0);
            context->commandMgr->bindSet(drawState->layout, set.get(), 2);
            obj3D->record(context->commandMgr, drawState->pipeline, drawState->layout, pushSet);
            context->commandMgr->compile(); // There is no halo for body_artificial
            return;
        case -1: break;
        default:
            context->commandMgr->setSubmission(commandIndex);
    }

    Mat4f matrix = mat.convert();
    *pNormalMatrix = matrix.inverse().transpose();
    pProj->ModelViewMatrix = matrix;
    pProj->clipping_fov = prj->getClippingFov();
    pLight->Position = eye_sun;

	//paramètres commun aux shaders
	//myShaderProg->setUniform("NormalMatrix", inv_matrix.transpose());

	//myShaderProg->unuse();

	//StateGL::disable(GL_CULL_FACE);
}
