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
	static void beginDraw();
	static void endDraw();

private:

	Body * body;

	std::unique_ptr<VertexArray> vertex;
	float *pHaloData;
	struct {
		Vec3f Color;
		float cmag;
	} uData;
	static int commandIndex;
	static VirtualSurface *surface;
	static Set *globalSet, *set;
	static ThreadedCommandBuilder *cmdMgr;
	static CommandMgr *cmdMgrTarget;
	static TextureMgr *texMgr;
	static Pipeline *pipeline;
	static PipelineLayout *layout;
	static VertexArray *m_haloGL;
	static s_texture *last_tex_halo;
	static s_texture *tex_halo;			// Little halo texture

	float cmag;
	float rmag;
};

#endif
