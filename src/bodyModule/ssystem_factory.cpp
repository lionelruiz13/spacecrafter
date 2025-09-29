/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <memory>

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_observatory.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"

SSystemFactory::SSystemFactory(Observer *observatory, Navigator *navigation, TimeMgr *timeMgr)
{
    // creation of 3D models for planets
    objLMgr = std::make_unique<ObjLMgr>();
	objLMgr -> setDirectoryPath(AppSettings::Instance()->getModel3DDir() );
	// objLMgr->insertDefault("Sphere");

	if (!objLMgr->checkDefaultObject()) {
		cLog::get()->write("SolarSystem: no default objMgr loaded, system aborded", LOG_TYPE::L_ERROR);
		exit(-7);
	}

    ssystem = std::make_unique<SolarSystem>(objLMgr.get(), observatory, navigation, timeMgr);
    currentSystem = ssystem.get();
    ssystemColor = std::make_unique<SolarSystemColor>(ssystem.get());
    ssystemTex = std::make_unique<SolarSystemTex>(ssystem.get());
    ssystemSelected = std::make_unique<SolarSystemSelected>(ssystem.get());
    ssystemScale = std::make_unique<SolarSystemScale>(ssystem.get());
    ssystemDisplay = std::make_unique<SolarSystemDisplay>(ssystem.get());

    stellarSystem = std::make_unique<ProtoSystem>(objLMgr.get(), observatory, navigation, timeMgr);

    galacticSystem = std::make_unique<ProtoSystem>(objLMgr.get(), observatory, navigation, timeMgr);
    galacticAnchorMgr = galacticSystem->getAnchorManager();
    bodytrace= std::make_shared<BodyTrace>();

    // Don't forget this-> for class variables with name of local variables
    this->observatory = observatory;
    this->navigation = navigation;
    this->timeMgr = timeMgr;
}

SSystemFactory::~SSystemFactory()
{
	//delete bodytrace;
}

void SSystemFactory::reloadColors(const std::string& planetfile) {
    // {"PlanetName": {"colorName": colorVec3f}}
    std::map<std::string, std::map<std::string, Vec3f>> colorMap;

    // {"PlanetName": "nameValue"}
    std::map<std::string, std::string> nameMap;

    std::ifstream file(planetfile);

    if (file) {
        std::string line;
        std::string currentBody;
        while (std::getline(file, line)) {
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (line.empty()) {
                continue; // Skip empty lines
            }

            // Remove comments
            size_t commentPos = line.find('#');
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
            }

            if (line[0] == '[') {
                // New body section
                size_t endPos = line.find(']');
                if (endPos != std::string::npos) {
                    currentBody = line.substr(1, endPos - 1);
                }
                continue;
            }

            if (!line.starts_with("label_color") &&
                !line.starts_with("orbit_color") &&
                !line.starts_with("trail_color") &&
                !line.starts_with("color") && // halo
                !line.starts_with("name")) {
                continue; // Skip non-color lines and non-name lines
            }

            // Parse the line for planet colors
            std::istringstream iss(line);
            std::string propertyName;
            int equalPos = line.find('=');
            if (equalPos != std::string::npos) {
                // Extract property name and trim whitespace
                propertyName = line.substr(0, equalPos);
                propertyName.erase(propertyName.find_last_not_of(" \t") + 1);

                // Ensure we have a current body and property name
                if (propertyName.empty() || currentBody.empty()) {
                    continue;
                }

                if (propertyName == "name") {
                    // Extract name value and trim whitespace
                    std::string nameValue = line.substr(equalPos + 1);
                    nameValue.erase(0, nameValue.find_first_not_of(" \t"));
                    nameValue.erase(nameValue.find_last_not_of(" \t") + 1);
                    nameMap[currentBody] = nameValue;
                } else {
                    // Parse the RGB values
                    Vec3f colorValue;
                    std::string values = line.substr(equalPos + 1);
                    std::replace(values.begin(), values.end(), ',', ' ');
                    std::istringstream valueStream(values);
                    float r, g, b;
                    if (valueStream >> r >> g >> b) {
                        colorValue = Vec3f(r, g, b);
                        colorMap[currentBody][propertyName] = colorValue;
                    }
                }
            }
        }
    }
    file.close();

    // Apply the colors to the bodies
    for (const auto& [planetName, colors] : colorMap) {
        for (const auto& [colorName, colorValue] : colors) {
            // Check if nameMap has an entry for this planet
            if (nameMap.find(planetName) == nameMap.end()) {
                cLog::get()->write("Warning: No name found for planet '" + planetName + "' in " + planetfile, LOG_TYPE::L_WARNING);
                continue; // Skip if no name found
            }

            if (colorName == "color") {
                ssystemColor->setBodyColor(nameMap[planetName], "halo", colorValue);
                continue;
            }

            // Remove the _color suffix
            std::string baseName = colorName.substr(0, colorName.size() - 6);

            // Set the color using the name from nameMap
            ssystemColor->setBodyColor(nameMap[planetName], baseName, colorValue);
        }
    }
}

