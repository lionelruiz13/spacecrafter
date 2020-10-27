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

#include "tools/vecmath.hpp"
#include <vector>
#include <memory>
#include "vulkanModule/Context.hpp"

class Body;
class Navigator;
class Projector;
class ToneReproductor;
class s_texture;
class VertexArray;
class PipelineLayout;
class Pipeline;
class Uniform;
class Texture;

class Halo {
public:

	Halo()=delete;
	Halo(const Halo&)=delete;

	Halo(Body* body);

	void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	void computeHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	static bool setTexHaloMap(const std::string &texMap);

	static void deleteDefaultTexMap();

	static void createSC_context(ThreadContext *context);

private:
	void build();

	Body * body;

	std::unique_ptr<VertexArray> vertex;
	std::unique_ptr<Set> set;
	std::unique_ptr<Uniform> uniform;
	struct {
		Vec3f Color;
		float cmag;
	} *uData;
	int commandIndex;
	static VirtualSurface *surface;
	static Set *globalSet;
	static SetMgr *setMgr;
	static CommandMgr *cmdMgr, *cmdMgrTarget;
	static TextureMgr *texMgr;
	static Pipeline *pipeline;
	static PipelineLayout *layout;
	static VertexArray *m_haloGL;
	std::vector<float> vecHaloPos;
	std::vector<float> vecHaloTex;
	s_texture *last_tex_halo = nullptr;
	static s_texture *tex_halo;			// Little halo texture

	float cmag;
	float rmag;
};

#endif
