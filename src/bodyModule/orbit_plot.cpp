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
* (c) 2017 - 2020 all rights reserved
*
*/

#include <iostream>


#include "bodyModule/orbit_plot.hpp"
#include "bodyModule/body.hpp"
#include "bodyModule/orbit.hpp"

#include "tools/context.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"

std::unique_ptr<VertexArray> OrbitPlot::m_Orbit;
Pipeline *OrbitPlot::pipelineOrbit2d, *OrbitPlot::pipelineOrbit3d;
PipelineLayout *OrbitPlot::layoutOrbit2d, *OrbitPlot::layoutOrbit3d;

OrbitPlot::OrbitPlot(Body* _body, int segments, int nbAdditionnalPoints) : nbAdditionnalPoints(nbAdditionnalPoints)
{
	body = _body;
	ORBIT_POINTS = segments;
	orbitPoint = new Vec3d[ORBIT_POINTS];
}

OrbitPlot::~OrbitPlot()
{
	delete[] orbitPoint;
}

void OrbitPlot::initDraw()
{
	if (!orbit)
		orbit = m_Orbit->createBuffer(0, ORBIT_POINTS + nbAdditionnalPoints, Context::instance->globalBuffer.get());
}

void OrbitPlot::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	assert(!m_Orbit);
	m_Orbit = std::make_unique<VertexArray>(vkmgr, 3*sizeof(float));
	m_Orbit->createBindingEntry(3*sizeof(float));
	m_Orbit->addInput(VK_FORMAT_R32G32B32_SFLOAT);

	context.layouts.emplace_back(new PipelineLayout(vkmgr));
	layoutOrbit2d = context.layouts.back().get();
	layoutOrbit2d->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vec4f));
	layoutOrbit2d->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(Vec4f), sizeof(Mat4f) + sizeof(float));
	layoutOrbit2d->build();

	context.pipelines.emplace_back(new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layoutOrbit2d));
	pipelineOrbit2d = context.pipelines.back().get();
	pipelineOrbit2d->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipelineOrbit2d->setDepthStencilMode();
	pipelineOrbit2d->bindVertex(*m_Orbit);
	pipelineOrbit2d->bindShader("body_orbit2d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipelineOrbit2d->bindShader("body_orbit2d.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT);
	pipelineOrbit2d->bindShader("body_orbit2d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineOrbit2d->build();

	context.layouts.emplace_back(new PipelineLayout(vkmgr));
	layoutOrbit3d = context.layouts.back().get();
	layoutOrbit3d->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vec4f));
	layoutOrbit3d->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(Vec4f), sizeof(Mat4f) + sizeof(Vec3f));
	layoutOrbit3d->build();

	context.pipelines.emplace_back(new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layoutOrbit3d));
	pipelineOrbit3d = context.pipelines.back().get();
	pipelineOrbit3d->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipelineOrbit3d->bindVertex(*m_Orbit);
	pipelineOrbit3d->bindShader("body_orbit3d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipelineOrbit3d->bindShader("body_orbit3d.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT);
	pipelineOrbit3d->bindShader("body_orbit3d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineOrbit3d->build();
}

void OrbitPlot::updateShader(double delta_time)
{
	orbit_fader.update(delta_time);
}

void OrbitPlot::computeOrbit(double date)
{
	// Large performance advantage from avoiding object overhead
	OsculatingFunctionType *oscFunc = body->orbit->getOsculatingFunction();
	double deltaTime = date - last_orbitJD;

	if (delta_orbitJD.second < 0)
		delta_orbitJD.first = -(delta_orbitJD.second = (body->re.sidereal_period/ORBIT_POINTS));

	// for performance only update orbit points if visible
	if (body->visibilityFader.getInterstate()>0.000001 && delta_orbitJD.second > 0 && (!orbit_cached || deltaTime < delta_orbitJD.first || deltaTime > delta_orbitJD.second)) {

		// calculate orbit first (for line drawing)
		double calc_date;
		//	  int delta_points = (int)(0.5 + (date - last_orbitJD)/date_increment);
		const double date_increment = body->re.sidereal_period/ORBIT_POINTS;

		if (orbit_cached && abs(date - last_orbitJD) < body->re.sidereal_period) {
			int delta_points;
			if (delta_orbitJD.second == date_increment) {
			 	delta_points = deltaTime/date_increment + (date > last_orbitJD) - 0.5;
				date = last_orbitJD + delta_points*date_increment;
			} else {
				if (deltaTime < 0 && deltaTime > delta_orbitJD.first * 1.6) {
					delta_points = -1;
					date = last_orbitJD + delta_orbitJD.first;
				} else if (deltaTime > 0 && deltaTime < delta_orbitJD.second * 1.6) {
					delta_points = 1;
					date = last_orbitJD + delta_orbitJD.second;
				} else { // Multiple points has changed
					// std::cout << body->getEnglishName() << " : deltaJD {" << delta_orbitJD.first << ", " << delta_orbitJD.second << "} ";
					((CometOrbit &) *body->orbit).deltaJDToOrbitJD(last_orbitJD, deltaTime);
					delta_points = deltaTime/date_increment + (date > last_orbitJD) - 0.5;
					// std::cout << "delta_points = " << delta_points << '\n';
					// std::cout << "date = " << date << " --> ";
					date = last_orbitJD + delta_points*date_increment;
					((CometOrbit &) *body->orbit).orbitJDToJD(date);
					// std::cout << date << '\n';
				}
			}

			delta_orbitJD = body->orbit->prepairFastPositionAtTimevInVSOP87Coordinates(date, date_increment);
			if (delta_points > 0) {
				for ( int d=0; d<ORBIT_POINTS; d++ ) {
					if (d + delta_points >= ORBIT_POINTS ) {
						// calculate new points
						calc_date = date + (d-ORBIT_POINTS/2)*date_increment;
						// date increments between points will not be completely constant though

						if(oscFunc)
							(*oscFunc)(date,calc_date,orbitPoint[d]);
						else
							body->orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
					} else {
						orbitPoint[d] = orbitPoint[d+delta_points];
					}
				}
			} else if (delta_points < 0) {
				for ( int d=ORBIT_POINTS-1; d>=0; d-- ) {
					if (d + delta_points < 0 ) {
						// calculate new points
						calc_date = date + (d-ORBIT_POINTS/2)*date_increment;

						if(oscFunc)
							(*oscFunc)(date,calc_date,orbitPoint[d]);
						else
							body->orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
					} else {
						orbitPoint[d] = orbitPoint[d+delta_points];
					}
				}
			}
		} else {
			delta_orbitJD = body->orbit->prepairFastPositionAtTimevInVSOP87Coordinates(date, date_increment);
			// update all points (less efficient)
			for ( int d=0; d<ORBIT_POINTS; d++ ) {
				calc_date = date + (d-ORBIT_POINTS/2)*date_increment;

				if(oscFunc)
					(*oscFunc)(date,calc_date,orbitPoint[d]);
				else
					body->orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
			}
			// \todo remove this for efficiency?  Can cause rendering issues near body though
			// If orbit is largely constant through time cache it
			if (body->orbit->isStable(date))
				orbit_cached = 1;
		}
		last_orbitJD = date;
	}
}

double OrbitPlot::computeOrbitBoundingRadius() const
{
	double highestSquaredLength = 0;
	for (int i = 0; i < ORBIT_POINTS; ++i) {
		double tmp = orbitPoint[i].lengthSquared();
		if (tmp > highestSquaredLength)
			highestSquaredLength = tmp;
	}
	return sqrt(highestSquaredLength);
}
