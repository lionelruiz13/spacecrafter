/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/
//! \file halo.hpp
//! \brief Draws a body's halo
//! \author Julien LAFILLE
//! \date april 2018

#ifndef HALO_HPP
#define HALO_HPP

#include "tools/fader.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include <vector>
#include <memory>

class Body;
class Navigator;
class Projector;
class ToneReproductor;
class s_texture;
class VertexArray;
class shaderProgram;

class Halo {
public:

	Halo()=delete;
	Halo(const Halo&)=delete;

	Halo(Body* body);

	void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	void computeHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	static bool setTexHaloMap(const std::string &texMap);

	static void deleteDefaultTexMap();

	static void createShader();
	static void deleteShader();

private:

	Body * body;

	static DataGL m_haloGL;
	static shaderProgram* shaderHalo;
	std::vector<float> vecHaloPos;
	std::vector<float> vecHaloTex;
	static s_texture * tex_halo;			// Little halo texture

	float cmag;
	float rmag;
};

#endif
