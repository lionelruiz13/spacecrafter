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

#include <fstream>
#include <sstream>
#include "appModule/space_date.hpp"
#include "bodyModule/body.hpp"
#include "bodyModule/orbit_creator_cor.hpp"
#include "bodyModule/solarsystem.hpp"
#include "coreModule/time_mgr.hpp"
#include "navModule/anchor_creator_cor.hpp"
#include "navModule/anchor_manager.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_body.hpp"
#include "navModule/anchor_point_orbit.hpp"
#include "navModule/anchor_point_observatory.hpp"
#include "navModule/observer.hpp"
#include "navModule/navigator.hpp"
#include "tools/log.hpp"


/*
 * returns the position for a movement at given date
 * 
 * The trajectory followed is a straight line between two points, however the speed is a cosine function.
 * This allows the speed to be low at the start and finish, but hight in between.
 * 
 */
Vec3d AnchorManager::getTravelPosition(double JD)
{

	if(JD > arrivalTime)
		JD = arrivalTime;

	if(JD < startTime)
		JD = startTime;

	double movement = distanceToTavel * ( cos( C_PI * ( (JD-startTime)/travelTime - 1 ) ) + 1) / 2 ;

	return startPosition + direction * movement;

}

/*
 * gets the anchor params from a file
 */
static stringHash_t getAnchorParams(const std::string& fileName)
{

	stringHash_t anchorParams;
	std::ifstream fileAnchor (fileName.c_str(), std::ifstream::in);
	cLog::get()->write("Reading fileAnchor "+ fileName);

	if(!fileAnchor.is_open()) {
		cLog::get()->write("AnchorManager::loadCameraPosition couldn't read file", LOG_TYPE::L_WARNING);
		return anchorParams;
	}
	std::string line;
	while(getline(fileAnchor, line)) {
		if (line[0]!='#' && line.size() != 0) {
			int pos = line.find(' ',0);
			std::string p1=line.substr(0,pos);
			std::string p2=line.substr(pos+1,line.size());
			anchorParams[p1]=p2;
		}
	}
	fileAnchor.close();

	return anchorParams;
}

/*
 * returns the type of an anchor from a string
 */
static AnchorManager::anchor_type getAnchorTypeFromString(const std::string & str)
{

	if(str == "point")
		return AnchorManager::anchor_type::point;

	if(str == "body")
		return AnchorManager::anchor_type::body;

	if(str == "orbit")
		return AnchorManager::anchor_type::orbit;

	if(str == "observatory")
		return AnchorManager::anchor_type::observatory;

	return AnchorManager::anchor_type::unknown;
}

/*
 * returns true if the anchor is of the given type
 */
static bool rightType(const AnchorPoint * anchor, const AnchorManager::anchor_type & type)
{

	switch(type) {
		case AnchorManager::anchor_type::point :
			return typeid(*anchor) == typeid(AnchorPoint);
			break;

		case AnchorManager::anchor_type::body :
			return typeid(*anchor) == typeid(AnchorPointBody);
			break;

		case AnchorManager::anchor_type::orbit :
			return typeid(*anchor) == typeid(AnchorPointOrbit);
			break;

		case AnchorManager::anchor_type::observatory:
			return typeid(*anchor) == typeid(AnchorPointObservatory);
			break;

		default:
			return false;
			break;
	}

	return false;
}

AnchorPoint * AnchorManager::constructAnchor(stringHash_t params)
{
	return anchorCreator->handle(params);
}

AnchorManager::AnchorManager(
    Observer * obs, Navigator * nav,
    SolarSystem * _ssystem,
    TimeMgr * mgr,
    const OrbitCreator * orbitCreator) noexcept
{
	observer = obs;
	navigator = nav;
	ssystem = _ssystem;
	_ssystem->setAnchorManager(this);
	timeMgr = mgr;

	AnchorCreator * observatory = new AnchorObservatoryCreator(nullptr);	
	AnchorCreator * orbit = new AnchorPointOrbitCreator(observatory,_ssystem,mgr, orbitCreator);
	AnchorCreator * body = new AnchorPointBodyCreator(orbit, _ssystem);
	anchorCreator = new AnchorPointCreator(body);
}

AnchorManager::~AnchorManager()
{
	for(auto it = anchors.begin(); it != anchors.end(); it++)
		delete(it->second);

	anchors.clear();
}

