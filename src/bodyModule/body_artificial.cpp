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


Artificial::Artificial(Body *parent,
                       const std::string& englishName,
                       bool flagHalo,
                       double radius,
                       BodyColor* _myColor,
                       float _sol_local_day,
                       float albedo,
                       Orbit *orbit,
                       bool close_orbit,
                       const std::string& model_name,
                       bool _deleteable,
                       double orbit_bounding_radius,
					   BodyTexture* _bodyTexture
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
	     _bodyTexture)
{
	selectShader();
	obj3D = new Ojm(AppSettings::Instance()->getModel3DDir() + model_name+"/" + model_name+".ojm", AppSettings::Instance()->getModel3DDir() + model_name+"/", radius);
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
	myShaderProg = BodyShader::getShaderArtificial();
}


void Artificial::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	StateGL::enable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	myShaderProg->use();

	//paramétrage des matrices pour opengl4
	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	//matrix = matrix * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));

	Mat4f inv_matrix = matrix.inverse();
	myShaderProg->setUniform("MVP",proj*matrix);
	myShaderProg->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("clipping_fov",prj->getClippingFov());

	myShaderProg->setUniform("Light.Position", eye_sun);
	myShaderProg->setUniform("Light.Intensity", Vec3f(1.0, 1.0, 1.0));

	obj3D->draw(myShaderProg);

	//paramètres commun aux shaders
	myShaderProg->setUniform("NormalMatrix", inv_matrix.transpose());

	myShaderProg->unuse();

	StateGL::disable(GL_CULL_FACE);
}
