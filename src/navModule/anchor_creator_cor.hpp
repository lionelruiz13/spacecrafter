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
//! \file anchor_creator_cor.hpp
//! \brief Chain of responsability for creating anchors from given parameters
//! \author Julien LAFILLE
//! \date may 2018

/*
 * Each member of the chain can handle the creation of an anchorType from parameters
 * If the chain link can handle the creation of the anchor it returns a new anchor
 * Overwise it passes the parameters to the next chain link
 * If there the parameters reach the end of the chain a null_ptr is returned
 * 
 */

#include "tools/utility.hpp"
#include <memory>

class AnchorPoint;
class ProtoSystem;
class TimeMgr;
class OrbitCreator;

/*
 * Base class for implementing chain links 
 */
class AnchorCreator {
public:
	AnchorCreator() = delete;
	AnchorCreator(const AnchorCreator * _next) {
		next = _next;
	}

	virtual std::shared_ptr<AnchorPoint> handle(stringHash_t param)const = 0;

protected:
	const AnchorCreator * next = nullptr;
};

class AnchorPointCreator : public AnchorCreator {
public:
	AnchorPointCreator() = delete;
	AnchorPointCreator(const AnchorCreator* _next);
	std::shared_ptr<AnchorPoint> handle(stringHash_t param)const;
};

class AnchorObservatoryCreator : public AnchorCreator {
public:
	AnchorObservatoryCreator() = delete;
	AnchorObservatoryCreator(const AnchorCreator* _next);
	std::shared_ptr<AnchorPoint> handle(stringHash_t param)const;
};

class AnchorPointBodyCreator : public AnchorCreator {
public:
	AnchorPointBodyCreator() = delete;
	AnchorPointBodyCreator(const AnchorCreator* _next, const ProtoSystem * ssystem);
	std::shared_ptr<AnchorPoint> handle(stringHash_t param)const;

private :
	const ProtoSystem * ssystem;
};

class AnchorPointOrbitCreator : public AnchorCreator {
public:
	AnchorPointOrbitCreator() = delete;
	AnchorPointOrbitCreator(const AnchorCreator* _next, const ProtoSystem * ssystem, const TimeMgr * timeMgr, std::shared_ptr<OrbitCreator> orbitCreator);
	std::shared_ptr<AnchorPoint> handle(stringHash_t param)const;

private :
	const ProtoSystem * ssystem;
	const TimeMgr * timeMgr;
	std::shared_ptr<OrbitCreator> orbitCreator;
};
