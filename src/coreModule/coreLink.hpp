#ifndef CORELINK_HPP
#define CORELINK_HPP

#include "coreModule/core.hpp"


class CoreLink {
public: 



	////////////////////////////////////////////////////////////////////////////////
	// Skyline et Skygrid---------------------------
	////////////////////////////////////////////////////////////////////////////////
    void skyLineMgrSetColor(SKYLINE_TYPE name, Vec3f a) {
	    core->skyLineMgr->setColor(name, a);
    };

    void skyGridMgrSetColor(SKYGRID_TYPE name, Vec3f a) {
		core->skyGridMgr->setColor(name, a);
	}

	const Vec3f& skyLineMgrGetColor(SKYLINE_TYPE name) {
		return core->skyLineMgr->getColor(name);
	}

	const Vec3f& skyGridMgrGetColor(SKYGRID_TYPE name) {
		return core->skyGridMgr->getColor(name);
	}

	void skyLineMgrFlipFlagShow(SKYLINE_TYPE name) {
		core->skyLineMgr->flipFlagShow(name);
	}

	void skyGridMgrFlipFlagShow(SKYGRID_TYPE name) {
		core->skyGridMgr->flipFlagShow(name);
	}

	void skyLineMgrSetFlagShow(SKYLINE_TYPE name, bool value) {
		core->skyLineMgr->setFlagShow(name, value);
	}

	void skyGridMgrSetFlagShow(SKYGRID_TYPE name, bool value) {
		core->skyGridMgr->setFlagShow(name, value);
	}

	bool skyLineMgrGetFlagShow(SKYLINE_TYPE name) {
		return core->skyLineMgr->getFlagShow(name);
	}

	bool skyGridMgrGetFlagShow(SKYGRID_TYPE name) {
		return core->skyGridMgr->getFlagShow(name);
	}


    CoreLink(Core* _core);
    ~CoreLink();

private: 
    Core *core=nullptr;

};

#endif