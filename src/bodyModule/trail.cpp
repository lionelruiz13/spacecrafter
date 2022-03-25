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

#include "tools/context.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"

std::unique_ptr<VertexArray> Trail::m_dataGL;
Pipeline *Trail::pipeline;
PipelineLayout *Trail::layout;

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
}

Trail::~Trail()
{
	trail.clear();
}

bool Trail::doDraw(const Navigator * nav, const Projector* prj)
{
    return (trail_fader.getInterstate() && trail.size() >= 2);
}

void Trail::drawTrail(VkCommandBuffer &cmd, const Navigator * nav, const Projector* prj)
{
    if (!doDraw(nav, prj))
        return;

    Context &context = *Context::instance;
    if (insertCount) {
        vertexOffset -= insertCount;
        if (!vertex) {
            vertex = m_dataGL->createBuffer(0, MaxTrail + TRAIL_OPTIMIZE_TRANSFER, context.globalBuffer.get());
            vertexOffset = MaxTrail + TRAIL_OPTIMIZE_TRANSFER - trail.size();
        }

        if (vertexOffset >= 0) {
            Vec3f *data = (Vec3f *) context.transfer->planCopy(vertex->get(), vertexOffset * m_dataGL->alignment, insertCount * m_dataGL->alignment);
            auto it = trail.begin();
            for (int i = 0; i < insertCount; ++i) {
                data[i] = it->point;
                ++it;
            }
        } else {
            vertexOffset = MaxTrail + TRAIL_OPTIMIZE_TRANSFER - trail.size();
            Vec3f *data = (Vec3f *) context.transfer->planCopy(vertex->get(), vertexOffset * m_dataGL->alignment, trail.size() * m_dataGL->alignment);
            for (auto &v : trail)
                *(data++) = v.point;
        }
        insertCount = 0;
    }

    // But this is the first segment, not the last one...
	// // draw final segment to finish at current Body position
	// if (!first_point) {
	// 	insert_vec3(vecTrailPos, body->getEarthEquPos(nav));
	// 	vecTrailIntensity.push_back(1.0);
	// }

	// for (iter=begin; iter != trail.end(); iter++) {
	// 	segment++;
	// 	insert_vec3(vecTrailPos, (*iter).point);
	// 	vecTrailIntensity.push_back( segment);
	// }

	int nbPos = trail.size();
	if (nbPos >= 2) {
        pipeline->bind(cmd);
        const VkDeviceSize offset = vertex->get().offset + vertexOffset * m_dataGL->alignment;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertex->get().buffer, &offset);
        auto tmp1 = body->myColor->getTrail();
        layout->pushConstant(cmd, 0, &tmp1);
        struct {
            int vertexCount;
            Mat4f modelViewMatrix;
            float fader;
        } cstData {nbPos, prj->getMatEarthEquToEye(), trail_fader.getInterstate()};
        layout->pushConstant(cmd, 1, &cstData);
        layout->bindSet(cmd, *context.uboSet);
        vkCmdDraw(cmd, trail.size(), 1, 0, 0);
	}
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
        insertCount = 0;
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
        ++insertCount;

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

void Trail::createSC_context()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    m_dataGL = std::make_unique<VertexArray>(vkmgr, 3*sizeof(float));
    m_dataGL->createBindingEntry(3*sizeof(float));
    m_dataGL->addInput(VK_FORMAT_R32G32B32_SFLOAT);

    layout = new PipelineLayout(vkmgr);
    context.layouts.emplace_back(layout);
    layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vec3f));
    layout->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Vec3f), sizeof(int) + sizeof(Mat4f) + sizeof(float));
    layout->setGlobalPipelineLayout(context.layouts.front().get());
    layout->build();

    pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout);
    context.pipelines.emplace_back(pipeline);
    pipeline->setDepthStencilMode();
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
    pipeline->bindVertex(*m_dataGL);
    pipeline->bindShader("body_trail.vert.spv");
    pipeline->bindShader("body_trail.geom.spv");
    pipeline->bindShader("body_trail.frag.spv");
    pipeline->build();
}
