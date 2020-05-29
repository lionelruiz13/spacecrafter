/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
 * The implementation of most functions in this file
 * (getInfoString,getShortInfoString,...) is taken from
 * Stellarium, Copyright (C) 2002 Fabien Chereau,
 * and therefore the copyright of these belongs to Fabien Chereau.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

#include <sstream>
#include "coreModule/time_mgr.hpp"
#include "navModule/observer.hpp"
#include "starModule/hip_star_wrapper.hpp"
#include "starModule/zone_array.hpp"
#include "tools/utility.hpp"
#include "tools/translator.hpp"
#include "tools/fmath.hpp"


namespace BigStarCatalog {

std::string StarWrapperBase::getInfoString(const Navigator *nav) const
{
	const Vec3d j2000_pos = getObsJ2000Pos(nav);
	double dec_j2000, ra_j2000;
	Utility::rectToSphe(&ra_j2000,&dec_j2000,j2000_pos);
	const Vec3d equatorial_pos = nav->j2000ToEarthEqu(j2000_pos);
	double dec_equ, ra_equ;
	Utility::rectToSphe(&ra_equ,&dec_equ,equatorial_pos);
	std::stringstream oss;
	oss.setf(std::ios::fixed, std::ios::floatfield);
	oss.precision(2);
	oss << "Magnitude: " << getMag(nav) << " B-V: " << getBV() << std::endl;
	oss << "J2000" << " " << "RA/DE: " << Utility::printAngleHMS(ra_j2000,true) << " / " << Utility::printAngleDMS(dec_j2000,true) << std::endl;
	oss << "Equ of date" << " " << "RA/DE: " << Utility::printAngleHMS(ra_equ) << " / " << Utility::printAngleDMS(dec_equ) << std::endl;

	// calculate alt az
	double az,alt;
	Utility::rectToSphe(&az,&alt,nav->earthEquToLocal(equatorial_pos));
	az = 3*M_PI - az;  // N is zero, E is 90 degrees
	if(az > M_PI*2) az -= M_PI*2;
	oss << "Alt/Az: " << Utility::printAngleDMS(alt) << " / " << Utility::printAngleDMS(az) << std::endl;

	return oss.str();
}

void StarWrapperBase::getRaDeValue(const Navigator *nav, double *ra, double *de) const
{
	const Vec3d j2000_pos = getObsJ2000Pos(nav);
	double dec_j2000, ra_j2000;
	Utility::rectToSphe(&ra_j2000,&dec_j2000,j2000_pos);
	*ra= ra_j2000*180/3.1415927;
	*de= dec_j2000*180/3.1415927;
}

std::string StarWrapperBase::getShortInfoString(const Navigator *nav) const
{
	std::stringstream oss;
	oss.setf(std::ios::fixed, std::ios::floatfield);
	oss.precision(2);
	oss << "Magnitude: " << getMag(nav);
	return oss.str();
}

std::string StarWrapperBase::getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const
{
	return " ";
}

float StarWrapper1::getStarDistance( void )
{
	if (s->getPlx())
		return (AU/(SPEED_OF_LIGHT*86400*365.25)) / (s->getPlx()*((0.00001/3600)*(M_PI/180)));
	else
		return 0;
}

std::string StarWrapper1::getEnglishName(void) const
{
	if (s->getHip()) {
		char buff[64];
		sprintf(buff,"HP %d",s->getHip());
		return buff;
	}
	return StarWrapperBase::getEnglishName();
}

std::string StarWrapper1::getInfoString(const Navigator *nav) const
{
	const Vec3d j2000_pos = getObsJ2000Pos(nav);
	double dec_j2000, ra_j2000;
	Utility::rectToSphe(&ra_j2000,&dec_j2000,j2000_pos);
	const Vec3d equatorial_pos = nav->j2000ToEarthEqu(j2000_pos);
	double dec_equ, ra_equ;
	Utility::rectToSphe(&ra_equ,&dec_equ,equatorial_pos);
	std::stringstream oss;
	if (s->getHip()) {
		const std::string commonNameI18 = HipStarMgr::getCommonName(s->getHip());
		const std::string sciName = HipStarMgr::getSciName(s->getHip());
		if (commonNameI18!="" || sciName!="") {
			oss << commonNameI18 << (commonNameI18 == "" ? "" : " ");
			if (commonNameI18!="" && sciName!="") oss << "(";
			oss << (sciName=="" ? "" : sciName);
			if (commonNameI18!="" && sciName!="") oss << ")";
			oss << std::endl;
		}
		oss << "HP " << s->getHip();
		if (s->getComponentIds()) {
			oss << " " << HipStarMgr::convertToComponentIds(s->getComponentIds()).c_str();
		}
		oss << std::endl;
	}

	oss.setf(std::ios::fixed, std::ios::floatfield);
	oss.precision(2);
	oss << _("Magnitude: ") << getMag(nav) << " " << _("B-V: ") << s->getBV() << std::endl;
	oss << _("J2000 RA/DE: ") << Utility::printAngleHMS(ra_j2000,true) << " / " << Utility::printAngleDMS(dec_j2000,true) << std::endl;
	oss << _("Equ of date RA/DE: ") << Utility::printAngleHMS(ra_equ) << " / " << Utility::printAngleDMS(dec_equ) << std::endl;

	// calculate alt az
	double az,alt;
	Utility::rectToSphe(&az,&alt,nav->earthEquToLocal(equatorial_pos));
	az = 3*M_PI - az;  // N is zero, E is 90 degrees
	if (az > M_PI*2)
		az -= M_PI*2;
	oss << _("Alt/Az: ") << Utility::printAngleDMS(alt) << " / " << Utility::printAngleDMS(az) << std::endl;

	if (s->getPlx()) {
		oss.precision(5);
		oss << _("Parallax: ") << (0.00001*s->getPlx()) << std::endl;
		oss.precision(2);
		oss << _("Distance: ") << (AU/(SPEED_OF_LIGHT*86400*365.25)) / (s->getPlx()*((0.00001/3600)*(M_PI/180))) << " " << _("ly") << std::endl;
	}

	if (s->getSpInt()) {
		oss << _("Spectral Type: ") << HipStarMgr::convertToSpectralType(s->getSpInt()).c_str() << std::endl;
	}
	return oss.str();
}



// void StarWrapper1::getRaDeValue(const Navigator *nav, double *ra, double *de) const
// {
// 	const Vec3d j2000_pos = getObsJ2000Pos(nav);
// 	double dec_j2000, ra_j2000;
// 	Utility::rectToSphe(&ra_j2000,&dec_j2000,j2000_pos);
// 	*ra= ra_j2000*180/3.1415927;
// 	*de= dec_j2000*180/3.1415927;
// }

std::string StarWrapper1::getShortInfoString(const Navigator *nav) const
{
	std::stringstream oss;
	if (s->getHip()) {
		const std::string commonNameI18 = HipStarMgr::getCommonName(s->getHip());
		const std::string sciName = HipStarMgr::getSciName(s->getHip());
		if (commonNameI18!="" || sciName!="") {
			oss << commonNameI18 << (commonNameI18 == "" ? "" : " ");
			if (commonNameI18!="" && sciName!="") oss << "(";
			oss << (sciName=="" ? "" : sciName);
			if (commonNameI18!="" && sciName!="") oss << ")";
			oss << "  ";
		}
		oss << "HP " << s->getHip();
		if (s->getComponentIds()) {
			oss << " " << HipStarMgr::convertToComponentIds(s->getComponentIds()).c_str();
		}
		oss << "  ";
	}

	oss.setf(std::ios::fixed, std::ios::floatfield);
	oss.precision(2);
	oss << _("Magnitude: ") << getMag(nav) << "  ";

	if (s->getPlx()) {
		oss << _("Distance: ") << (AU/(SPEED_OF_LIGHT*86400*365.25)) / (s->getPlx()*((0.00001/3600)*(M_PI/180))) << " " << _("ly") << "  ";
	}

	if (s->getSpInt()) {
		oss << _("Spectral Type: ") << HipStarMgr::convertToSpectralType(s->getSpInt()).c_str();
	}
	return oss.str();
}

std::string StarWrapper1::getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const
{
	std::stringstream oss;
	const Vec3d j2000_pos = getObsJ2000Pos(nav);
	float tempDE, tempRA;
	Vec3d equatorial_pos = nav->j2000ToEarthEqu(j2000_pos);
	Utility::rectToSphe(&tempRA,&tempDE,equatorial_pos);
	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << "/" << Utility::printAngleDMS(tempDE);

	double jd=timeMgr->getJulian();
	double sidereal;
	double T;
	double Le;
	double HA;
	double GHA;
	double PA;

	T = (jd - 2451545.0) / 36525.0;
	Le = observatory->getLongitude();
	/* calc mean angle */
	sidereal = 280.46061837 + (360.98564736629 * (jd - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
	HA=sidereal+Le-tempRA*180.0/M_PI;
	GHA=sidereal-tempRA*180.0/M_PI;
	while (HA>=360) HA-=360;
	while (HA<0)    HA+=360;
	if (HA<180) PA=HA;
	else PA=360-HA;
	while (GHA>=360) GHA-=360;
	while (GHA<0)    GHA+=360;
	while (tempRA>=2*M_PI) tempRA-=2*M_PI;
	while (tempRA<0)    tempRA+=2*M_PI;

	oss << _("SA ") << Utility::printAngleDMS(2*M_PI-tempRA) << " LHA " << Utility::printAngleDMS(HA*M_PI/180.0) << " GHA " << Utility::printAngleDMS(GHA*M_PI/180.0);

	// calculate alt az
	Vec3d local_pos = nav->earthEquToLocal(equatorial_pos);
	Utility::rectToSphe(&tempRA,&tempDE,local_pos);
	tempRA = 3*M_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > M_PI*2) tempRA -= M_PI*2;

	oss << "@" << _(" Az/Alt: ") << Utility::printAngleDMS(tempRA) << "/" << Utility::printAngleDMS(tempDE) << " LPA " << Utility::printAngleDMS(PA*M_PI/180.0);
	return oss.str();
}


ObjectBaseP Star1::createStelObject(const SpecialZoneArray<Star1> *a, const SpecialZoneData<Star1> *z) const
{
	return ObjectBaseP(new StarWrapper1(a,z,this));
}

ObjectBaseP Star2::createStelObject(const SpecialZoneArray<Star2> *a, const SpecialZoneData<Star2> *z) const
{
	return ObjectBaseP(new StarWrapper2(a,z,this));
}

ObjectBaseP Star3::createStelObject(const SpecialZoneArray<Star3> *a, const SpecialZoneData<Star3> *z) const
{
	return ObjectBaseP(new StarWrapper3(a,z,this));
}

} // namespace BigStarCatalog

