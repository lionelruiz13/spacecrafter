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

#include "bodyModule/body_sun.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"

#include "bodyModule/axis.hpp"
#include "bodyModule/hints.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "bodyModule/body_color.hpp"
#include "navModule/observer.hpp"

Sun::Sun(Body *parent,
         const std::string& englishName,
         bool flagHalo,
         double radius,
         double oblateness,
         BodyColor* myColor,
         float _sol_local_day,
         float albedo,
         Orbit *orbit,
         bool close_orbit,
         ObjL* _currentObj,
         double orbit_bounding_radius,
		 BodyTexture* _bodyTexture):
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
		_bodyTexture
	    )
{
	//more adding could be placed here for the constructor of Sun
	shaderSun = nullptr;
	tex_big_halo = nullptr;
	createSunShader();
	createHaloShader();
}

Sun::~Sun()
{
	if (tex_big_halo) delete tex_big_halo;
	tex_big_halo = nullptr;
	deleteHaloShader();
}

void Sun::deleteHaloShader()
{
	glDeleteBuffers(1, &BigHalo.tex);
	glDeleteBuffers(1, &BigHalo.pos);
	glDeleteVertexArrays(1, &BigHalo.vao);
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
}

void Sun::createHaloShader()
{
	shaderBigHalo = new shaderProgram();
	shaderBigHalo->init("sun_big_halo.vert","sun_big_halo.geom","sun_big_halo.frag");
	shaderBigHalo->setUniformLocation("Rmag");
	shaderBigHalo->setUniformLocation("cmag");
	shaderBigHalo->setUniformLocation("Center");
	shaderBigHalo->setUniformLocation("radius");
	shaderBigHalo->setUniformLocation("color");

	glGenVertexArrays(1,&BigHalo.vao);
	glBindVertexArray(BigHalo.vao);
	glGenBuffers(1,&BigHalo.pos);
	glEnableVertexAttribArray(0);
}


void Sun::drawBigHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	Vec2f screenPosF ((float) screenPos[0], (float)screenPos[1]);

	StateGL::BlendFunc(GL_ONE, GL_ONE);
	float screen_r = getOnScreenSize(prj, nav);
	float rmag = big_halo_size/2/sqrt(nav->getObserverHelioPos().length());
	float cmag = rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r*2) {
		cmag*=rmag/(screen_r*2);
		rmag = screen_r*2;
	}

	if (rmag<32) rmag = 32;

	shaderBigHalo->use();
	glBindTexture(GL_TEXTURE_2D, tex_big_halo->getID());
	StateGL::enable(GL_BLEND);
	shaderBigHalo->setUniform("color", myColor->getHalo());
	shaderBigHalo->setUniform("cmag", cmag);
	shaderBigHalo->setUniform("Rmag", rmag);
	shaderBigHalo->setUniform("radius", getOnScreenSize(prj, nav));
	glBindVertexArray(BigHalo.vao);

	glBindBuffer (GL_ARRAY_BUFFER, BigHalo.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*2,screenPosF,GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_POINTS,0,1);

	shaderBigHalo->unuse();
}

void Sun::createSunShader()
{
	myShader = SHADER_SUN;
	shaderSun = new shaderProgram();
	shaderSun->init( "body_sun.vert", "body_sun.frag");
	shaderSun->setUniformLocation("ModelViewProjectionMatrix");

	//fisheye
	shaderSun->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderSun->setUniformLocation("ModelViewMatrix");
	shaderSun->setUniformLocation("planetScaledRadius");

	myShaderProg = shaderSun;
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

bool Sun::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet, bool selected)
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
		axis->drawAxis(prj, mat);
		drawBody(prj, nav, mat, screen_sz);
		drawn = true;
	}

	return drawn;
}


void Sun::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	StateGL::enable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, tex_current->getID());

	myShaderProg->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_current->getID());

	//paramétrage des matrices pour opengl4
	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(C_PI/180*(axis_rotation + 90));


	myShaderProg->setUniform("ModelViewProjectionMatrix",proj*matrix);
	myShaderProg->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("planetScaledRadius",radius);

	currentObj->draw(screen_sz);

	myShaderProg->unuse();
	glActiveTexture(GL_TEXTURE0);
	StateGL::disable(GL_CULL_FACE);
}