bool AnchorManager::addAnchor(const std::string& name, AnchorPoint * anchor) noexcept
{
	auto it = anchors.find(name);
	if(it != anchors.end()) //name already present in container
		return false;

	if(anchor == nullptr) {
		// @TODO le passer en log
		cLog::get()->write("error addAnchor AnchorPoint nullptr " + name, LOG_TYPE::L_ERROR);
		// std::cout << "error addAnchor AnchorPoint nullptr " << name << std::endl;
		return false;
	}

	anchors.insert(std::pair<std::string, AnchorPoint*>(name, anchor));
	return true;
}

bool AnchorManager::removeAnchor(const std::string& name) noexcept
{
	for(auto it = anchors.begin(); it != anchors.end(); it++) {
		if(it->first == name) {

			if(currentAnchor == it->second)
				return false;

			delete(it->second);
			anchors.erase(it);
			return true;
		}
	}
	return false;
}

void AnchorManager::removeAnchor(const Body * body)noexcept
{
	for(auto it = anchors.begin(); it != anchors.end(); it++) {

		//if the anchor is linked to a body
		if(typeid(*it->second) == typeid(AnchorPointBody)) {
			AnchorPointBody * temp = ((AnchorPointBody*)it->second);

			if(temp->getBody() == body) {
				removeAnchor(it->first);
			}
		}
	}
}

void AnchorManager::load(const std::string& path) noexcept
{
	stringHash_t anchorParams;
	std::ifstream fileAnchor (path.c_str(), std::ifstream::in);
	cLog::get()->write("Reading fileAnchor "+ path);
	// std::cout << "Reading fileAnchor " << path.c_str() << std::endl;
	if(fileAnchor) {
		std::string line;
		while(getline(fileAnchor, line)) {
			if (line[0] != '[' ) {
				if (line[0]!='#' && line.size() != 0) {
					int pos = line.find('=',0);
					std::string p1=line.substr(0,pos-1);
					std::string p2=line.substr(pos+2,line.size());
					anchorParams[p1]=p2;
				}
			}
			else {
				if (anchorParams.size() !=0) {
					// std::cout << "in fileAnchor get " << anchorParams["name"] << std::endl;
					addAnchor(anchorParams);
					anchorParams.clear();
				}
			}
		}
		fileAnchor.close();
	}
	else {
		//pour debug
		// std::cout << "Error reading fileAnchor " << path.c_str() << std::endl;
		cLog::get()->write("Error reading fileAnchor", LOG_TYPE::L_ERROR);
	}
}

bool AnchorManager::addAnchor(const std::string& name, Body * b) noexcept
{
	if(b == nullptr) {
		cLog::get()->write("error addAnchor Body nullptr " + name, LOG_TYPE::L_ERROR);
		// std::cout << "error addAnchor Body nullptr " << name << std::endl;
		return false;
	}

	return addAnchor(name, new AnchorPointBody(b));
}

bool AnchorManager::addAnchor(stringHash_t params)
{
	AnchorPoint * anchor = constructAnchor(params);

	if(!params["name"].empty()) {
		// std::cout << "addAnchor params " << params["name"] << std::endl;
		return addAnchor(params["name"], anchor);
	}
	return false;
}

void AnchorManager::update() noexcept
{
	if(currentAnchor != nullptr) {

		if(!moving)
			currentAnchor->update();
		else {

			setCurrentAnchorPos(getTravelPosition(timeMgr->getJDay()));

			if(timeMgr->getJDay() >= arrivalTime) {
				moving = false;
				return;
			}
		}

		if(typeid(*currentAnchor) == typeid(AnchorPointBody)){

			AnchorPointBody * temp = (AnchorPointBody*)currentAnchor;
			const Body * body = temp->getBody();
			double radius = body->getRadius();

			if(followRotation && observer->getAltitude() > radius * rotationMultiplierCondition * AU *1000.0){
				followRotation = false;
				lastUpdate = timeMgr->getJDay();
				return;
			}

			if(!followRotation && observer->getAltitude() < radius * rotationMultiplierCondition * AU *1000.0){
				followRotation = true;
			}

			if(!followRotation || overrideRotationCondition){
				double correction;

				correction = 
					body->getSiderealTime(lastUpdate) - 
					body->getSiderealTime(timeMgr->getJDay());

				observer->setLongitude(observer->getLongitude() + correction);
				lastUpdate = timeMgr->getJDay();
			}
		}

	}
}

void AnchorManager::initFirstAnchor(const std::string& anchorName) noexcept
{
	bool result = this->switchToAnchor(anchorName);
	if (result)
		cLog::get()->write("AnchorManager:: initFirtAnchor to "+ anchorName);
	else {
		cLog::get()->write("AnchorManager:: error initFirtAnchor to "+ anchorName);
		exit(-1);
	}
}

