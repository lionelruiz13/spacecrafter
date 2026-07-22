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
#include <fstream>
#include <iomanip>

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "experimentalModule/Camera.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "tools/context.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_observatory.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "experimentalModule/ModularSystem.hpp"
#include "experimentalModule/Camera.hpp"
#include "experimentalModule/ModularObject.hpp"
#include "experimentalModule/ModuleLoaderMgr.hpp"
#include "experimentalModule/EnvironmentManager.hpp"
#include "experimentalModule/environmentModules/MilkyWayEnv.hpp"

SSystemFactory::SSystemFactory(Observer *observatory, Navigator *navigation, TimeMgr *timeMgr) :
    observatory(observatory), navigation(navigation), timeMgr(timeMgr)
{
    // Projection mode mirror (INTENT 11.33): Context::projectionType is
    // parsed from config at App init, before Core builds this factory -
    // launch-constant, same precondition as the old path's per-pipeline
    // spec constants.
    ModularBody::setProjectionMode(Context::projectionType);

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

    // The nesting spine (G2, INTENT 11.36): universe ⊃ milkyway ⊃ systems.
    // The universe is the tree root and only eternal node (INTENT 6.6);
    // the milkyway is its INNER child - a galaxy is a body like any other,
    // and its 2D backdrop (MilkyWayEnv, wired in wireEnvironment) shows only
    // while the camera's reference chain includes it: leaving the galaxy
    // drops the backdrop as a natural consequence of chain membership.
    std::map<std::string, std::string> params;
    params["coord_func"] = "still_orbit";
    params["orbit_x"] = "0";
    params["orbit_y"] = "0";
    params["orbit_z"] = "0";
    ModularBodyCreateInfo universeInfo{
        .orbit = ModuleLoaderMgr::instance.loadOrbit(params),
        .englishName = "Universe",
        .re = {},
        .haloColor = {},
        .albedo = 0,
        .radius = 0,
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::SYSTEM,
        .isHaloEnabled = false,
        .altitudeRelativeToRadius = false,
    };
    universe = std::make_unique<ModularSystem>(nullptr, universeInfo);
    // Galactic disc radius ~15.5 kpc in AU - placeholder constant until the
    // galaxy gets a data home (suspended: galaxy data model). Bounds the
    // galaxy body (bounding/screen size from outside) and feeds the AoI
    // heuristic; AoI tuning at galaxy scale is suspended with it.
    constexpr float MILKYWAY_RADIUS_AU = 3.2e9f;
    ModularBodyCreateInfo milkywayInfo{
        .orbit = ModuleLoaderMgr::instance.loadOrbit(params),
        .englishName = "MilkyWay",
        .re = {},
        .haloColor = {},
        .albedo = 0,
        .radius = MILKYWAY_RADIUS_AU,
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::GALAXY,
        .isHaloEnabled = false,
        .altitudeRelativeToRadius = false,
    };
    milkyway = universe->createChildSystem(milkywayInfo, BodyRelation::INNER);
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
    // Mount: same config key as the old navigator (viewing_mode) - the two
    // complementary viewing systems (Camera.hpp CameraMount).
    camera->setMount(conf.getStr(SCS_NAVIGATION, SCK_VIEWING_MODE) == "equator"
                     ? CameraMount::EQUATORIAL : CameraMount::ALTAZ);
    camera->setHeading(conf.getDouble(SCS_NAVIGATION, SCK_HEADING) * (M_PI/180));
    camera->setHalfFov(conf.getDouble(SCS_NAVIGATION, SCK_INIT_FOV) * (M_PI/360), 0);
    {
        // init_view_pos is an OLD-path local-frame vector (x=South, y=East,
        // z=Up: observer getRotLocalToEquatorialFixed = Z(-lon)·Y(90-lat)).
        // The camera's local frame is x=East, y=North, z=Up (the placement
        // fold in Camera::update puts the pole at +y). Same components in
        // both frames = a 90° roll about the zenith - measured as the
        // 89.9943° init-view differential (harness 2026-07-12). Convert at
        // the seam, like the longitude sign: (x,y,z)_old -> (y,-x,z)_camera.
        const Vec3f v = Utility::strToVec3f(conf.getStr(SCS_NAVIGATION, SCK_INIT_VIEW_POS));
        camera->lookTo(Vec3f(v[1], -v[0], v[2]), 0);
    }
}

