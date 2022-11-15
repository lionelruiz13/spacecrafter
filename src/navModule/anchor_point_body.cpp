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

#include <iostream>
#include <sstream>
#include "bodyModule/body.hpp"
#include "navModule/anchor_point_body.hpp"
//#include "tools/fmath.hpp"

AnchorPointBody::AnchorPointBody(std::shared_ptr<Body> _body) noexcept
{
	body = _body;
	setHeliocentricEclipticPos(_body->get_heliocentric_ecliptic_pos());
}

Mat4d AnchorPointBody::getRotLocalToEquatorial(double jd, double lat, double lon, double alt) const noexcept
{

	// TODO: Figure out how to keep continuity in sky as reach poles
	// otherwise sky jumps in rotation when reach poles in equatorial mode
	// This is a kludge
	if ( lat > 89.5 )  lat = 89.5;
	if ( lat < -89.5 ) lat = -89.5;

	// if(alt > body->getRadius() * rotationMultiplierCondition * AU *1000.0 || overrideRotationCondition) {
	// 	//~ cout << "Décrochage : Altitude : " << alt << " limite pour la rotation " << body->getRadius() * rotationMultiplierCondition * AU *1000.0 << endl;
	// 	//~ if(alt > body->getRadius() * rotationMultiplierCondition * 149597870000 || overrideRotationCondition){
	// 	//~ if(alt > body->getRadius() * 5 * AU || overrideRotationCondition){
	// 	//if we are further than rotationMultiplierCondition times the body's radius or the user chose to not follow the rotation

	// 	if(followRotation) {
	// 		//Remember the current offset and the date we stopped following the rotation
	// 		followRotation = false;
	// 		lastOffset = body->getSiderealTime(jd) - elapsedSideralTime;
	// 		lastSideralTime = body->getSiderealTime(jd);
	// 	}

	// 	return Mat4d::zrotation((lastOffset + lon)*(M_PI/180.))
	// 	       * Mat4d::yrotation((90.-lat)*(M_PI/180.));
	// }
	// else {
	// 	//~ cout << "Accrochage : Altitude : " << alt << " limite pour la rotation " << body->getRadius() * rotationMultiplierCondition * AU *1000.0 << endl;
	// 	if(!followRotation) {
	// 		//catch up on the rotation that occured while we didn't follow the body's rotation
	// 		followRotation = true;
	// 		elapsedSideralTime += body->getSiderealTime(jd) - lastSideralTime;
	// 		elapsedSideralTime -= ((int)elapsedSideralTime/360) * 360;
	// 	}

	return Mat4d::zrotation((body->getSiderealTime(jd)+lon)*(M_PI/180.))
			* Mat4d::yrotation((90.-lat)*(M_PI/180.));
	// }
}

Mat4d AnchorPointBody::getRotEquatorialToVsop87()const noexcept
{
	return body->getRotEquatorialToVsop87();
}


void AnchorPointBody::update() noexcept
{
	setHeliocentricEclipticPos(body->get_heliocentric_ecliptic_pos());
}

bool AnchorPointBody::isOnBody(Body *_body) const noexcept
{
	return body.get() == _body;
}

bool AnchorPointBody::isOnBody()const noexcept
{
	return true;
}

std::string AnchorPointBody::saveAnchor()const noexcept
{

	std::ostringstream os;

	os << "type body" << std::endl;
	os << "body_name " << body->getEnglishName() << std::endl;

	return os.str();

}
