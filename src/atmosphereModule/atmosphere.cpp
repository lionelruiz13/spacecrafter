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
#include "tools/scoped_value.hpp"
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
}

//initializes the point grid for the atmosphere calculation
void Atmosphere::initGridPos()
{
	{
		float *data = (float *) Context::instance->transfer->planCopy(skyPos->get());
		float y_val = -1.f;
		for (int y=0; y<SKY_RESOLUTION+1; y++) {
			float x_val = -1.f;
			for (int x=0; x<SKY_RESOLUTION+1; x++) {
				auto pos = VulkanMgr::instance->rectToRender({x_val, y_val});
				*(data++) = pos.first;
				*(data++) = pos.second;
				x_val += (2.f / SKY_RESOLUTION);
			}
			y_val += (2.f / SKY_RESOLUTION);
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
	                   		   Vec3d earthPos_helio, Vec3d moonPos_helio,
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

	// Use GEOCENTRIC positions for lunar eclipse calculation
	// (Earth's shadow is projected from Earth's center, not from observer position)
	// In heliocentric coords: Sun=(0,0,0), Earth=earthPos_helio, Moon=moonPos_helio
	// Convert to geocentric (Earth-centered):
	Vec3d sunPos_geo = -earthPos_helio;  // Sun seen from Earth
	Vec3d moonPos_geo = moonPos_helio - earthPos_helio;  // Moon seen from Earth
	double sun_distance_geo = sunPos_geo.length();
	double moon_distance_geo = moonPos_geo.length();
	sunPos_geo.normalize();
	moonPos_geo.normalize();

	// Calculate the anti-sun direction (direction of Earth's shadow from Earth center)
	Vec3d anti_sun = -sunPos_geo;

	// Calculate angular distance between moon center and shadow center (geocentric)
	double shadow_alignment = acos(moonPos_geo.dot(anti_sun));

	// Calculate Earth's umbra angular radius at moon distance (real-time calculation)
	// Based on geometry: umbra radius decreases with distance from Earth
	const double earth_radius_km = 6371.0;
	const double sun_radius_km = 696000.0;
	double sun_distance_km = sun_distance_geo * AU;
	double moon_distance_km = moon_distance_geo * AU;

	// At typical moon distance (384,400 km), this should give ~4600-4700 km
	// For a more accurate formula, we can recalculate:
	// Using similar triangles: umbra_radius / d_moon = (R_earth - R_sun) / d_sun
	// But since R_sun >> R_earth, the shadow cone angle is atan((R_sun - R_earth) / d_sun)
	double shadow_cone_half_angle = atan((sun_radius_km - earth_radius_km) / sun_distance_km);
	double umbra_radius_km = earth_radius_km - moon_distance_km * tan(shadow_cone_half_angle);

	// Convert to angular radius as seen from Earth
	double umbra_angle = atan(umbra_radius_km / moon_distance_km);
	// Moon's angular radius: already calculated
	double moon_radius = moon_angular_size;

	// Calculate what percentage of the moon's disk is covered by Earth's shadow
	// This is a linear approximation based on how far the moon has entered the umbra
	// When moon center is at this distance or more from shadow center: moon is completely outside
	double moon_fully_outside = umbra_angle + moon_radius;
	// When moon center is at this distance or less from shadow center: moon is completely inside
	double moon_fully_inside = umbra_angle - moon_radius;

	float moon_coverage_percent = 0.0f;
	if (shadow_alignment <= moon_fully_inside) {
		// Moon is completely engulfed by shadow
		moon_coverage_percent = 1.0f;
	} else if (shadow_alignment < moon_fully_outside) {
		// Moon is partially in shadow - calculate percentage linearly
		// As moon moves from fully outside to fully inside, coverage goes from 0% to 100%
		moon_coverage_percent = (moon_fully_outside - shadow_alignment) / (2.0 * moon_radius);
	}
	// else: moon is completely outside shadow, coverage stays 0%

	// ScopedValue to temporarily reduce moon brightness during eclipse based on coverage (and automatically restore after scope exit (exit of this function))
	auto moon_brightness_scoped = ScopedValue(
		[this]() { return this->skyb->getMoonBrightness(); },
		[this](double brightness) { this->skyb->setMoonBrightness(brightness); }
	);
	// Fade atmosphere proportionally to moon coverage
	// 0% moon coverage = 100% atmosphere, 100% moon coverage = 0% atmosphere
	if (moon_coverage_percent > 0.0f) {
		float lunar_eclipse_fader = 1.0f - moon_coverage_percent;
		moon_brightness_scoped.set(skyb->getMoonBrightness() * lunar_eclipse_fader); // Reduce moon brightness proportionally to coverage to avoid bright moon during eclipse
	}

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

	// Calculate sun transition factor (0 = full night, 1 = day/twilight)
	// Transition zoone: sun_pos[2] from -0.2 (deep night) to -0.05 (twilight starts)
	float sun_transition = 0.0f;
	if (sun_pos[2] < -0.2f) {
		sun_transition = 0.0f; // Deep night - blue correction active
	} else if (sun_pos[2] > -0.05f) {
		sun_transition = 1.0f; // Twilight/day - natural coolors from Skylight model
	} else {
		// Smooth transition - faster fade to preserve warm twilight colors
		sun_transition = (sun_pos[2] + 0.2f) / 0.15f; // Linear transition
	}

	// Pre-calculate moon-related factors
	bool apply_blue = (sun_transition < 1.0f && moon_pos[2] > -0.25f);
	float total_blue_factor = 0.0f;
	const float target_x = 0.25f;
	const float target_y = 0.25f;

	// At night (sun well below horizon), apply blue correction ONLY if moon is present
	if (apply_blue) {
		// Calculate moon presence factor
		float moon_presence = 0.0f;
		if (moon_pos[2] >= 0.0f) {
			moon_presence = 1.0f;
		} else if (moon_pos[2] > -0.25f) {
			// Fade in as moon approaches horizon
			moon_presence = (moon_pos[2] + 0.25f) / 0.25f;
		}

		// Pre-calculate normalized moon brightness
		// Use the original moon brightness setting (before eclipse dimming) to determine how much blue to add,
		// so that the blue effect is stronger for higher moon brightness settings.
		// The division by 10 is arbitrary to scale it to a reasonable range for blue factor calculation.
		float normalized_moon_brightness = moon_brightness_scoped.saved() / 10.0f;

		// Base night blue color - only when moon is present
		// Scale with moon presence AND moon_brightness setting to avoid blue without moon
		// If moon_brightness is 0, no blue effect at all
		float night_factor = (1.0f - sun_transition) * moon_presence * normalized_moon_brightness * 0.7f;

		// Calculate moon altitude transition
		float moon_altitude_transition = 0.0f;
		if (moon_pos[2] < -0.25f) {
			moon_altitude_transition = 0.0f;
		} else if (moon_pos[2] > 0.15f) {
			moon_altitude_transition = 1.0f;
		} else {
			float t = (moon_pos[2] + 0.25f) / 0.4f;
			moon_altitude_transition = t * t * (3.0f - 2.0f * t);
		}

		// When both below horizon, scale moon effect based on its relative height to sun to avoid blue when moon is much lower than sun
		float relative_height_factor = 1.0f;
		if (sun_pos[2] < -0.05f && moon_pos[2] < -0.05f) {
			// Both below: blend based on who's higher
			float altitude_diff = moon_pos[2] - sun_pos[2];
			if (altitude_diff > 0.0f) {
				// Moon higher than sun - full moon effect
				relative_height_factor = std::min(1.0f, altitude_diff / 0.15f);
			} else {
				// Sun higher - reduce moon effect smoothly
				relative_height_factor = std::max(0.0f, 1.0f + altitude_diff / 0.15f);
			}
		}

		// Moon influence based on altitude and brightness setting
		// Use the original moon brightness setting to determine influence, so that the blue effect is stronger for higher moon brightness settings
		float moon_influence = std::max(0.0f, moon_pos[2]) * moon_brightness_scoped.saved();
		float moon_factor = std::min(1.0f, moon_influence * 2.0f);
		moon_factor *= moon_altitude_transition;
		moon_factor *= relative_height_factor; // Apply relative height scaling
		moon_factor *= (1.0f - sun_transition);

		// Moon adds up to 30% extra blue on top of base night blue
		float moon_bonus = moon_factor * 0.3f;

		// Total blue shift = base night (scaled by moon presence) + moon bonus
		total_blue_factor = std::min(1.0f, night_factor + moon_bonus);
	}

	// Compute the sky color for every point above the ground
	float x_val = -1.f;
	for (int x=0; x <= SKY_RESOLUTION; x++) {
		float y_val = -1.f;
		for (int y=0; y <= SKY_RESOLUTION; y++) {
			prj->unprojectNormalizedLocal(x_val, y_val, point);
			point.normalize();
			y_val += (2.f / SKY_RESOLUTION);

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

			// Apply blue correction if needed (pre-calculated values)
			if (total_blue_factor > 0.01f) {
				b2.color[0] = b2.color[0] * (1.0f - total_blue_factor) + target_x * total_blue_factor;
				b2.color[1] = b2.color[1] * (1.0f - total_blue_factor) + target_y * total_blue_factor;
			}

			sum_lum+=b2.color[2];
			eye->xyY_to_RGB(b2.color);
			pSkyColor[x + y * (SKY_RESOLUTION + 1)].set(atm_intensity*b2.color[0],atm_intensity*b2.color[1],atm_intensity*b2.color[2]);
		}
		x_val += (2.f / SKY_RESOLUTION);
	}
	if (isnormal(sum_lum)) {
		world_adaptation_luminance = 3.75f + lightPollutionLuminance + 3.5*sum_lum/NB_LUM*atm_intensity;
		milkyway_adaptation_luminance = min_mw_lum*(1-atm_intensity) + 30*sum_lum/NB_LUM*atm_intensity;
	}
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
