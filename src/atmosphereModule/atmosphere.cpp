/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

//	Class which compute and display the daylight sky color using openGL
//	the sky is computed with the Skylight class.


#include <string>
#include "appModule/space_date.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include "atmosphereModule/skybright.hpp"
#include "atmosphereModule/skylight.hpp"

#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/sc_const.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/utility.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

Atmosphere::Atmosphere()
{
	sky = std::make_unique<Skylight>();
	skyb = std::make_unique<Skybright>();
	setFaderDuration(0.f);
	createSC_context();
}

Atmosphere::~Atmosphere()
{
	Context::instance->stagingMgr->releaseBuffer(stagingSkyColor);
	Context::instance->indexBufferMgr->releaseBuffer(indexBuffer);
}

void Atmosphere::initGridViewport(const Projector *prj)
{
	stepX = (float)prj->getViewportWidth() / SKY_RESOLUTION;
	stepY = (float)prj->getViewportHeight() / SKY_RESOLUTION;
	viewport_left = (float)prj->getViewportPosX();
	viewport_bottom = (float)prj->getViewportPosY();
}

//initializes the point grid for the atmosphere calculation
void Atmosphere::initGridPos()
{
	{
		float *data = (float *) Context::instance->transfer->planCopy(skyPos->get());
		for (int y=0; y<SKY_RESOLUTION+1; y++) {
			for (int x=0; x<SKY_RESOLUTION+1; x++) {
				*(data++) = viewport_left+x*stepX;
				*(data++) = viewport_bottom+y*stepY;
			}
		}
	}
	{
		uint16_t *data = (uint16_t *) Context::instance->transfer->planCopy(indexBuffer);
		for (int y=0; y<SKY_RESOLUTION; ++y) {
			const int offset1 = y * (SKY_RESOLUTION + 1);
			const int offset2 = offset1 + SKY_RESOLUTION+1;
			for (int x=0; x<SKY_RESOLUTION+1; ++x) {
				*(data++) = x + offset1;
				*(data++) = x + offset2;
			}
			*(data++) = UINT16_MAX;
		}
	}
}

void Atmosphere::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	assert(!m_atmGL);

	m_atmGL = std::make_unique<VertexArray>(vkmgr);
	m_atmGL->createBindingEntry(2 * sizeof(float));
	m_atmGL->addInput(VK_FORMAT_R32G32_SFLOAT);
	m_atmGL->createBindingEntry(3 * sizeof(float));
	m_atmGL->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	skyPos = m_atmGL->createBuffer(0, NB_LUM, context.globalBuffer.get());
	skyColor = m_atmGL->createBuffer(1, NB_LUM, context.globalBuffer.get());

	auto blendMode = BLEND_ADD;
	blendMode.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendMode.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, context.layouts.front().get());
	pipeline->bindVertex(*m_atmGL);
	pipeline->setBlendMode(blendMode);
	pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);
	pipeline->bindShader("atmosphere.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("atmosphere.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->build();

	indexBuffer = context.indexBufferMgr->acquireBuffer(NB_INDEX * sizeof(uint16_t));
	stagingSkyColor = context.stagingMgr->acquireBuffer(NB_LUM * sizeof(Vec3f));
	pSkyColor = (Vec3f *) context.stagingMgr->getPtr(stagingSkyColor);

	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		VkCommandBuffer &cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipeline->bind(cmd);
		context.layouts.front()->bindSet(cmd, *context.uboSet);
		VertexArray::bind(cmd, {skyPos.get(), skyColor.get()});
		vkCmdBindIndexBuffer(cmd, indexBuffer.buffer, indexBuffer.offset, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmd, NB_INDEX, 1, 0, 0, 0);
		context.frame[i]->compile(cmd);
	}
}

