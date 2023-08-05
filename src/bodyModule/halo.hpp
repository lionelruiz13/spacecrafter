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

#include "tools/vecmath.hpp"
#include <vector>
#include <memory>
#include "EntityCore/SubBuffer.hpp"
#include "tools/fader.hpp"

constexpr int HALO_STRIDE = 6*sizeof(float);

class Body;
class Navigator;
class Projector;
class ToneReproductor;
class s_texture;
class VertexArray;
class VertexBuffer;
class PipelineLayout;
class Pipeline;
class Texture;
class Set;

class Halo {
	friend class Renderer; // For now, Renderer use internal globals
public:

	Halo()=delete;
	Halo(const Halo&)=delete;

	Halo(Body* body);

	void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);
	void drawHaloOverride(const Navigator* nav, const Projector* prj, const ToneReproductor* eye, float rmag, float cmag);

	void computeHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	static bool setTexHaloMap(const std::string &texMap);

	static void deleteDefaultTexMap();

	static void createSC_context();
	static void destroySC_context();

	//! Prepair drawing halos in a batch
	static void beginDraw();
	//! Process halo draws in the given CommandBuffer and prepair the next batch
	static void nextDraw(VkCommandBuffer cmd);
	//! Finalize and submit halo drawings
	static void endDraw();

private:
	Body * body;

	struct HaloContext {
		struct pData_t {
			std::pair<float, float> pos;
			Vec3f Color;
			float rmag;
		} *pData;
		std::unique_ptr<VertexArray> pattern;
		std::unique_ptr<VertexBuffer> vertex;
		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<PipelineLayout> layout;
		std::unique_ptr<s_texture> tex_halo;
		s_texture *last_tex_halo = nullptr;
		std::unique_ptr<Set> set;
		SubBuffer staging;
		int cmds[3] = {-1, -1, -1};
		int initialOffset = 0;
		int offset = 0; // vertex offset of the next packed draw
		int size = 0; // current vertex count of the next packed draw
	};
	static HaloContext *global;

	float cmag;
	float rmag;
};

#endif
