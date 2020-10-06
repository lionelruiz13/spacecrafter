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

#include "bodyModule/trail.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/time_mgr.hpp"
#include "bodyModule/body.hpp"
#include "bodyModule/body_color.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/ResourceTracker.hpp"
#include "vulkanModule/Buffer.hpp"

CommandMgr *Trail::cmdMgr;
ThreadContext *Trail::context;
VertexArray *Trail::m_dataGL;
PipelineLayout *Trail::layout;
Pipeline *Trail::pipeline;

Trail::Trail(Body * _body,
             int _MaxTrail,
             double _DeltaTrail,
             double _last_trailJD,
             bool _trail_on,
             bool _first_point) :
	MaxTrail(_MaxTrail),
	DeltaTrail(_DeltaTrail),
	last_trailJD(_last_trailJD),
	trail_on(_trail_on),
	first_point(_first_point)
{
	body = _body;

    vertex = std::make_unique<VertexArray>(*m_dataGL);
    vertex->build(MaxTrail + 1);
    drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    *static_cast<VkDrawIndirectCommand *>(drawData->data) = (VkDrawIndirectCommand){0, 1, 0, 0};
    nbVertices = static_cast<typeof(nbVertices)>(drawData->data);

    set = std::make_unique<Set>(context->surface, context->setMgr, layout);
    uMat = std::make_unique<Uniform>(context->surface, sizeof(*pMat));
    pMat = static_cast<typeof(pMat)>(uMat->data);
    set->bindUniform(uMat.get(), 0);
    uColor = std::make_unique<Uniform>(context->surface, sizeof(*pColor));
    pColor = static_cast<typeof(pColor)>(uColor->data);
    set->bindUniform(uColor.get(), 0);
    uFader = std::make_unique<Uniform>(context->surface, sizeof(*pFader));
    pFader = static_cast<typeof(pFader)>(uFader->data);
    set->bindUniform(uFader.get(), 0);
    uNbPoints = std::make_unique<Uniform>(context->surface, sizeof(*pNbPoints));
    pNbPoints = static_cast<typeof(pNbPoints)>(uNbPoints->data);
    set->bindUniform(uNbPoints.get(), 0);

    commandIndex = cmdMgr->initNew(pipeline, renderPassType::DEFAULT);
    cmdMgr->bindSet(layout, set.get());
    cmdMgr->bindSet(layout, context->global->globalSet, 1);
    cmdMgr->bindVertex(vertex.get());
    cmdMgr->indirectDraw(drawData.get());
    cmdMgr->compile();
}


Trail::~Trail()
{
	vecTrailPos.clear();
	vecTrailIntensity.clear();
	trail.clear();
}

void Trail::drawTrail(const Navigator * nav, const Projector* prj)
{
	float fade = trail_fader.getInterstate();
	if (!fade)
		return;
	if (trail.empty())
		return;

	std::list<TrailPoint>::iterator iter;
	std::list<TrailPoint>::iterator begin = trail.begin();

	float segment = 0;

	// draw final segment to finish at current Body position
	if ( !first_point) {
		insert_vec3(vecTrailPos, body->getEarthEquPos(nav));
		vecTrailIntensity.push_back(1.0);
	}

	for (iter=begin; iter != trail.end(); iter++) {
		segment++;
		insert_vec3(vecTrailPos, (*iter).point);
		vecTrailIntensity.push_back( segment);
	}

	int nbPos = vecTrailPos.size()/3 ;
	if (nbPos >= 2) {

		// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// StateGL::enable(GL_BLEND);

		// shaderTrail->use();
		*pMat = prj->getMatEarthEquToEye();
		*pColor = body->myColor->getTrail();
		*pFader = fade;
		*pNbPoints = nbPos;

		vertex->fillVertexBuffer(BufferType::POS3D, vecTrailPos);
		vertex->fillVertexBuffer(BufferType::MAG, vecTrailIntensity);

        *nbVertices = nbPos;
        drawData->update();
        cmdMgr->setSubmission(commandIndex);

		// m_dataGL->bind();
		// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, nbPos);
		// m_dataGL->unBind();
		// shaderTrail->unuse();
		// Renderer::drawArrays(shaderTrail.get(), m_dataGL.get(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, nbPos);

		// StateGL::enable(GL_BLEND);
	}

	vecTrailPos.clear();
	vecTrailIntensity.clear();
}

// update trail points as needed
void Trail::updateTrail(const Navigator* nav, const TimeMgr* timeMgr)
{
	if (trail_fader.getInterstate()< 0.001)
		return;

	double date = timeMgr->getJulian();

	int dt=0;
	if (first_point || (dt=abs(int((date-last_trailJD)/DeltaTrail))) > MaxTrail) {
		dt=1;
		trail.clear();
		first_point = 0;
	}

	// Note that when jump by a week or day at a time, loose detail on trails
	// particularly for moon (if decide to show moon trail)
	// add only one point at a time, using current position only
	if (dt) {
		last_trailJD = date;
		TrailPoint tp;
		Vec3d v = body->get_heliocentric_ecliptic_pos();
		tp.point = nav->helioToEarthPosEqu(v);
		tp.date = date;
		trail.push_front( tp );

		if ( trail.size() > (unsigned int)MaxTrail ) {
			trail.pop_back();
		}
	}

	// because sampling depends on speed and frame rate, need to clear out
	// points if trail gets longer than desired
	std::list<TrailPoint>::iterator iter;
	std::list<TrailPoint>::iterator end = trail.end();

	for ( iter=trail.begin(); iter != end; iter++) {
		if ( fabs((*iter).date - date)/DeltaTrail > MaxTrail ) {
			trail.erase(iter, end);
			break;
		}
	}
}

void Trail::startTrail(bool b)
{
	if (b) {
		trail_on = true; // No trail for Sun or moons
		first_point = true;
	}
	else {
		trail_on = false;
	}
}

void Trail::updateFader(int delta_time)
{
	trail_fader.update(delta_time);
}

void Trail::createSC_context(ThreadContext *_context)
{
    context = _context;
    cmdMgr = context->commandMgr;
	// shaderTrail = std::make_unique<shaderProgram>();
	// shaderTrail->init( "body_trail.vert","body_trail.geom","body_trail.frag");
	// shaderTrail->setUniformLocation({"Mat", "Color", "fader", "nbPoints"});

	m_dataGL = context->global->tracker->track(new VertexArray(context->surface, cmdMgr));
	m_dataGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
	m_dataGL->registerVertexBuffer(BufferType::MAG, BufferAccess::DYNAMIC);

    layout = context->global->tracker->track(new PipelineLayout(context->surface));
    layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 2);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 3);
    layout->buildLayout();
    layout->setGlobalPipelineLayout(context->global->globalLayout);
    layout->build();

    pipeline = context->global->tracker->track(new Pipeline(context->surface, layout));
    pipeline->setDepthStencilMode();
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
    pipeline->bindVertex(m_dataGL);
    pipeline->bindShader("body_trail.vert.spv");
    pipeline->bindShader("body_trail.geom.spv");
    pipeline->bindShader("body_trail.frag.spv");
    pipeline->build();
}
