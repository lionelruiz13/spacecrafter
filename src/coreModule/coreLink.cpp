# include "coreModule/coreLink.hpp"

CoreLink::CoreLink(Core * _core)
{
    core= _core;
}

CoreLink::~CoreLink()
{}


void CoreLink::skyLineMgrSetColor(SKYLINE_TYPE name, Vec3f a) {
	core->skyLineMgr->setColor(name, a);
}