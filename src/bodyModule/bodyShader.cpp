/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#include <iostream>
#include "bodyModule/bodyShader.hpp"


std::unique_ptr<shaderProgram> BodyShader::shaderBump;
std::unique_ptr<shaderProgram> BodyShader::shaderNight;
std::unique_ptr<shaderProgram> BodyShader::myEarth;
std::unique_ptr<shaderProgram> BodyShader::shaderRinged;
std::unique_ptr<shaderProgram> BodyShader::shaderNormal;
std::unique_ptr<shaderProgram> BodyShader::shaderNormalTes;
// std::unique_ptr<shaderProgram> BodyShader::shaderMoonNight;
// std::unique_ptr<shaderProgram> BodyShader::shaderMoonNormal;
// std::unique_ptr<shaderProgram> BodyShader::shaderMoonBump;
std::unique_ptr<shaderProgram> BodyShader::myMoon;

std::unique_ptr<shaderProgram> BodyShader::shaderArtificial;


void BodyShader::createShader()
{
	shaderNight = std::make_unique<shaderProgram>();
	shaderNight->init( "body_night.vert", "body_night.frag");
	// shaderNight->setUniformLocation("Clouds");
	// shaderNight->setUniformLocation("CloudNormalTexture");
	// shaderNight->setUniformLocation("CloudTexture");

	//commum
	shaderNight->setUniformLocation("planetRadius");
	shaderNight->setUniformLocation("planetScaledRadius");
	shaderNight->setUniformLocation("planetOneMinusOblateness");

	shaderNight->setUniformLocation("SunHalfAngle");
	shaderNight->setUniformLocation("LightPosition");
	shaderNight->setUniformLocation("ModelViewProjectionMatrix");
	shaderNight->setUniformLocation("ModelViewMatrix");
	shaderNight->setUniformLocation("NormalMatrix");
	shaderNight->setUniformLocation("ViewProjection");
	shaderNight->setUniformLocation("Model");

	shaderNight->setUniformLocation("MoonPosition1");
	shaderNight->setUniformLocation("MoonRadius1");
	shaderNight->setUniformLocation("MoonPosition2");
	shaderNight->setUniformLocation("MoonRadius2");
	shaderNight->setUniformLocation("MoonPosition3");
	shaderNight->setUniformLocation("MoonRadius3");
	shaderNight->setUniformLocation("MoonPosition4");
	shaderNight->setUniformLocation("MoonRadius4");

	shaderNight->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderNight->setUniformLocation("clipping_fov");

	// shaderMoonNight = std::make_unique<shaderProgram>();
	// shaderMoonNight->init( "body_moon_night.vert", "body_moon_night.frag");
	// // shaderMoonNight->setUniformLocation("Clouds");
	// // shaderMoonNight->setUniformLocation("CloudNormalTexture");
	// // shaderMoonNight->setUniformLocation("CloudTexture");

	// //commum
	// shaderMoonNight->setUniformLocation("planetRadius");
	// shaderMoonNight->setUniformLocation("planetScaledRadius");
	// shaderMoonNight->setUniformLocation("planetOneMinusOblateness");

	// shaderMoonNight->setUniformLocation("SunHalfAngle");
	// shaderMoonNight->setUniformLocation("LightPosition");
	// shaderMoonNight->setUniformLocation("ModelViewProjectionMatrix");
	// shaderMoonNight->setUniformLocation("ModelViewMatrix");
	// shaderMoonNight->setUniformLocation("NormalMatrix");
	// shaderMoonNight->setUniformLocation("ViewProjection");
	// shaderMoonNight->setUniformLocation("Model");

	// shaderMoonNight->setUniformLocation("MoonPosition1");
	// shaderMoonNight->setUniformLocation("MoonRadius1");

	// shaderMoonNight->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderMoonNight->setUniformLocation("clipping_fov");

	myEarth= std::make_unique<shaderProgram>();
	myEarth->init( "my_earth.vert", "my_earth.tesc","my_earth.tese", "my_earth.geom", "my_earth.frag");
	// myEarth->setUniformLocation("Clouds");
	// myEarth->setUniformLocation("CloudNormalTexture");
	// myEarth->setUniformLocation("CloudTexture");
	myEarth->setUniformLocation("TesParam");

	//commum
	myEarth->setUniformLocation("planetRadius");
	myEarth->setUniformLocation("planetScaledRadius");
	myEarth->setUniformLocation("planetOneMinusOblateness");

	myEarth->setUniformLocation("SunHalfAngle");
	myEarth->setUniformLocation("LightPosition");
	myEarth->setUniformLocation("ModelViewProjectionMatrix");
	myEarth->setUniformLocation("ModelViewMatrix");
	myEarth->setUniformLocation("NormalMatrix");
	myEarth->setUniformLocation("ViewProjection");
	myEarth->setUniformLocation("Model");

	myEarth->setUniformLocation("MoonPosition1");
	myEarth->setUniformLocation("MoonRadius1");
	myEarth->setUniformLocation("MoonPosition2");
	myEarth->setUniformLocation("MoonRadius2");
	myEarth->setUniformLocation("MoonPosition3");
	myEarth->setUniformLocation("MoonRadius3");
	myEarth->setUniformLocation("MoonPosition4");
	myEarth->setUniformLocation("MoonRadius4");

	//fisheye
	myEarth->setUniformLocation("inverseModelViewProjectionMatrix");
	myEarth->setUniformLocation("clipping_fov");

	shaderBump = std::make_unique<shaderProgram>();
	shaderBump->init( "body_bump.vert","", "","", "body_bump.frag");
	//shaderBump->setUniformLocation("UmbraColor");

	//commum
	shaderBump->setUniformLocation("planetRadius");
	shaderBump->setUniformLocation("planetScaledRadius");
	shaderBump->setUniformLocation("planetOneMinusOblateness");

	shaderBump->setUniformLocation("SunHalfAngle");
	shaderBump->setUniformLocation("LightPosition");
	shaderBump->setUniformLocation("ModelViewProjectionMatrix");
	shaderBump->setUniformLocation("ModelViewMatrix");
	shaderBump->setUniformLocation("NormalMatrix");

	shaderBump->setUniformLocation("MoonPosition1");
	shaderBump->setUniformLocation("MoonRadius1");
	shaderBump->setUniformLocation("MoonPosition2");
	shaderBump->setUniformLocation("MoonRadius2");
	shaderBump->setUniformLocation("MoonPosition3");
	shaderBump->setUniformLocation("MoonRadius3");
	shaderBump->setUniformLocation("MoonPosition4");
	shaderBump->setUniformLocation("MoonRadius4");

	//fisheye
	shaderBump->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderBump->setUniformLocation("clipping_fov");

	shaderRinged = std::make_unique<shaderProgram>();
	shaderRinged->init( "body_ringed.vert", "body_ringed.frag");
	shaderRinged->setUniformLocation("RingInnerRadius");
	shaderRinged->setUniformLocation("RingOuterRadius");
	//shaderRinged->setUniformLocation("UmbraColor");

	//commum
	shaderRinged->setUniformLocation("planetRadius");
	shaderRinged->setUniformLocation("planetScaledRadius");
	shaderRinged->setUniformLocation("planetOneMinusOblateness");

	shaderRinged->setUniformLocation("SunHalfAngle");
	shaderRinged->setUniformLocation("LightPosition");
	shaderRinged->setUniformLocation("ModelViewProjectionMatrix");
	shaderRinged->setUniformLocation("ModelViewMatrix");
	shaderRinged->setUniformLocation("ModelViewMatrixInverse");
	shaderRinged->setUniformLocation("NormalMatrix");

	shaderRinged->setUniformLocation("MoonPosition1");
	shaderRinged->setUniformLocation("MoonRadius1");
	shaderRinged->setUniformLocation("MoonPosition2");
	shaderRinged->setUniformLocation("MoonRadius2");
	shaderRinged->setUniformLocation("MoonPosition3");
	shaderRinged->setUniformLocation("MoonRadius3");
	shaderRinged->setUniformLocation("MoonPosition4");
	shaderRinged->setUniformLocation("MoonRadius4");

	//fisheye
	shaderRinged->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderRinged->setUniformLocation("clipping_fov");

	shaderNormal = std::make_unique<shaderProgram>();
	shaderNormal->init("body_normal.vert", "body_normal.frag");
	//shaderNormal->setUniformLocation("UmbraColor");

	//commum
	shaderNormal->setUniformLocation("planetRadius");
	shaderNormal->setUniformLocation("planetScaledRadius");
	shaderNormal->setUniformLocation("planetOneMinusOblateness");

	shaderNormal->setUniformLocation("SunHalfAngle");
	shaderNormal->setUniformLocation("LightPosition");
	// shaderNormal->setUniformLocation("ModelViewProjectionMatrix");
	shaderNormal->setUniformLocation("ModelViewMatrix");
	shaderNormal->setUniformLocation("NormalMatrix");

	shaderNormal->setUniformLocation("MoonPosition1");
	shaderNormal->setUniformLocation("MoonRadius1");
	shaderNormal->setUniformLocation("MoonPosition2");
	shaderNormal->setUniformLocation("MoonRadius2");
	shaderNormal->setUniformLocation("MoonPosition3");
	shaderNormal->setUniformLocation("MoonRadius3");
	shaderNormal->setUniformLocation("MoonPosition4");
	shaderNormal->setUniformLocation("MoonRadius4");

	//fisheye
	// shaderNormal->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderNormal->setUniformLocation("clipping_fov");

	shaderNormalTes = std::make_unique<shaderProgram>();
	shaderNormalTes->init( "body_normal_tes.vert", "body_normal_tes.tesc", "body_normal_tes.tese", "body_normal_tes.geom", "body_normal_tes.frag");
	//shaderNormalTes->setUniformLocation("UmbraColor");
	shaderNormalTes->setUniformLocation("TesParam");

	//commum
	shaderNormalTes->setUniformLocation("planetRadius");
	shaderNormalTes->setUniformLocation("planetScaledRadius");
	shaderNormalTes->setUniformLocation("planetOneMinusOblateness");

	shaderNormalTes->setUniformLocation("SunHalfAngle");
	shaderNormalTes->setUniformLocation("LightPosition");
	shaderNormalTes->setUniformLocation("ModelViewProjectionMatrix");
	shaderNormalTes->setUniformLocation("ModelViewMatrix");
	shaderNormalTes->setUniformLocation("NormalMatrix");
	shaderNormalTes->setUniformLocation("ViewProjection");
	shaderNormalTes->setUniformLocation("Model");

	shaderNormalTes->setUniformLocation("MoonPosition1");
	shaderNormalTes->setUniformLocation("MoonRadius1");
	shaderNormalTes->setUniformLocation("MoonPosition2");
	shaderNormalTes->setUniformLocation("MoonRadius2");
	shaderNormalTes->setUniformLocation("MoonPosition3");
	shaderNormalTes->setUniformLocation("MoonRadius3");
	shaderNormalTes->setUniformLocation("MoonPosition4");
	shaderNormalTes->setUniformLocation("MoonRadius4");

	//fisheye
	shaderNormalTes->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderNormalTes->setUniformLocation("clipping_fov");



	shaderArtificial = std::make_unique<shaderProgram>();
	shaderArtificial->init( "body_artificial.vert", "body_artificial.geom", "body_artificial.frag");

	//commum
	shaderArtificial->setUniformLocation("radius");
	shaderArtificial->setUniformLocation("useTexture");
	shaderArtificial->setUniformLocation("ProjectionMatrix");
	shaderArtificial->setUniformLocation("ModelViewMatrix");
	shaderArtificial->setUniformLocation("NormalMatrix");
	shaderArtificial->setUniformLocation("MVP");

	shaderArtificial->setUniformLocation("Light.Position");
	shaderArtificial->setUniformLocation("Light.Intensity");
	shaderArtificial->setUniformLocation("Material.Ka");
	shaderArtificial->setUniformLocation("Material.Kd");
	shaderArtificial->setUniformLocation("Material.Ks");
	shaderArtificial->setUniformLocation("Material.Ns");

	//fisheye
	shaderArtificial->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderArtificial->setUniformLocation("clipping_fov");


	// shaderMoonBump = std::make_unique<shaderProgram>();
	// shaderMoonBump->init( "moon_bump.vert","", "","", "moon_bump.frag");
	// shaderMoonBump->setUniformLocation("UmbraColor");
	// //commum
	// shaderMoonBump->setUniformLocation("planetRadius");
	// shaderMoonBump->setUniformLocation("planetScaledRadius");
	// shaderMoonBump->setUniformLocation("planetOneMinusOblateness");

	// shaderMoonBump->setUniformLocation("SunHalfAngle");
	// shaderMoonBump->setUniformLocation("LightPosition");
	// shaderMoonBump->setUniformLocation("ModelViewProjectionMatrix");
	// shaderMoonBump->setUniformLocation("ModelViewMatrix");
	// shaderMoonBump->setUniformLocation("NormalMatrix");

	// shaderMoonBump->setUniformLocation("MoonPosition1");
	// shaderMoonBump->setUniformLocation("MoonRadius1");

	// //fisheye
	// shaderMoonBump->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderMoonBump->setUniformLocation("clipping_fov");



	myMoon = std::make_unique<shaderProgram>();
	myMoon->init( "my_moon.vert","my_moon.tesc", "my_moon.tese","my_moon.geom", "my_moon.frag");
	myMoon->setUniformLocation("UmbraColor");
	myMoon->setUniformLocation("TesParam");

	//commum
	myMoon->setUniformLocation("planetRadius");
	myMoon->setUniformLocation("planetScaledRadius");
	myMoon->setUniformLocation("planetOneMinusOblateness");

	myMoon->setUniformLocation("SunHalfAngle");
	myMoon->setUniformLocation("LightPosition");
	myMoon->setUniformLocation("ModelViewProjectionMatrix");
	myMoon->setUniformLocation("ModelViewMatrix");
	myMoon->setUniformLocation("NormalMatrix");

	myMoon->setUniformLocation("MoonPosition1");
	myMoon->setUniformLocation("MoonRadius1");

	//fisheye
	myMoon->setUniformLocation("inverseModelViewProjectionMatrix");
	myMoon->setUniformLocation("clipping_fov");


	// shaderMoonNormal = std::make_unique<shaderProgram>();
	// shaderMoonNormal->init( "moon_normal.vert", "moon_normal.frag");
	// //shaderMoonNormal->setUniformLocation("UmbraColor");

	// //commum
	// shaderMoonNormal->setUniformLocation("planetRadius");
	// shaderMoonNormal->setUniformLocation("planetScaledRadius");
	// shaderMoonNormal->setUniformLocation("planetOneMinusOblateness");

	// shaderMoonNormal->setUniformLocation("SunHalfAngle");
	// shaderMoonNormal->setUniformLocation("LightPosition");
	// shaderMoonNormal->setUniformLocation("ModelViewProjectionMatrix");
	// shaderMoonNormal->setUniformLocation("ModelViewMatrix");
	// shaderMoonNormal->setUniformLocation("NormalMatrix");

	// shaderMoonNormal->setUniformLocation("MoonPosition1");
	// shaderMoonNormal->setUniformLocation("MoonRadius1");

	// //fisheye
	// shaderMoonNormal->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderMoonNormal->setUniformLocation("clipping_fov");
}


// void BodyShader::deleteShader()
// {
// 	if (shaderNight) delete shaderNight;
// 	if (shaderMoonNight) delete shaderMoonNight;
// 	if (myEarth) delete myEarth;
// 	if (shaderBump) delete shaderBump;
// 	if (shaderRinged) delete shaderRinged;
// 	if (shaderNormal) delete shaderNormal;

// 	if (shaderArtificial) delete shaderArtificial;
// 	if (shaderMoonBump) delete shaderMoonBump;
// 	if (myMoon) delete myMoon;
// 	if (shaderMoonNormal) delete shaderMoonNormal;
// }
