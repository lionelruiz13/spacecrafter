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
#include "bodyModule/ssystem_factory.hpp"
#include "coreModule/time_mgr.hpp"
#include "navModule/anchor_creator_cor.hpp"
#include "navModule/anchor_manager.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_body.hpp"
#include "navModule/anchor_point_orbit.hpp"
#include "navModule/anchor_point_observatory.hpp"
#include "navModule/observer.hpp"
#include "navModule/navigator.hpp"
//#include "tools/fmath.hpp"
#include "tools/sc_const.hpp"
#include "tools/log.hpp"
#include "eventModule/CoreEvent.hpp"
#include "eventModule/event_recorder.hpp"

double AnchorManager::lastCorrection = 0;

/*
 * returns the position for a movement at given date
 *
 * The trajectory followed is a straight line between two points, however the speed is a logistic function.
 * This allows the speed to be low at the start and finish, but hight in between.
 *
 */
Vec3d AnchorManager::getTravelPosition(double JD)
{

	if(JD > arrivalTime)
		JD = arrivalTime;

	if(JD < startTime)
		JD = startTime;

	double s = 1;
	double u = 2;
	double x = (((JD-startTime)/travelTime)*25)-5;

	double logistic = 1/(1 + exp(-(x-u)/s));
	double movement = distanceToTavel * logistic;

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
static bool rightType(std::shared_ptr<AnchorPoint> anchor, const AnchorManager::anchor_type & type)
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

std::shared_ptr<AnchorPoint> AnchorManager::constructAnchor(stringHash_t params)
{
	return anchorCreator->handle(params);
}

AnchorManager::AnchorManager(
    Observer * obs, Navigator * nav,
    ProtoSystem * _ssystem,
    TimeMgr * mgr,
    std::shared_ptr<OrbitCreator> orbitCreator) noexcept
{
	observer = obs;
	navigator = nav;
	ssystem = _ssystem;
	//_ssystem->setAnchorManager(this);
	timeMgr = mgr;

	AnchorCreator * observatory = new AnchorObservatoryCreator(nullptr);
	AnchorCreator * orbit = new AnchorPointOrbitCreator(observatory,_ssystem, mgr, orbitCreator);
	AnchorCreator * body = new AnchorPointBodyCreator(orbit, _ssystem);
	anchorCreator = new AnchorPointCreator(body);
}

AnchorManager::~AnchorManager()
{
	//for(auto it = anchors.begin(); it != anchors.end(); it++)
	//	delete(it->second);

	anchors.clear();
}

bool AnchorManager::addAnchor(const std::string& name, std::shared_ptr<AnchorPoint> anchor) noexcept
{
	auto it = anchors.find(name);
	if(it != anchors.end()) //name already present in container
		return false;

	if(anchor == nullptr) {
		// @TODO pass it in log
		cLog::get()->write("error addAnchor AnchorPoint nullptr " + name, LOG_TYPE::L_ERROR);
		// std::cout << "error addAnchor AnchorPoint nullptr " << name << std::endl;
		return false;
	}

	anchors.insert(std::pair<std::string, std::shared_ptr<AnchorPoint>>(name, anchor));
	return true;
}

bool AnchorManager::removeAnchor(const std::string& name) noexcept
{
	for(auto it = anchors.begin(); it != anchors.end(); it++) {
		if(it->first == name) {

			if(currentAnchor == it->second)
				return false;

			//delete(it->second);
			anchors.erase(it);
			return true;
		}
	}
	return false;
}

void AnchorManager::removeAnchor(std::shared_ptr<Body> body)noexcept
{
	for(auto it = anchors.begin(); it != anchors.end(); it++) {

		//if the anchor is linked to a body
		if(typeid(*it->second) == typeid(AnchorPointBody)) {
			std::shared_ptr<AnchorPointBody> temp = std::dynamic_pointer_cast<AnchorPointBody>(it->second);

			if(temp->getBody() == body) {
				removeAnchor(it->first);
				return; // @TODO Olivier : why ?
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
		//for debug
		// std::cout << "Error reading fileAnchor " << path.c_str() << std::endl;
		cLog::get()->write("Error reading fileAnchor", LOG_TYPE::L_ERROR);
	}
}

bool AnchorManager::addAnchor(const std::string& name, std::shared_ptr<Body> b) noexcept
{
	if(b == nullptr) {
		cLog::get()->write("error addAnchor Body nullptr " + name, LOG_TYPE::L_ERROR);
		// std::cout << "error addAnchor Body nullptr " << name << std::endl;
		return false;
	}

	return addAnchor(name, std::make_shared<AnchorPointBody>(b));
}

bool AnchorManager::addAnchor(stringHash_t params)
{
	std::shared_ptr<AnchorPoint> anchor = constructAnchor(params);

	if(!params["name"].empty()) {
		// std::cout << "addAnchor params " << params["name"] << std::endl;
		return addAnchor(params["name"], anchor);
	}
	return false;
}

void AnchorManager::update() noexcept
{
	if (observer->getEyeRelativeMode()) {
		// Assume the nearest anchor doesn't change when the observer doesn't move
		const Vec3d center = observer->getObserverCenterPoint();
		if (center != lastCenterPosition) {
			lastCenterPosition = center;
			// Set the nearest anchor to the observer
			double bestFit = observer->getDistanceFromCenter();
			bestFit *= bestFit;
			for (auto &a : anchors) {
				if (a.second->isOnBody()) // Otherwise it crash...
					a.second->update();
				double tmp = (a.second->getHeliocentricEclipticPos() - center).lengthSquared();
				if (tmp < bestFit) {
					bestFit = tmp;
					currentAnchor = a.second;
					observer->setAnchorPoint(currentAnchor);
				}
			}
		}
	} else if (currentAnchor != nullptr) {

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

			std::shared_ptr<AnchorPointBody> temp = std::dynamic_pointer_cast<AnchorPointBody>(currentAnchor);
			const std::shared_ptr<Body> body = temp->getBody();
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

				lastCorrection = body->getSiderealTime(timeMgr->getJDay());
				correction = body->getSiderealTime(lastUpdate) - lastCorrection;

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
		// std::cout << "switchToAnchor: change made to " << anchorName  << std::endl;
		return true;
	}

	// std::cout << "switchToAnchor: change error for " << anchorName << std::endl;
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
	// std::cout << "Modification of " << x <<" "<< y <<" "<< z<< std::endl;
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

bool AnchorManager::moveTo(std::shared_ptr<AnchorPointBody> targetAnchor, double time, double alt)
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

	std::shared_ptr<AnchorPointBody> target = std::dynamic_pointer_cast<AnchorPointBody>(it->second);

	return moveTo(target, time, alt);
}

bool AnchorManager::transitionToPoint(const std::string& name)
{

	auto it = anchors.find(name);

	if(it == anchors.end()) {

		std::shared_ptr<AnchorPoint> target = std::make_shared<AnchorPoint>();
		addAnchor(name, target);
		return transitionToPoint(target);
	}

	if(typeid(*it->second) != typeid(AnchorPoint)) {
		// @TODO passage via cLog
		// std::cout << "not a AnchorPoint" << std::endl;
		cLog::get()->write("AnchorManager::transitionToPoint: not an AnchorPoint "+name, LOG_TYPE::L_ERROR);
		return false;
	}

	std::shared_ptr<AnchorPoint> target = std::dynamic_pointer_cast<AnchorPoint>(it->second);

	return transitionToPoint(target);
}

bool AnchorManager::transitionToPoint(std::shared_ptr<AnchorPoint> targetPoint)
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

bool AnchorManager::transitionToBody(std::shared_ptr<AnchorPointBody> targetBody)
{

	//Note : we used a dichotomy for a lack of a better way to calculate longitude and latitude

	Vec3d obsPos = currentAnchor->getHeliocentricEclipticPos();

	Vec3d bodyPos = targetBody->getBody()->get_heliocentric_ecliptic_pos();

	double planetRadius = targetBody->getBody()->getRadius() * AU * 1000;

	double alt = (obsPos-bodyPos).length() * AU * 1000 - planetRadius;


	currentAnchor = targetBody;
	observer->setAnchorPoint(targetBody);
	Event* event= new ObserverEvent(targetBody->getBody()->getEnglishName());
	EventRecorder::getInstance()->queue(event);
	targetBody->update();

	double angle = targetBody->getBody()->getAxisAngle()*(180.0f/M_PI);

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

	//angle to heading
	// if (angle > 180) angle = -(-180 + (angle - 180)); // angle = 360 - angle
	// if (angle < -180) angle = 180 + (angle + 180); // angle += 360
	angle -= floor((angle + 180.) / 360.) * 360.;

	//set the heading to the same angle that the planet was originally seen at
	navigator->setHeading(-angle);

	//Changethe heading to 0 gradually so that the planet axis is vertical
	navigator->changeHeading(0, 5000);

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
		// std::cout << "not a body" << std::endl;
		cLog::get()->write("AnchorManager::transitionToPoint: not a body "+name, LOG_TYPE::L_ERROR);
		return false;
	}

	std::shared_ptr<AnchorPointBody> target = std::dynamic_pointer_cast<AnchorPointBody>(it->second);
	// std::cout << "change to " << name << std::endl;

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

	std::shared_ptr<AnchorPoint> anchor = nullptr;

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
		anchors.insert(std::pair<std::string, std::shared_ptr<AnchorPoint>>(anchorParams["name"], anchor));

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

bool AnchorManager::alignCameraToBody(std::string name, double duration)
{
	std::shared_ptr<AnchorPointBody> anchor = nullptr;

	auto it = anchors.find(name);

	if(it == anchors.end()){
		cLog::get()->write("AnchorManager::alignCameraToBody anchor not found", LOG_TYPE::L_WARNING);
		return false;
	}

	if(!rightType(it->second, anchor_type::body)){
		cLog::get()->write("AnchorManager::alignCameraToBody wrong anchor type", LOG_TYPE::L_WARNING);
		return false;
	}

	anchor = std::dynamic_pointer_cast<AnchorPointBody>(it->second);

	Mat4d rot = anchor->getRotEquatorialToVsop87();

	navigator->alignUpVectorTo(rot, duration);

	return true;
}


void AnchorManager::displayAnchor() const
{
	std::cout << " All Anchor ------------------------------" <<std::endl;
	for (auto it : anchors) {
		std::cout << it.first << " - " << it.second->isOnBody() << " - " << it.second->getHeliocentricEclipticPos() << std::endl;
	}
	std::cout << " -----------------------------------------" <<std::endl;
}

void AnchorManager::selectAnchor()
{
	auto srcAnchor = observer->getAnchorPoint();
	Mat4d srcMat;
	if (srcAnchor)
		srcMat = observer->getRotEquatorialToVsop87();
	// Cancel last compensation
	auto longitude = observer->getLongitude() + lastCorrection;
	if (currentAnchor) {
		observer->setAnchorPoint(currentAnchor);
	} else {
		if (ssystem->getCenterObject())
			switchToAnchor(ssystem->getCenterObject()->getEnglishName());
		else if (!switchToAnchor("Sun")) {
			cLog::get()->write("Switch to system without center, which is invalid\n", LOG_TYPE::L_ERROR);
			return;
		}
	}
	if (srcAnchor) { // Move to the new axis
		auto quaternion = (currentAnchor->getRotEquatorialToVsop87().transpose() * srcMat).toQuaternion();
		observer->setRelRotation(quaternion);
		observer->moveTo({1, 0, 0, 0}, 10000, true);
	}
	auto tmp = dynamic_cast<AnchorPointBody *>(observer->getAnchorPoint().get());
	lastUpdate = timeMgr->getJDay();
	lastCorrection = (tmp) ? tmp->getBody()->getSiderealTime(lastUpdate) : 0;
	observer->setLongitude(longitude - lastCorrection);
	navigator->updateTransformMatrices(observer, lastUpdate);
}

std::string AnchorManager::querySelectedAnchorName() const
{
	for (auto &p : anchors) {
		if (p.second == currentAnchor)
			return p.first;
	}
	return {};
}
