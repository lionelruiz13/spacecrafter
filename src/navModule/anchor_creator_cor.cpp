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

#include "anchor_point_orbit.hpp"
#include "anchor_point_observatory.hpp"
#include "anchor_creator_cor.hpp"
#include "bodyModule/orbit_creator_cor.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_body.hpp"
#include "tools/log.hpp"

AnchorPointCreator::AnchorPointCreator(const AnchorCreator * _next):
	AnchorCreator(_next) { }

AnchorPoint * AnchorPointCreator::handle(stringHash_t params)const
{

	if(params["type"] != "point") {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("AnchorPointCreator::handle unknown type");
			return nullptr;
		}
	}

	if(
	    params["x"].empty() ||
	    params["y"].empty() ||
	    params["z"].empty()) {
		cLog::get()->write("AnchorPointCreator::handle x y or z parameter missing");
		return nullptr;
	}

	return new AnchorPoint(
	           stod(params["x"]),
	           stod(params["y"]),
	           stod(params["z"]));

}

AnchorObservatoryCreator::AnchorObservatoryCreator(const AnchorCreator * next)
	: AnchorCreator(next){ }
AnchorPoint * AnchorObservatoryCreator::handle(stringHash_t params) const{
	if(params["type"] != "observatory") {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("AnchorPointCreator::handle unknown type");
			return nullptr;
		}
	}
	if(
	    params["x"].empty() ||
	    params["y"].empty() ||
	    params["z"].empty()) {
		cLog::get()->write("AnchorPointCreator::handle x y or z parameter missing");
		return nullptr;
	}
	return new AnchorPointObservatory(
	           stod(params["x"]),
	           stod(params["y"]),
	           stod(params["z"]));
}

AnchorPointBodyCreator::AnchorPointBodyCreator(const AnchorCreator * _next, const SSystemFactory * _ssystem) :
	AnchorCreator(_next)
{
	ssystem = _ssystem;
}

AnchorPoint * AnchorPointBodyCreator::handle(stringHash_t params)const
{

	if(params["type"] != "body") {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("AnchorPointBodyCreator::handle unknown type");
			return nullptr;
		}
	}

	Body * body = ssystem->searchByEnglishName(params["body_name"]);

	if(body != nullptr)
		return new AnchorPointBody(body);

	cLog::get()->write("AnchorPointBodyCreator::handle the given body was not found in solar system");
	return nullptr;

}

AnchorPointOrbitCreator::AnchorPointOrbitCreator(
    const AnchorCreator * _next,
    const SSystemFactory * _ssystem,
    const TimeMgr * _timeMgr,
    const OrbitCreator * _orbitCreator):
	AnchorCreator(_next)
{
	ssystem = _ssystem;
	timeMgr = _timeMgr;
	orbitCreator = _orbitCreator;
}

AnchorPoint * AnchorPointOrbitCreator::handle(stringHash_t params)const
{
	if(params["type"] != "orbit") {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("AnchorPointOrbitCreator::handle unknown type");
			return nullptr;
		}
	}

	Orbit * orbit = orbitCreator->handle(params).get();

	if(orbit == nullptr) {
		cLog::get()->write("AnchorPointOrbitCreator:: could not create orbit from given paramaters");
		return nullptr;
	}

	if(params["parent"].empty()) {

		if(	params["orbit_center_x"].empty() ||
		        params["orbit_center_y"].empty() ||
		        params["orbit_center_z"].empty()) {

			cLog::get()->write("AnchorPointOrbitCreator:: orbit needs a orbit_center if it has no parent");
			return nullptr;
		}

		return new AnchorPointOrbit(orbit, timeMgr, nullptr,
		                            Vec3d(stod(params["orbit_center_x"]),
		                                  stod(params["orbit_center_y"]),
		                                  stod(params["orbit_center_z"])));
	}
	else {
		Body * body = ssystem->searchByEnglishName(params["parent"]);

		if(body != nullptr)
			return new AnchorPointOrbit(orbit, timeMgr, body);
		else {
			cLog::get()->write("AnchorPointOrbitCreator:: could not find given parent : " + params["parent"]);
			return nullptr;
		}

	}

}