bool AnchorManager::switchToAnchor(const std::string& anchorName) noexcept
{
	if(moving) {
		cLog::get()->write("AnchorManager:: error can't switch anchor when moving", LOG_TYPE::L_ERROR);
		return false;
	}

	auto it = anchors.find(anchorName);

	if(it != anchors.end()) {
		currentAnchor = it->second;
		observer->setAnchorPoint(it->second);
		// std::cout << "switchToAnchor: changement fait pour " << anchorName  << std::endl;
		return true;
	}

	// std::cout << "switchToAnchor: erreur de changement pour " << anchorName << std::endl;
	return false;
}

bool AnchorManager::setCurrentAnchorPos(const Vec3d& pos) noexcept
{
	if(typeid(*currentAnchor) != typeid(AnchorPointBody)) {
		currentAnchor->setHeliocentricEclipticPos(pos);
		return true;
	}
	cLog::get()->write("AnchorManager:: error can't move when on a body", LOG_TYPE::L_ERROR);
	return false;
}

bool AnchorManager::moveRelativeXYZ(double x, double y, double z) noexcept
{
	Vec3d newPos = currentAnchor->getHeliocentricEclipticPos() + Vec3d(x,y,z);
	setCurrentAnchorPos(newPos);
	// std::cout << "Modification de " << x <<" "<< y <<" "<< z<< std::endl;
	return true;
}

bool AnchorManager::moveTo(const Vec3d& pos, double time)
{

	if(moving) {
		cLog::get()->write("AnchorManager:: error already moving to a destination", LOG_TYPE::L_ERROR);
		return false;
	}

	if(typeid(*currentAnchor) == typeid(AnchorPointBody)) {
		cLog::get()->write("AnchorManager:: error cannot travel when on a body", LOG_TYPE::L_ERROR);
		return false;
	}

	if(time < 0) {
		cLog::get()->write("AnchorManager:: error negative time", LOG_TYPE::L_ERROR);
		return false;
	}

	if(time == 0) {
		setCurrentAnchorPos(pos);
		return true;
	}

	// std::cout << "moving to " << pos << std::endl;

	time = time / (24 * 60 * 60);

	moving = true;
	startPosition = currentAnchor->getHeliocentricEclipticPos();
	startTime = timeMgr->getJDay();
	direction = pos - currentAnchor->getHeliocentricEclipticPos();
	distanceToTavel = direction.length();
	direction.normalize();
	travelTime = time;
	arrivalTime = startTime + time;

	return true;
}

bool AnchorManager::moveTo(const AnchorPointBody * targetAnchor, double time, double alt)
{
	if(moving) {
		cLog::get()->write("AnchorManager:: error already moving to a destination", LOG_TYPE::L_ERROR);
		return false;
	}

	if(typeid(*currentAnchor) == typeid(AnchorPointBody)) {
		cLog::get()->write("AnchorManager:: error cannot travel when on a body", LOG_TYPE::L_ERROR);
		return false;
	}

	if(time < 0) {
		cLog::get()->write("AnchorManager:: error negative time", LOG_TYPE::L_ERROR);
		return false;
	}

	time = time / (24 * 60 * 60);

	Vec3d target = targetAnchor->getBody()->getPositionAtDate(timeMgr->getJDay() + time);

	moving = true;
	startPosition = currentAnchor->getHeliocentricEclipticPos();
	startTime = timeMgr->getJDay();
	travelTime = time;
	arrivalTime = startTime + time;

	//get the direction
	direction = target - currentAnchor->getHeliocentricEclipticPos();
	direction.normalize();

	double distanceFromPlanet = alt < 0 ? targetAnchor->getBody()->getRadius() * 5 : targetAnchor->getBody()->getRadius() + alt/AU;

	target =
	    target - currentAnchor->getHeliocentricEclipticPos()
	    - direction * distanceFromPlanet;

	distanceToTavel = target.length();

	// std::cout << "moving to " << target << std::endl;

	return true;
}

bool AnchorManager::moveToBody(const std::string& bodyName, double time, double alt)
{

	auto it = anchors.find(bodyName);

	if(it == anchors.end())
		return false;

	if(typeid(*it->second) != typeid(AnchorPointBody))
		return false;

	AnchorPointBody * target = ((AnchorPointBody*)it->second);

	return moveTo(target, time, alt);
}