void SSystemFactory::changeSystem(const std::string &mode)
{
    if (mode == "SolarSystem" || mode == "Sun" || mode == "temp_point")
        currentSystem = ssystem.get();
    else {
        try {
            currentSystem = systems.at(mode).get();
        } catch (...) {
            currentSystem = createSystem(mode).get();
        }
    }
    selectSystem();
}

void SSystemFactory::selectSystem()
{
    currentSystem->selectSystem();
    ssystemColor->changeSystem(currentSystem);
    ssystemDisplay->changeSystem(currentSystem);
    ssystemScale->changeSystem(currentSystem);
    ssystemSelected->changeSystem(currentSystem);
    ssystemSelected->setSelected(querySelectedAnchorName());
    ssystemTex->changeSystem(currentSystem);
}

void SSystemFactory::addSystem(const std::string &name, const std::string &file)
{
    auto &system = systems[name];
    system = std::make_unique<ProtoSystem>(objLMgr.get(), observatory, navigation, timeMgr, systemOffsets[name]);
    system->load(file);
}

void SSystemFactory::loadGalacticSystem(const std::string &path, const std::string &name)
{
    stringHash_t params;

    std::ifstream file(path + name);
    if (file) {
        std::string line;
		while(getline(file , line)) {
            if (line.empty() || line.front() == '#')
                continue;
			if (line.front() != '[' ) {
				if (line.back() == '\r')
					line.pop_back();
				auto pos = line.find_first_of('=');
                if (pos != std::string::npos)
					params[line.substr(0,pos-1)] = line.substr(pos+2);
			} else if (!params.empty())
                loadSystem(path, params);
    		}
        if (!params.empty())
            loadSystem(path, params);
		file.close();
    } else {
        galacticAnchorMgr->addAnchor("Sun", std::make_shared<AnchorPointObservatory>(0, 0, 0));
    }
}

void SSystemFactory::loadSystem(const std::string &path, stringHash_t &params)
{
    std::cout << "Params :\n";
    for (auto &p : params) {
        std::cout << p.first << " : " << p.second << '\n';
    }
    params["type"] = "observatory";
    galacticAnchorMgr->addAnchor(params);
    systemOffsets[params["name"]].set(stod(params["x"]), stod(params["y"]), stod(params["z"]));
    if (!params["system"].empty())
        addSystem(params["name"], path + params["system"]);
    params.clear();
}

std::unique_ptr<ProtoSystem> &SSystemFactory::createSystem(const std::string &mode)
{
    stringHash_t params;
    auto pos = observatory->getObserverCenterPoint();
    params["name"] = mode;
    params["type"] = "observatory";
    params["x"] = std::to_string(pos[0]);
    params["y"] = std::to_string(pos[1]);
    params["z"] = std::to_string(pos[2]);
    galacticAnchorMgr->addAnchor(params);
    systemOffsets[mode] = pos;
    auto &system = systems[mode];
    system = std::make_unique<ProtoSystem>(objLMgr.get(), observatory, navigation, timeMgr, systemOffsets[mode]);
    system->load(selected_object);
    return system;
}

std::string SSystemFactory::querySelectedAnchorName()
{
    return currentSystem->getAnchorManager()->querySelectedAnchorName();
}

void SSystemFactory::enterSystem()
{
    if (!inSystem) {
        changeSystem(querySelectedAnchorName());
        inSystem = true;
    }
}

void SSystemFactory::leaveSystem()
{
    if (inSystem) {
        currentSystem = galacticSystem.get();
        selectSystem();
        inSystem = false;
    }
}
