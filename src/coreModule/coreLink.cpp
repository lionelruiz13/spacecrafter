# include "coreModule/coreLink.hpp"
#include "tools/app_settings.hpp"
#include "coreModule/coreFont.hpp"

CoreLink::CoreLink(Core * _core)
{
    core= _core;
}

CoreLink::~CoreLink()
{}

bool CoreLink::cameraSave(const std::string& name)
{
	return core->anchorManager->saveCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + name);
}
	
bool CoreLink::loadCameraPosition(const std::string& filename)
{
	return core->anchorManager->loadCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + filename);
}

void CoreLink::fontUpdateFont(const std::string& _moduleName, const std::string& _fontName, const std::string& _sizeValue)
{
	core->coreFont->updateFont(_moduleName, _fontName, _sizeValue);
}