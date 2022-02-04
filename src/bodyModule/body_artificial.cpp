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
#include "bodyModule/body_color.hpp"

#include "bodyModule/axis.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/hints.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "ojmModule/ojm.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

Artificial::Artificial(std::shared_ptr<Body> parent,
                       const std::string& englishName,
                       bool flagHalo,
                       double radius,
                       std::unique_ptr<BodyColor> _myColor,
                       float _sol_local_day,
                       float albedo,
                       std::unique_ptr<Orbit> orbit,
                       bool close_orbit,
                       const std::string& model_name,
                       bool _deleteable,
                       double orbit_bounding_radius,
					   std::shared_ptr<BodyTexture> _bodyTexture
                      ):
	Body(parent,
	     englishName,
	     BODY_TYPE::ARTIFICIAL,
	     flagHalo,
	     radius,
	     1.0,
	     std::move(_myColor),
	     _sol_local_day,
	     albedo,
	     std::move(orbit),
	     close_orbit,
	     nullptr,
	     orbit_bounding_radius,
	     _bodyTexture)
{
	selectShader();
	obj3D = std::make_unique<Ojm>(AppSettings::Instance()->getModel3DDir() + model_name+"/" + model_name+".ojm", AppSettings::Instance()->getModel3DDir() + model_name+"/", radius);
	if (!obj3D -> getOk())
		std::cout << "Error with " << englishName << " " << model_name << std::endl;
	orbitPlot = std::make_unique<Orbit2D>(this);

}

Artificial::~Artificial()
{
	//if (obj3D) delete obj3D;
}

void Artificial::selectShader ()
{
	myShader = SHADER_ARTIFICIAL;
	drawState = BodyShader::getShaderArtificial();
}

void Artificial::createSC_context()
{
    if (initialized)
        return;
    auto &vkmgr = *VulkanMgr::instance;
    auto &context = *Context::instance;
    set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, 2, false, true);
    uNormalMatrix = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
    uProj = std::make_unique<SharedBuffer<artGeom>>(*context.uniformMgr);
    uLight = std::make_unique<SharedBuffer<LightInfo>>(*context.uniformMgr);
    set->bindUniform(uNormalMatrix, 0);
    set->bindUniform(uProj, 1);
    set->bindUniform(uLight, 2);
    initialized = true;
}

void Artificial::drawBody(VkCommandBuffer &cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
    createSC_context();

    auto &context = *Context::instance;

    Mat4f matrix = mat.convert() * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));
    *uNormalMatrix = matrix.inverse().transpose();
    uLight->get().Intensity =Vec3f(1.0, 1.0, 1.0);
    drawState->layout->bindSet(cmd, *context.uboSet);
    drawState->layout->bindSet(cmd, *set, 2);
    obj3D->record(cmd, drawState->pipeline, drawState->layout);
    uProj->get().ModelViewMatrix = matrix;
    uProj->get().clipping_fov = prj->getClippingFov();
    uLight->get().Position = eye_sun;
}