bool AnchorManager::transitionToPoint(const std::string& name)
{

	auto it = anchors.find(name);

	if(it == anchors.end()) {

		AnchorPoint * target = new AnchorPoint();
		addAnchor(name, target);
		return transitionToPoint(target);
	}

	if(typeid(*it->second) != typeid(AnchorPoint)) {
		// @TODO passage via cLog
		// std::cout << "pas un AnchorPoint" << std::endl;
		cLog::get()->write("AnchorManager::transitionToPoint: not an AnchorPoint "+name, LOG_TYPE::L_ERROR);
		return false;
	}

	AnchorPoint * target = ((AnchorPoint*)it->second);

	return transitionToPoint(target);
}

bool AnchorManager::transitionToPoint(AnchorPoint * targetPoint)
{

	Vec3d obsPos = observer->getHeliocentricPosition(timeMgr->getJDay());
	Mat4d obsLocalToEquatorial = observer->getRotLocalToEquatorial(timeMgr->getJDay());
	Mat4d obsEquatorialToVsop87 = observer->getRotEquatorialToVsop87();

	observer->setAltitude(0);

	targetPoint->setHeliocentricEclipticPos(obsPos);
	targetPoint->setRotLocalToEquatorial(obsLocalToEquatorial);
	targetPoint->setRotEquatorialToVsop87(obsEquatorialToVsop87);

	currentAnchor = targetPoint;
	observer->setAnchorPoint(currentAnchor);

	// std::cout << "transition to point done" << std::endl;
	return true;

}

bool AnchorManager::transitionToBody(AnchorPointBody * targetBody)
{

	//Note : we used a dichotomy for a lack of a better way to calculate longitude and latitude

	Vec3d obsPos = currentAnchor->getHeliocentricEclipticPos();

	Vec3d bodyPos = targetBody->getBody()->get_heliocentric_ecliptic_pos();
	
	double planetRadius = targetBody->getBody()->getRadius() * AU * 1000;

	double alt = (obsPos-bodyPos).length() * AU * 1000 - planetRadius;

	currentAnchor = targetBody;
	observer->setAnchorPoint(targetBody);
	targetBody->update();

	observer->setAltitude(alt);
	observer->setLatitude(0);

	Vec3d posLower;
	Vec3d posUpper;
	double lower;
	double upper;
	double interval = 90;
	
	//decide wich half of the circle we start on
	observer->setLongitude(-90);		
	navigator->updateTransformMatrices(observer, timeMgr->getJDay());
	posLower = observer->getHeliocentricPosition(timeMgr->getJDay());

	observer->setLongitude(90);
	navigator->updateTransformMatrices(observer, timeMgr->getJDay());
	posUpper = observer->getHeliocentricPosition(timeMgr->getJDay());

	double distLower = (posLower - obsPos).lengthSquared();
	double distUpper = (posUpper - obsPos).lengthSquared();


	if( distLower < distUpper ){
		lower = -180;
		upper = 0;
	}
	else{
		lower = 0;
		upper = 180;
	}

	while(interval > 0.00001){
		
		observer->setLongitude(lower);	
		navigator->updateTransformMatrices(observer, timeMgr->getJDay());
		posLower = observer->getHeliocentricPosition(timeMgr->getJDay());

		observer->setLongitude(upper);
		navigator->updateTransformMatrices(observer, timeMgr->getJDay());
		posUpper = observer->getHeliocentricPosition(timeMgr->getJDay());

		distLower = (posLower - obsPos).lengthSquared();
		distUpper = (posUpper - obsPos).lengthSquared();

		if( distLower < distUpper ){
			upper -= interval;
		}
		else{
			lower += interval;
		}

		interval = interval / 2;

	}

	double longitude = (lower + upper)/2;

	observer->setLongitude(longitude);

	
	lower = -90;
	upper = 90;
	interval = 90;

	while(interval > 0.01){

		observer->setLatitude(lower);	
		navigator->updateTransformMatrices(observer, timeMgr->getJDay());
		posLower = observer->getHeliocentricPosition(timeMgr->getJDay());

		observer->setLatitude(upper);
		navigator->updateTransformMatrices(observer, timeMgr->getJDay());
		posUpper = observer->getHeliocentricPosition(timeMgr->getJDay());

		distLower = (posLower - obsPos).lengthSquared();
		distUpper = (posUpper - obsPos).lengthSquared();

		if( distLower < distUpper ){
			upper -= interval;
		}
		else{
			lower += interval;
		}

		interval = interval/2;
	}

	observer->setLatitude( (lower+upper)/2 );
	navigator->setHeading(0);

	return true;

}

