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
//! \file anchor_point.hpp
//! \brief creates/deletes/updates/switches anchor points for the observer
//! \author Julien LAFILLE
//! \date may 2018

/*
 * This class stores all the anchor points the software uses in a map. 
 * They are identified by a unique name.
 * All interactions with anchor points should be handled in this class.
 * This class contains methods meant to be called by scripts to navigate
 * between anchor points.
 * 
 * This class has a current anchor. This is the anchor the camera is attached to.
 * The current anchor is updated whenever this class is updated. 
 * /!\ Note that other anchor points are not updated; 
 * 
 */

#ifndef ANCHOR_MANAGER_HPP
#define ANCHOR_MANAGER_HPP

#include <string>
#include <map>
#include "tools/utility.hpp"

class ProtoSystem;
class SolarSystem;
class AnchorPoint;
class Observer;
class Navigator;
class Body;
class TimeMgr;
class AnchorPointBody;
class AnchorCreator;
class OrbitCreator;

class AnchorManager {

public:

	enum anchor_type { unknown = 0, point = 1, body = 2, orbit = 3, observatory = 4 };

	AnchorManager() = delete;

	AnchorManager(Observer * obs, Navigator * nav, ProtoSystem * _ssystem, TimeMgr * mgr, std::shared_ptr<OrbitCreator> orbitCreator)noexcept;

	AnchorManager(const AnchorManager&) = delete;

	virtual ~AnchorManager();

	/*
	* adds an achor to the map. does nothing if the name already exists
	* returns true if the anchor is added
	*/
	bool addAnchor(const std::string& name, std::shared_ptr<AnchorPoint> anchor) noexcept;

	//! add a Body from solarsystem to the map .
	//! @warning no control if the name already exist
	bool addAnchor(const std::string& name, std::shared_ptr<Body> b) noexcept;

	/*
	* removes an anchor, does nothing if the anchor doesn't exist
	*/
	bool removeAnchor(const std::string& name)noexcept;

	/*
	 * remove all anchors linked to the body
	 */
	void removeAnchor(std::shared_ptr<Body> body)noexcept;

	/*
	 * loads anchors from ini file
	 */
	void load(const std::string& path)noexcept;

	/*
	 * updates the current anchor
	 */
	void update()noexcept;

	/*
	 * set the observer's anchor point and updates it if it is found, does nothing otherwise
	 */

	void stopMovement() {
		moving = false;
	}

	void resumeMovement() {
		moving = true;
	}

	/*
	 * sets the current anchor
	 * returns true if an anchor matching anchorName is found
	 * returns false otherwise
	 */	
	bool switchToAnchor(const std::string& anchorName) noexcept;

	//! initialize the frist anchor point
	//! @warning returns exit(-1) if no anchor point was found
	void initFirstAnchor(const std::string& anchorName) noexcept;

	/*
	* adds an achor from a collection of strings
	*/
	bool addAnchor(stringHash_t params);

	/*
	 * sets the current anchor's position to the given position
	 * return false if you try to move a AnchorPointBody
	 */
	bool setCurrentAnchorPos(const Vec3d& pos) noexcept;

	/*
	 * adds the given x, y, z to the current anchor's position 
	 */
	bool moveRelativeXYZ(double x, double y, double z) noexcept;

	/*
	* moves the current anchor to pos in the given time
	* can only move the current anchor if it is a fix point
	*/
	bool moveTo(const Vec3d& pos, double time);

	/*
	* moves the current anchor to the given body at given altitude in the given time
	* can only move the current anchor if it is a fix point
	*/
	bool moveTo(std::shared_ptr<AnchorPointBody> target, double time, double alt);

	/*
	* moves the current anchor to pos in the given time
	* can only move the current anchor if it is a fix point
	*/
	bool moveToBody(const std::string& bodyName, double time, double alt);

	/*
	* set the current anchor to a fix point at the observer's position
	*/
	bool transitionToPoint(std::shared_ptr<AnchorPoint> targetPoint);

	/*
	* set the current anchor to a fix point at the observer's position
	*/
	bool transitionToPoint(const std::string& name);

	/*
	* set the current anchor to an anchor attached to the given AnchorPointBody
	*/
	bool transitionToBody(std::shared_ptr<AnchorPointBody> targetBody);

	bool transitionToBody(const std::string& name);

	/*
	* saves the current anchor, time and vision vector
	*/
	bool saveCameraPosition(const std::string& fileName);

	/*
	* loads the given anchor, time and vision vector from given file
	*/
	bool loadCameraPosition(const std::string& fileName);

	/*
	 * returns an anchor constructed from given parameters
	 */
	std::shared_ptr<AnchorPoint> constructAnchor(stringHash_t params);

	/*
	 * sets the follow variable of given anchor point
	 */
	bool setFollowRotation(bool follow) {
		overrideRotationCondition = !follow;

		return true;
	}
	/*
	 * fixe le coefficient sur l'altitude pour suspendre la rotation de l'ancre
	 */
	bool setRotationMultiplierCondition(float v) noexcept {
		if (v>1.0)
			rotationMultiplierCondition = v;
		//~ std::cout << "limit for rotation " << rotationMultiplierCondition << std::endl;
		return true;
	}

	/*
	 *	This fuction aligns the up vector from the camera with the planet's axis
	 */
	bool alignCameraToBody(std::string name, double duration);

	void displayAnchor() const;

	/*
	 *  Select the current anchor of this anchor manager
	 */
	void selectAnchor();

	/*
	 * Query the name of the currently selected anchor
	 */
	std::string querySelectedAnchorName() const;
private:
	//! returns the position the observer should be at the JD date
	/*
	 * uses a cos function to interpolate the trajectory
	 */
	Vec3d getTravelPosition(double JD);

	Observer * observer = nullptr;
	Navigator * navigator = nullptr;
	const ProtoSystem * ssystem = nullptr;
	TimeMgr * timeMgr = nullptr;
	const AnchorCreator * anchorCreator = nullptr;

	std::map<std::string, std::shared_ptr<AnchorPoint>> anchors; //container for anchor points
	std::shared_ptr<AnchorPoint> currentAnchor = nullptr;

	bool moving = false;
	Vec3d direction;
	Vec3d startPosition;
	double startTime;
	double travelTime;
	double distanceToTavel;
	double arrivalTime;

	bool followRotation = true;
	bool overrideRotationCondition = false;
	float rotationMultiplierCondition = 5.0f;
	double lastUpdate;
	static double lastCorrection;
};

#endif