void SSystemFactory::reloadColors(const std::string& planetfile)
{
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
            auto equalPos = line.find('=');
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
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::SYSTEM,
        .isHaloEnabled = false,
        .altitudeRelativeToRadius = false,
    };
    // Systems nest IN the tree as the milkyway's INNER children (G2):
    // shown while the camera is inside the galaxy, isolated roots for their
    // own content, registered in the milkyway's sorted body list.
    ModularSystem *system = milkyway->createChildSystem(info, BodyRelation::INNER);
    modularSystemOf[name] = system;
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
        system->loadBody(bodyParams);
    } else {
        system->loadSystem(filename);
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
    // camera/environment updates live in updateExperimental (executor-mode
    // independent - see the header note); this method remains the OLD-path
    // update, reached only through the solar/stellar executor modules.

    static int downCounter = 1000;
    downCounter -= delta_time;
    if (downCounter < 0) {
        downCounter = 1000;
        if (!pathPinned) // flag experimental_path stops the A/B alternation
            drawModularSystem = !drawModularSystem;
    }
}

void SSystemFactory::updateExperimental(int delta_time, const TimeMgr* timeMgr)
{
    camera->update(timeMgr->getJDay(), delta_time/1000.f);
    // Environment aggregation - after the camera (chain state fresh);
    // engine writes only when the modular phase is the drawing one.
    if (environment)
        environment->update(*camera, timeMgr->getJDay(), delta_time/1000.f, drawModularSystem);
}

void SSystemFactory::addBody(stringHash_t &param)
{
    currentSystem->addBody(param);
    camera->getCurrentSystem()->loadBody(param);
}

// Contract + rationale: ssystem_factory.hpp (reloadCurrentSystem).
bool SSystemFactory::reloadCurrentSystem()
{
    ModularSystem *system = camera->getCurrentSystem();
    // Names, captured BEFORE the rebuild - the objects will not survive it.
    // Empty name = no such reference to restore (never a body).
    const std::string referenceName = camera->getReferenceBody()->getEnglishName();
    ModularBody *tracked = camera->getTrackedBody();
    const std::string trackedName = tracked ? tracked->getEnglishName() : std::string();
    ModularBody *selected = ModularBody::getSelected();
    const std::string selectedName = selected ? selected->getEnglishName() : std::string();

    if (!system->reloadSystem()) {
        cLog::get()->write("Cannot reload the system '" + system->getEnglishName()
            + "': it was not loaded from a data file (it is a container level of the "
              "hierarchy, not a system description). Move the observer into a system "
              "loaded from a file (e.g. 'set home_planet Earth') and reload from there.",
            LOG_TYPE::L_ERROR);
        return false;
    }

    // Re-seat the camera-side references by name (see the header: valid is
    // not the same as preserved). Losing one is a data-content change, so it
    // is traced as an error naming what was lost and what it fell back to.
    if (ModularBody *body = ModularBody::findBodyOnce(referenceName)) {
        camera->rebindReference(body);
    } else {
        cLog::get()->write("System reload: the observer's reference body '" + referenceName
            + "' is not defined by the reloaded data - the observer now references '"
            + camera->getReferenceBody()->getEnglishName()
            + "'. Restore that body in the data file, or move the observer with "
              "'set home_planet <body>'.", LOG_TYPE::L_ERROR);
    }
    if (!trackedName.empty())
        camera->rebindTarget(ModularBody::findBodyOnce(trackedName));
    if (!selectedName.empty())
        newSelectedBody = ModularBody::findBodyOnce(selectedName);
    cLog::get()->write("System '" + system->getEnglishName() + "' reloaded (observer state kept)",
        LOG_TYPE::L_INFO);
    return true;
}

// Dual-path trace harness (experimentalModule/INTENT.md 11.14).
// JSON lines: one header (jd, camera state), then one line per old-path body
// of the CURRENT system with the matching new-path body (by english name,
// null when absent - itself a finding, cf INTENT 11.3 hardcoded-flag case).
void SSystemFactory::wireEnvironment(MilkyWay *milky, Atmosphere *atmosphere, ToneReproductor *eye)
{
    environment = std::make_unique<EnvironmentManager>(milky, atmosphere);
    // The galaxy node's InAoI milkyway member: active from anywhere IN the
    // galaxy - i.e. while the camera's reference chain includes the milkyway.
    // With the universe above it (INTENT 11.36), leaving the galaxy drops the
    // backdrop as a natural consequence of the chain diff - the mandate's
    // "outer milkyway only shown while in the milkyway". The 2D/3D regime
    // switch is post-parity (EnvironmentModule.hpp convergence note).
    milkyway->addEnvironment(std::make_unique<MilkyWayEnv>(milky, eye), false);
}

void SSystemFactory::setEnvironmentLandscape(Landscape *landscape)
{
    if (environment)
        environment->setLandscape(landscape);
}

void SSystemFactory::setEnvironmentAtmosphereFlag(bool b)
{
    if (environment)
        environment->setAtmosphereUserFlag(b);
}

const EnvironmentState &SSystemFactory::getEnvironmentState() const
{
    return environment->getState();
}

const AtmosphereComputeInput &SSystemFactory::getEnvironmentAtmosphereInput() const
{
    return environment->getAtmosphereInput();
}

