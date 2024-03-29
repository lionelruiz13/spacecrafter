#include "bodyModule/body_decor.hpp"
#include "bodyModule/body.hpp"
#include "coreModule/milkyway.hpp"
#include "atmosphereModule/atmosphere.hpp"

BodyDecor::BodyDecor(std::shared_ptr<MilkyWay> _milky, std::shared_ptr<Atmosphere> _atmosphere)
{
	milky = _milky;
	atmosphere = _atmosphere;
}



void BodyDecor::anchorAssign(/*bool Spacecraft*/)
{
	// if (Spacecraft) {
	// 	drawLandscape = true;
	// }
	// else {
		drawLandscape = false;
	// }
	drawMeteor = false;
	// we are in space
	drawBody = true;
	milky->useIrisTexture(true);
}

void BodyDecor::bodyAssign(double altitude, const AtmosphereParams* atmParam/*, bool Spacecraft*/)
{

	// landscape and body management
	if ((altitude<atmParam->limLandscape) /*or Spacecraft*/) {
		drawLandscape = true;
		drawBody = false; //Spacecraft;
	}
	else {
		drawLandscape = false;
		drawBody = true;
	}

	bool hasAtmosphere = atmParam->hasAtmosphere;

	if (!hasAtmosphere) {
		milky->useIrisTexture(true);
		drawMeteor = false;
		atmosphere->setFlagShow(false);
		return;
	}

	bool outZoneAtmosphere = (altitude > atmParam->limSup);

	//we are on a body but outside the atmosphere zone
	if (outZoneAtmosphere) {
		atmosphere->setFlagShow(false);
		drawMeteor = false;
		milky->useIrisTexture(true);
		return;
	}

	//we are on a body that has an atmosphere and we are in it
	//the user doesn't want to draw an atmosphere
	if (!atmState) {
		atmosphere->setFlagShow(false);
		milky->useIrisTexture(true);
		drawMeteor = false;
		return;
	}

	// we are on a body that has an atmosphere and we are close to the star
	if (altitude < atmParam->limInf) {
		atmosphere->setFlagShow(true);
		milky->useIrisTexture(false);
		drawMeteor = true;
		return;
	}
	else {
		atmosphere->setFlagShow(false);
		milky->useIrisTexture(true);
		drawMeteor = true;
		return;
	}
}