bool AnchorManager::transitionToBody(const std::string& name)
{
	auto it = anchors.find(name);

	if(it == anchors.end()) {
		return false;
	}

	if(typeid(*it->second) != typeid(AnchorPointBody)) {
		// @TODO passage via cLog
		// std::cout << "pas un body" << std::endl;
		cLog::get()->write("AnchorManager::transitionToPoint: not a body "+name, LOG_TYPE::L_ERROR);
		return false;
	}

	AnchorPointBody * target = ((AnchorPointBody*)it->second);
	// std::cout << "changement vers " << name << std::endl;

	return transitionToBody(target);
}

bool AnchorManager::saveCameraPosition(const std::string& fileName)
{

	std::ofstream fileAnchor (fileName.c_str(), std::ofstream::trunc);
	std::ostringstream os;

	if (!fileAnchor.is_open()) {
		cLog::get()->write("AnchorManager::saveCameraPosition couldn't create a file "+fileName, LOG_TYPE::L_WARNING);
		return false;
	}

	bool stop = false;
	std::string name;

	for(auto it = anchors.begin(); it != anchors.end() && !stop; it++) {
		if(it->second == currentAnchor) {
			name = it->first;
			stop = true;
		}
	}

	if(!stop) {
		cLog::get()->write("AnchorManager::saveCameraPosition couldn't find current anchor in "+fileName, LOG_TYPE::L_WARNING);
		return false;
	}

	os << "name " << name << std::endl;
	os << currentAnchor->saveAnchor();
	os << "alt " << observer->getAltitude() << std::endl;
	os << "lon " << observer->getLongitude() << std::endl;
	os << "lat " << observer->getLatitude() << std::endl;
	os << "time " << timeMgr->getJDay() << std::endl;

	Vec3d vision = navigator->getLocalVision();

	os << "vx " << vision[0] << std::endl;
	os << "vy " << vision[1] << std::endl;
	os << "vz " << vision[2] << std::endl;

	std::string str = os.str();
	fileAnchor.write(str.c_str(), str.length());
	fileAnchor.close();

	return true;
}

bool AnchorManager::loadCameraPosition(const std::string& fileName)
{

	stringHash_t anchorParams = getAnchorParams(fileName);

	if(anchorParams.empty())
		return false;

	AnchorPoint * anchor = nullptr;

	auto it = anchors.find(anchorParams["name"]);

	if(it != anchors.end())
		anchor = it->second;

	anchor_type type = getAnchorTypeFromString(anchorParams["type"]);

	if(anchor != nullptr) { //we found an anchor with that name
		if(!rightType(anchor, type))
			cLog::get()->write("AnchorManager::loadCameraPosition wrong anchor type", LOG_TYPE::L_WARNING);
	}
	else { //we have to construct the anchor
		anchor = constructAnchor(anchorParams);

		if(anchor == nullptr)
			cLog::get()->write("AnchorManager::loadCameraPosition could not create anchor from given parameters", LOG_TYPE::L_WARNING);
	}

	if(it != anchors.end())
		anchors.insert(std::pair<std::string, AnchorPoint*>(anchorParams["name"], anchor));

	currentAnchor = anchor;
	observer->setAnchorPoint(anchor);

	observer->setAltitude(stod(anchorParams["alt"]));
	observer->setLongitude(stod(anchorParams["lon"]));
	observer->setLatitude(stod(anchorParams["lat"]));

	Vec3d vision(
	    stod(anchorParams["vx"]),
	    stod(anchorParams["vy"]),
	    stod(anchorParams["vz"]));

	navigator->setLocalVision(vision);

	timeMgr->setJDay(stod(anchorParams["time"]));

	cLog::get()->write("anchor loading done", LOG_TYPE::L_WARNING);
	return true;
}

bool AnchorManager::alignCameraToBody(std::string name, double duration){

	AnchorPointBody * anchor = nullptr;

	auto it = anchors.find(name);

	if(it == anchors.end()){
		cLog::get()->write("AnchorManager::alignCameraToBody anchor not found", LOG_TYPE::L_WARNING);
		return false;
	}

	if(!rightType(it->second, anchor_type::body)){
		cLog::get()->write("AnchorManager::alignCameraToBody wrong anchor type", LOG_TYPE::L_WARNING);
		return false;
	}
		
	anchor = (AnchorPointBody*)(it->second);
	
	Mat4d rot = anchor->getRotEquatorialToVsop87();

	navigator->alignUpVectorTo(rot, duration);

	return true;
}