void Atmosphere::computeColor(double JD, Vec3d sunPos, Vec3d moonPos, float moon_phase,
                               const ToneReproductor * eye, const Projector* prj,
                               float latitude, float altitude, float temperature, float relative_humidity)
{
	float min_mw_lum = 0.13;

	// no need to calculate if not visible
	if (fader.isZero()) {
		atm_intensity = 0;
		world_adaptation_luminance = 3.75f + lightPollutionLuminance;
		milkyway_adaptation_luminance = min_mw_lum;  // brighter than without atm, since no drawing addition of atm brightness
		return;
	} else {
		atm_intensity = fader;
	}

	skylight_struct2 b2;

	// these are for radii
	double sun_angular_size = atan(696000./AU/sunPos.length());
	double moon_angular_size = atan(1738./AU/moonPos.length());

	double touch_angle = sun_angular_size + moon_angular_size;
	double dark_angle = moon_angular_size - sun_angular_size;

	sunPos.normalize();
	moonPos.normalize();

	// determine luminance falloff during solar eclipses
	double separation_angle = acos( sunPos.dot( moonPos ));  // angle between them
	//	printf("touch at %f\tnow at %f (%f)\n", touch_angle, separation_angle, separation_angle/touch_angle);

	// bright stars should be visible at total eclipse
	// because of above issues, this algorithm darkens more quickly than reality
	if ( separation_angle < touch_angle) {
		float min;
		if (dark_angle < 0) {
			// annular eclipse
			float asun = sun_angular_size*sun_angular_size;
			min = (asun - moon_angular_size*moon_angular_size)/asun;  // minimum proportion of sun uncovered
			dark_angle *= -1;
		} else min = 0.001; // so bright stars show up at total eclipse

		if (separation_angle < dark_angle) atm_intensity = min;
		else atm_intensity *= min + (1.-min)*(separation_angle-dark_angle)/(touch_angle-dark_angle);
		//		printf("atm int %f (min %f)\n", atm_intensity, min);
	}

	float sun_pos[3];
	sun_pos[0] = sunPos[0];
	sun_pos[1] = sunPos[1];
	sun_pos[2] = sunPos[2];

	float moon_pos[3];
	moon_pos[0] = moonPos[0];
	moon_pos[1] = moonPos[1];
	moon_pos[2] = moonPos[2];

	sky->setParamsv(sun_pos, 5.f);

	skyb->setLoc(latitude * M_PI/180., altitude, temperature, relative_humidity);
	skyb->setSunMoon(moon_pos[2], sun_pos[2]);//, cor_optoma);

	// Calculate the date from the julian day.
	ln_date date;
	SpaceDate::JulianToDate(JD, &date);

	skyb->setDate(date.years, date.months, moon_phase);

	Vec3d point(1., 0., 0.);

	// Variables used to compute the average sky luminance
	double sum_lum = 0.;

	// Compute the sky color for every point above the ground
	for (int x=0; x<SKY_RESOLUTION+1; x++) {
		for (int y=0; y<SKY_RESOLUTION+1; y++) {
			prj->unprojectLocal((double)viewport_left+x*stepX, (double)viewport_bottom+y*stepY,point);
			point.normalize();

			if (point[2]<=0) {
				point[2] = -point[2];
				// The sky below the ground is the symetric of the one above :
				// it looks nice and gives proper values for brightness estimation
			}

			b2.pos[0] = point[0];
			b2.pos[1] = point[1];
			b2.pos[2] = point[2];

			// Use the Skylight model for the color
			sky->get_xyY_Valuev(b2);

			// Use the Skybright.cpp 's models for brightness which gives better results.
			b2.color[2] = skyb->getLuminance(moon_pos[0]*b2.pos[0]+moon_pos[1]*b2.pos[1]+
			                                 moon_pos[2]*b2.pos[2], sun_pos[0]*b2.pos[0]+sun_pos[1]*b2.pos[1]+
			                                 sun_pos[2]*b2.pos[2], b2.pos[2]); //,cor_optoma);

			sum_lum+=b2.color[2];
			eye->xyY_to_RGB(b2.color);
			pSkyColor[x + y * (SKY_RESOLUTION + 1)].set(atm_intensity*b2.color[0],atm_intensity*b2.color[1],atm_intensity*b2.color[2]);
		}
	}

	world_adaptation_luminance = 3.75f + lightPollutionLuminance + 3.5*sum_lum/NB_LUM*atm_intensity;
	milkyway_adaptation_luminance = min_mw_lum*(1-atm_intensity) + 30*sum_lum/NB_LUM*atm_intensity;
	sum_lum = 0.f;
}

void Atmosphere::draw()
{
	if (fader.isZero())
		return;

	Context::instance->transfer->planCopyBetween(stagingSkyColor, skyColor->get());
	Context::instance->frame[Context::instance->frameIdx]->toExecute(cmds[Context::instance->frameIdx], PASS_MULTISAMPLE_DEPTH);
}

void Atmosphere::setModel(ATMOSPHERE_MODEL atmModel)
{
	sky->setComputeTypeColor(atmModel);
}
