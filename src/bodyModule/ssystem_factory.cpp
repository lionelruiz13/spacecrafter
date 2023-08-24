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
#include "tools/context.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_observatory.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "experimentalModule/ModularSystem.hpp"
#include "experimentalModule/Camera.hpp"
#include "experimentalModule/ModuleLoaderMgr.hpp"

SSystemFactory::SSystemFactory(Observer *observatory, Navigator *navigation, TimeMgr *timeMgr) :
    observatory(observatory), navigation(navigation), timeMgr(timeMgr)
{
    // creation of 3D models for planets
    objLMgr = std::make_unique<ObjLMgr>();
	objLMgr -> setDirectoryPath(AppSettings::Instance()->getModel3DDir() );
	objLMgr->insertDefault("Sphere");

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

    std::map<std::string, std::string> params;
    params["coord_func"] = "still_orbit";
    params["orbit_x"] = "0";
    params["orbit_y"] = "0";
    params["orbit_z"] = "0";
    ModularBodyCreateInfo createInfo{
        .orbit = ModuleLoaderMgr::instance.loadOrbit(params),
        .englishName = "MilkyWay",
        .re = {},
        .haloColor = {},
        .albedo = 0,
        .radius = 0,
        .innerRadius = 0,
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::GALAXY,
        .isHaloEnabled = false,
        .altitudeRelativeToRadius = false,
    };
    milkyway = std::make_unique<ModularSystem>(nullptr, createInfo);
    galacticSystem = std::make_unique<ProtoSystem>(objLMgr.get(), observatory, navigation, timeMgr);
    galacticAnchorMgr = galacticSystem->getAnchorManager();
    bodytrace= std::make_shared<BodyTrace>();
    createModularSystem("Solar", "ssystem.ini", {});
}

SSystemFactory::~SSystemFactory()
{
	//delete bodytrace;
}

void SSystemFactory::loadCamera(const InitParser &conf)
{
    const Vec3f lonLatAlt(
        Utility::getDecAngle(conf.getStr(SCS_INIT_LOCATION, SCK_LONGITUDE)) * (M_PI/180),
        Utility::getDecAngle(conf.getStr(SCS_INIT_LOCATION, SCK_LATITUDE)) * (M_PI/180),
        conf.getDouble(SCS_INIT_LOCATION, SCK_ALTITUDE) / (1000*AU)
    );
    ModularBody *home = ModularBody::findBody(conf.getStr(SCS_INIT_LOCATION, SCK_HOME_PLANET));
    if (camera) {
        camera->setBoundToSurface(true);
        camera->setFreeMode(false);
        camera->warpToBody(home);
        camera->moveTo(lonLatAlt);
    } else {
        camera = std::make_unique<Camera>(home, lonLatAlt[0], lonLatAlt[1], lonLatAlt[2]);
    }
    camera->trackBody(nullptr);
    camera->setHeading(conf.getDouble(SCS_NAVIGATION, SCK_HEADING) * (M_PI/180));
    camera->setHalfFov(conf.getDouble(SCS_NAVIGATION, SCK_INIT_FOV) * (M_PI/360), 0);
    camera->lookTo(Utility::strToVec3f(conf.getStr(SCS_NAVIGATION, SCK_INIT_VIEW_POS)), 0);
}

void SSystemFactory::changeSystem(const std::string &mode)
{
    if (mode == "SolarSystem" || mode == "Sun" || mode == "temp_point") {
        currentSystem = ssystem.get();
        camera->switchToBody(ModularBody::findBodyOnce("SolarSystem"));
    } else {
        try {
            currentSystem = systems.at(mode).get();
        } catch (...) {
            currentSystem = createSystem(mode).get();
        }
        camera->switchToBody(ModularBody::findBodyOnce(mode + "System"));
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
    auto &offset = systemOffsets[name];
    system = std::make_unique<ProtoSystem>(objLMgr.get(), observatory, navigation, timeMgr, offset);
    system->load(file);
    createModularSystem(name, file, offset);
}

void SSystemFactory::createModularSystem(const std::string &name, const std::string &filename, const Vec3d &pos)
{
    std::map<std::string, std::string> params;
    params["coord_func"] = "still_orbit";
    params["orbit_x"] = std::to_string(pos[0]);
    params["orbit_y"] = std::to_string(pos[1]);
    params["orbit_z"] = std::to_string(pos[2]);
    ModularBodyCreateInfo info {
        .orbit = ModuleLoaderMgr::instance.loadOrbit(params),
        .englishName = name + "System",
        .re = {},
        .haloColor = {},
        .albedo = 0,
        .radius = 0,
        .innerRadius = 0,
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::SYSTEM,
        .isHaloEnabled = false,
        .altitudeRelativeToRadius = false,
    };
    modularSystems.emplace_back(milkyway.get(), info);
    if (filename.empty()) {
        stringHash_t bodyParams;
        bodyParams["name"] = name.substr(0, name.size()-6); // Remove the 'System' suffix for the star
    	bodyParams["parent"] = "none";
    	bodyParams["type"] = "Star";
    	bodyParams["radius"] = "1190.856";
    	bodyParams["halo"] = "false";
    	Vec3f color = selected_object.getRGB();
    	bodyParams["color"] = std::to_string(color[0]) + "," + std::to_string(color[1]) + "," + std::to_string(color[2]);
    	bodyParams["label_color"] = bodyParams["color"];
    	bodyParams["orbit_color"] = bodyParams["color"];
    	bodyParams["tex_halo"] = "empty";
    	bodyParams["tex_big_halo"] = "big_halo.png";
    	bodyParams["big_halo_size"] = "10";
    	bodyParams["lighting"] = "false";
    	bodyParams["albedo"] = "-1.";
    	bodyParams["coord_func"] = "sun_special";
        bodyParams["system_star"] = "true";
        modularSystems.back().loadBody(bodyParams);
    } else {
        modularSystems.back().loadSystem(filename);
    }
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
    createModularSystem(mode, "", pos);
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

void SSystemFactory::draw(Projector *prj, const Navigator *nav, const Observer *observatory, const ToneReproductor *eye, bool drawHomePlanet)
{
    if (drawModularSystem) {
        Context::instance->renderer.beginDraw(Context::instance->frameIdx);
        camera->draw(Context::instance->renderer);
    } else {
        bodytrace->draw(prj, nav);
        ssystemDisplay->draw(prj, nav, observatory, eye, drawHomePlanet);
    }
}

void SSystemFactory::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
    ssystemTex->updateTesselation(delta_time);
    currentSystem->update(delta_time, nav, timeMgr);
    bodytrace->update(delta_time);
    camera->update(timeMgr->getJDay(), delta_time/1000.f);

    #ifndef NDEBUG // For switching between modes in debug build
    static int downCounter = 1000;
    downCounter -= delta_time;
    if (downCounter < 0) {
        downCounter = 1000;
        drawModularSystem = !drawModularSystem;
    }
    #endif
}

void SSystemFactory::addBody(stringHash_t &param)
{
    currentSystem->addBody(param);
    camera->getCurrentSystem()->loadBody(param);
}
