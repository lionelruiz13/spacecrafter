# include "coreModule/coreLink.hpp"
#include "tools/app_settings.hpp"

CoreLink::CoreLink(Core * _core)
{
    core= _core;
}

CoreLink::~CoreLink()
{}

bool CoreLink::cameraSave(const std::string& name){
	return core->anchorManager->saveCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + name);
}
	
bool CoreLink::loadCameraPosition(const std::string& filename){
	return core->anchorManager->loadCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + filename);
}