void SSystemFactory::drawEnvironmentBackdrop()
{
    if (environment)
        environment->drawBackdrop(Context::instance->renderer);
}

void SSystemFactory::drawEnvironmentSky()
{
    if (environment)
        environment->drawSky(Context::instance->renderer);
}

void SSystemFactory::syncCameraReference(const std::string &name)
{
    if (ModularBody *body = ModularBody::findBody(name)) {
        if (camera)
            camera->warpToBody(body);
    } else {
        cLog::get()->write("New path has no body '" + name + "' to re-reference the camera on", LOG_TYPE::L_WARNING);
    }
}

void SSystemFactory::dumpTracePaths(const std::string &file)
{
    std::ofstream out(file.empty() ? "/tmp/dual_trace.json" : file);
    out << std::setprecision(17) << "{\"type\":\"header\",\"jd\":"
        << timeMgr->getJDay() << ",\"helioToEye\":[";
    {
        const Mat4d &h = navigation->getHelioToEyeMat();
        for (int i = 0; i < 16; ++i)
            out << h.r[i] << ((i < 15) ? "," : "");
    }
    // Old-path view state (initial-view seam investigation, 2026-07-12):
    // local_vision is what updateViewMat actually consumes - dumped to compare
    // against the value the seams believe they set (init_view_pos & co).
    {
        const Vec3d &lv = navigation->getLocalVision();
        out << "],\"oldLocalVision\":[" << lv[0] << ',' << lv[1] << ',' << lv[2];
    }
    out << "],\"camera\":";
    camera->dumpTrace(out);
    out << "}\n";
    // B9 az-convention observability (INTENT §11.4/§11.60): the alt/az the two
    // paths expose to the UI/scripting surface, per body, at the SAME frame.
    // altaz_old = Body::getAltAz (old convention, az = 3π−az); altaz_new = the
    // REAL ModularObject::getAltAz (the D2 bridge method getSelectedAZ would
    // call once wired). The nav-string sidecar (<file>.navstr) captures the
    // caller-visible strings both paths print. Locked by harness/b9_azconv.py.
    std::ofstream navout(file.empty() ? "/tmp/dual_trace.json.navstr"
                                      : (file + ".navstr"));
    for (auto it = currentSystem->begin(); it != currentSystem->end(); ++it) {
        out << "{\"type\":\"body\",\"name\":\"" << it->first << "\",\"old\":";
        it->second.body->dumpTrace(out);
        out << ",\"new\":";
        ModularBody *nb = ModularBody::findBodyOnce(it->first);
        if (nb)
            nb->dumpTrace(out);
        else
            out << "null";
        {
            double oalt = 0, oaz = 0;
            it->second.body->getAltAz(navigation, &oalt, &oaz);
            out << std::setprecision(17) << ",\"altaz_old\":[" << oalt << ',' << oaz << ']';
            if (nb) {
                ModularObject mo;
                mo.body = nb;
                double nalt = 0, naz = 0;
                mo.getAltAz(navigation, &nalt, &naz);
                out << ",\"altaz_new\":[" << nalt << ',' << naz << ']';
                navout << it->first
                       << "\n  OLD nav: " << it->second.body->getShortInfoNavString(navigation, timeMgr, observatory)
                       << "\n  NEW nav: " << mo.getShortInfoNavString(navigation, timeMgr, observatory)
                       << "\n  OLD inf: " << it->second.body->getInfoString(navigation)
                       << "\n  NEW inf: " << mo.getInfoString(navigation) << "\n";
            } else {
                out << ",\"altaz_new\":null";
            }
            out << std::setprecision(9);
        }
        out << "}\n";
    }
    // Quadruplet (minimal set separating translation / common rotation /
    // hop-accumulated rotation): identity, down-hop, up-hop, up-then-down.
    // + Pluto/Charon (INTENT 11.34 orientation set: the loudest declared
    // parent-obliquity case, 115.60deg - tilt pieces needed for the
    // convention checker even while the branch is invisible; lastJD stays
    // fresh through recursiveTranslationUpdate).
    // Earth/Moon/Sun/Pluto/Charon = the 11.34 orientation set; the 7
    // rot_pole_ra planets (Mercury..Neptune) added for the B28 bit-identical
    // gate (INTENT 11.67): their self-hop `tilt` + raw obliquity/ascendingNode
    // are the projection-free, B30-immune readout of the frame conversion.
    for (const char *name : {"Earth", "Moon", "Sun", "Mercury", "Venus", "Mars",
                             "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto", "Charon"}) {
        if (ModularBody *nb = ModularBody::findBodyOnce(name)) {
            out << "{\"type\":\"hops\",\"name\":\"" << name << "\",\"new\":";
            nb->dumpHops(out);
            out << "}\n";
        }
    }
}
