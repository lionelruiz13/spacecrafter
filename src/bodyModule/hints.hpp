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


#ifndef _HINTS_HPP_
#define _HINTS_HPP_

//! \file hints.hpp
//! \brief Draws a body's name and a circle arround it
//! \author Julien LAFILLE
//! \date april 2018

#include "tools/fader.hpp"

#include "tools/vecmath.hpp"
#include <vector>
#include <memory>
#include "EntityCore/Resource/PipelineLayout.hpp"

class Body;
class Navigator;
class Projector;
class VertexArray;
class Pipeline;

class Hints {
public :

	Hints() = delete;
	Hints(const Hints&)=delete;
	Hints(Body * body);

	void setFlagHints(bool b) {
		hint_fader = b;
	}

	void drawHints(const Navigator* nav, const Projector* prj);

	void drawHintCircle(const Navigator* nav, const Projector* prj);

	void updateShader(double delta_time);

	int computeHints(float *&data);

	static void createSC_context();

	static void bind(VkCommandBuffer cmd, const Vec4f &color);
	static inline void push(VkCommandBuffer cmd, const Vec4f &color) {
		layout->pushConstant(cmd, 0, &color);
	}
private :
	static const int nbrFacets;
	static const int hintCircleRadius;

	Body * body;

	static std::unique_ptr<VertexArray> m_HintsGL;
	static Pipeline *pipeline;
	static PipelineLayout *layout;
	LinearFader hint_fader;
	bool initialized = false;

	struct {
		unsigned char flag;
		Vec4f color;
		Hints *self;
	} drawData;
};

#endif // _HINTS_HPP_
