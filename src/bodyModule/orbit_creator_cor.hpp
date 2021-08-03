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
//! \file orbit_creator_cor.hpp
//! \brief Chain of responsability for creating orbits from given parameters
//! \author Julien LAFILLE
//! \date may 2018

#include "tools/utility.hpp"
#include <memory>

class Orbit;
class SolarSystem;
class ProtoSystem;

class OrbitCreator {
public:
	OrbitCreator() = delete;
	OrbitCreator(const OrbitCreator * _next) {
		next = _next;
	}

	virtual std::unique_ptr<Orbit> handle(stringHash_t param)const = 0;

protected:
	const OrbitCreator * next = nullptr;
};

class OrbitCreatorEliptic : public OrbitCreator {
public:
	OrbitCreatorEliptic() = delete;
	OrbitCreatorEliptic(const OrbitCreator * next, const ProtoSystem * ssystem);
	virtual std::unique_ptr<Orbit> handle(stringHash_t params)const;

private :
	const ProtoSystem * psystem;
};

class OrbitCreatorComet : public OrbitCreator {
public:
	OrbitCreatorComet() = delete;
	OrbitCreatorComet(const OrbitCreator * next, const ProtoSystem * ssystem);
	virtual std::unique_ptr<Orbit> handle(stringHash_t params)const;

private :
	const ProtoSystem * psystem;
};

class OrbitCreatorSpecial : public OrbitCreator {
public:
	OrbitCreatorSpecial() = delete;
	OrbitCreatorSpecial(const OrbitCreator *);
	virtual std::unique_ptr<Orbit> handle(stringHash_t params) const;
};

class OrbitCreatorBary : public OrbitCreator {
public:
	OrbitCreatorBary() = delete;
	OrbitCreatorBary(const OrbitCreator *, ProtoSystem * _psystem);
	virtual std::unique_ptr<Orbit> handle(stringHash_t params) const;

private:
	ProtoSystem * psystem;
};
