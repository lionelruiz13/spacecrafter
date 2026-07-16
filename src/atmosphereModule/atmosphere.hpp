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

//! @class Atmosphere
//!	@brief Class which compute and display the daylight sky color using openGL
//!	sky is computed with the Skylight class.

#ifndef _ATMOSTPHERE_H_
#define _ATMOSTPHERE_H_

#include <vector>
#include <memory>
#include "atmosphereModule/atmosphere_commun.hpp"
#include "tools/auto_fader.hpp"


#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include "EntityCore/SubBuffer.hpp"


class Skylight;
class Skybright;
class Projector;
class Navigator;
class ToneReproductor;
class VertexArray;
class VertexBuffer;
class Pipeline;

#define SKY_RESOLUTION 48
#define NB_LUM ((SKY_RESOLUTION+1) * (SKY_RESOLUTION+1))
#define NB_INDEX (((SKY_RESOLUTION+1) * 2 + 1) * SKY_RESOLUTION)

//! Path-neutral input of the sky-table computation (dual-path migration,
//! 2026-07-16). The skylight/skybright models consume only angles between
//! (view direction, sun, moon, zenith), so any right-handed local frame with
//! z = zenith is valid: the old path fills this from navigator/projector
//! (helioToLocal positions + unprojectNormalizedLocal grid), the new path
//! from the camera chain (observed positions rotated by eyeToLocal + the
//! same inverse-fisheye formula the projector applies).
//! Grid direction source selection: prj != nullptr -> old projector
//! unproject; otherwise eyeToLocal/halfFov (Camera::viewRotation transpose).
struct AtmosphereComputeInput {
	double jd = 0;
	Vec3d sunPos;         //!< sun position, observer-local frame, AU (length used for angular size)
	Vec3d moonPos;        //!< moon position, observer-local frame, AU
	double moonRadiusKm = 1738.; //!< angular-size source (old hardcode: 1738)
	float moonPhase = 0;  //!< illuminated fraction (old Body::get_phase)
	float latitudeDeg = 45.f;
	float altitudeM = 200.f;
	float temperatureC = 15.f;
	float relativeHumidity = 40.f;
	const Projector *prj = nullptr; //!< old-path grid directions
	Mat4f eyeToLocal;     //!< new-path grid directions: eye -> local rotation
	float halfFov = 0;    //!< new-path grid directions: radians
};

class Atmosphere: public NoCopy {
public:
	Atmosphere();
	virtual ~Atmosphere();

	//! Path-neutral sky-table computation (single authority; the legacy
	//! signature below wraps it).
	void computeColor(const AtmosphereComputeInput &in, const ToneReproductor *eye);

	void computeColor(double JD, Vec3d sunPos, Vec3d moonPos, float moon_phase, const ToneReproductor * eye, const Projector* prj,
	                   float latitude = 45.f, float altitude = 200.f,
	                   float temperature = 15.f, float relative_humidity = 40.f);

	//! Draw the atmosphere using the precalc values stored in tab_sky
	void draw();

	//! Set fade in/out duration in seconds
	void setFaderDuration(float duration) {
		fader.setDuration(duration);
	}
	//! Get fade in/out duration in seconds
	float getFaderDuration() {
		return fader.getDuration();
	}

	//set default_fader duration
	void setDefaultFaderDuration(float duration) {
		default_fader_duration = duration;
	}

	void setDefaultFaderDuration() {
		setFaderDuration(default_fader_duration);
	}

	//! Define whether to display atmosphere
	void setFlagShow(bool b) {
		fader = b;
	}

	//! Get whether atmosphere is displayed
	bool getFlagShow() const {
		return fader.finalState();
	}

	//! flip whether to display atmosphere
	void flipFlagShow() {
		fader = !fader.finalState();
	}

	//! tells you actual atm intensity due to eclipses + fader
	float getIntensity() {
		return atm_intensity;
	}

	//! let's you know how far faded in or out the atm is (0-1)
	float getFadeIntensity() {
		return fader;
	}

	float getWorldAdaptationLuminance() const { // Computed from eye luminance to update eye luminance
		return world_adaptation_luminance;
	}

	float getMilkywayAdaptationLuminance() const {
		return milkyway_adaptation_luminance;
	}

	//! for determining world adaptation luminance
	//! @warning DO NOT SET DIRECTLY, use core->setLightPollutionLimitingMagnitude instead
	void setLightPollutionLuminance(float luminance) {
		lightPollutionLuminance = luminance;
	}

	float getLightPollutionLuminance() {
		return lightPollutionLuminance;
	}

	//! builds the point display grid for the shaders
	void initGridPos();

	//! determines the viewport for the construction of the grids
	void initGridViewport(const Projector *prj);

	// indicates which atmosphere model will be calculated according to the planet
	void setModel(ATMOSPHERE_MODEL atmModel);

	inline float getMoonBrightness() {
		return (skyb->getMoonBrightness());
	}

	inline void setMoonBrightness(float b) {
		skyb->setMoonBrightness(b);
	}

	inline void setDefaultMoonBrightness(float b) {
		skyb->setDefaultMoonBrightness(b);
		skyb->setMoonBrightness(b);
	}

	inline void setDefaultMoonBrightness() {
		skyb->setDefaultMoonBrightness();
	}
private:
	//! initialize the shader parameters
	void createSC_context();

	std::unique_ptr<Skylight> sky;
	std::unique_ptr<Skybright> skyb;

	float world_adaptation_luminance = 0.f;
	float milkyway_adaptation_luminance = 0.f;
	float atm_intensity = 0.f;

	AParabolicFader fader;
	float lightPollutionLuminance = 0.f; 	//! light pollution simulation, add to svn 20070220

	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<VertexArray> m_atmGL;
	std::unique_ptr<VertexBuffer> skyPos;
	std::unique_ptr<VertexBuffer> skyColor;
	SubBuffer stagingSkyColor;
	SubBuffer indexBuffer;
	Vec3f *pSkyColor = nullptr;
	VkCommandBuffer cmds[3];

	float default_fader_duration;
};

#endif // _ATMOSTPHERE_H_
