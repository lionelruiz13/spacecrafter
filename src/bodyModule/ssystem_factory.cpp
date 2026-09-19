/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jeremy Calvo
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
#include <filesystem> // B24 composed-file candidacy + twin directory
#include <sstream>    // config/file values in the deprecation diagnostic
#include <set> // B24 new-only dump sweep

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "experimentalModule/Camera.hpp"
#include "tools/app_settings.hpp"
#include "tools/ini_line.hpp" // the ONE .ini line grammar (INTENT S5.38/S5.39/D29)
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
#include "EntityCore/Core/VulkanMgr.hpp" // screenToRect: the click-coordinate authority
#include "interfaceModule/base_command_interface.hpp" // W_NAME/W_KEEPTIME: the ONE spelling of the command's keys (protosystem.cpp reads the same two)
#include "tools/utility.hpp"
#include "tools/s_texture.hpp" // dumpBigTextures: the preload seam's observable

SSystemFactory::SSystemFactory(Observer *observatory, Navigator *navigation, TimeMgr *timeMgr) :
    observatory(observatory), navigation(navigation), timeMgr(timeMgr)
{
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
    };
    universe = std::make_unique<ModularSystem>(nullptr, universeInfo);
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
    };
    milkyway = universe->createChildSystem(milkywayInfo, BodyRelation::INNER);
    // New-path named anchors (B4, S11.111). Built with the tree root: fixed-point
    // anchors are read in the Universe frame (R3) and their bodies live there.
    cameraAnchors = std::make_unique<CameraAnchors>(universe.get());
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
        const Vec3f v = Utility::strToVec3f(conf.getStr(SCS_NAVIGATION, SCK_INIT_VIEW_POS));
        camera->lookTo(Camera::oldLocalToLocal(v), 0);
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
    };
    ModularSystem *system = milkyway->createChildSystem(info, BodyRelation::INNER);
    modularSystemOf[name] = system;
    const std::string composedPath = composedPathOf(name + "System");
    if (std::filesystem::exists(composedPath)) {
        cLog::get()->write("Composed system file " + composedPath + " wins over "
            + (filename.empty() ? "the built-in " + name + " system content"
                                : "the legacy file " + filename)
            + " (shadowed, NOT read). To fall back, rename or remove " + composedPath + ".",
            LOG_TYPE::L_INFO);
        system->loadComposedSystem(composedPath);
        return;
    }
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
        pendingTwins.emplace_back(filename, composedTwinPathOf(name + "System"));
        if (twinsUnblocked)
            generatePendingTwins();
    }
}

// Contract + rationale: ssystem_factory.hpp (generatePendingTwins).
void SSystemFactory::generatePendingTwins()
{
    twinsUnblocked = true;
    if (pendingTwins.empty())
        return;
    // One directory check for the batch; the twin itself goes through the one
    // atomic writer (failure leaves any previous twin untouched, logged).
    std::error_code ec;
    std::filesystem::create_directories("modularSystem", ec);
    for (const auto &[legacyFile, twinPath] : pendingTwins) {
        if (ec) {
            cLog::get()->write("Can't create the modularSystem directory ("
                + ec.message() + ") - composed twin of " + legacyFile + " not generated.",
                LOG_TYPE::L_WARNING);
            continue;
        }
        // The system is found by the twin it was recorded for: a name lookup
        // would be a second authority for the pairing this list already holds.
        for (const auto &[systemName, system] : modularSystemOf) {
            if (system && composedTwinPathOf(system->getEnglishName()) == twinPath) {
                system->generateComposedTwin(legacyFile, twinPath);
                break;
            }
        }
    }
    pendingTwins.clear();
}

// Contract + rationale: ssystem_factory.hpp (initDisplayScaling).
void SSystemFactory::initDisplayScaling(bool flagMoonScale, double moonScale,
                                        bool flagSunScale, double sunScale)
{
    ssystem->setFlagMoonScale(flagMoonScale);
    ssystem->setMoonScale(moonScale, true);
    ssystem->setFlagSunScale(flagSunScale);
    mirrorSunHaloSize();
    ssystem->setSunScale(sunScale, true);
    // THE NEW PATH, format-scoped.
    initBodyDisplayScale("Moon", SCK_MOON_SCALE, flagMoonScale, moonScale);
    initBodyDisplayScale("Sun", SCK_SUN_SCALE, flagSunScale, sunScale);
}

// Contract + rationale: ssystem_factory.hpp (initDisplayScaling).
void SSystemFactory::initBodyDisplayScale(const char *bodyName, const char *configKey,
                                          bool flag, double value)
{
    ModularBody *body = ModularBody::findBody(bodyName);
    if (!body)
        return;   // this field's data has no such body; nothing owns anything
    if (body->isComposedDeclared()) {
        // The modular file owns this body's display scale and has already
        // applied it; config.ini's value is deprecated for it, and said so.
        announceDeprecatedScale(bodyName, configKey, flag, value);
        body->updateCache();
        return;
    }
    body->restoreScaling(flag ? static_cast<float>(value) : 1.f);
}

// Contract + rationale: ssystem_factory.hpp (announceDeprecatedScale).
void SSystemFactory::announceDeprecatedScale(const char *bodyName, const char *configKey,
                                             bool flag, double value)
{
    if (!flag)
        return;
    std::string source = "the modular system file that declares it";
    const ModularBody *body = ModularBody::findBody(bodyName);
    for (const auto &[systemName, system] : modularSystemOf) {
        if (system && body && body->isInSubtreeOf(system)) {
            source = system->getSystemFilename();
            break;
        }
    }
    std::ostringstream os;
    os << value;
    const std::string configured = os.str();
    os.str({});
    os << (body ? body->getScalingTarget() : 1.f);
    const std::string authored = os.str();
    cLog::get()->write(std::string("config.ini [viewing] ") + configKey + " = "
        + configured + " is IGNORED for '" + bodyName + "': that body is declared by "
        + source + ", and a modular system file owns the display scaling of the bodies it "
        "declares (config.ini owns it for the legacy format only). '" + bodyName
        + "' is drawn at display_scale = " + authored + ". To change it, set display_scale in "
        "its section of " + source + "; to give the value back to config.ini, remove or rename "
        "that file so the legacy system file is read again.", LOG_TYPE::L_INFO);
}

// Contract + rationale: ssystem_factory.hpp (restoreDisplayScaling).
void SSystemFactory::restoreDisplayScaling()
{
    if (!fileOwnsDisplayScale("Moon")) {
        if (ModularBody *moon = ModularBody::findBody("Moon"))
            moon->restoreScaling(ssystem->getFlagMoonScale() ? ssystem->getMoonScale() : 1.f);
    }
    if (!fileOwnsDisplayScale("Sun")) {
        if (ModularBody *sun = ModularBody::findBody("Sun"))
            sun->restoreScaling(ssystem->getFlagSunScale() ? ssystem->getSunScale() : 1.f);
    }
}

void SSystemFactory::createExperimentalOort(unsigned int nbr, const Vec3f &color)
{
    // The oort belongs to the MAIN solar system only (its geometry is
    // heliocentric); other created systems (createSystem) are foreign stars.
    auto it = modularSystemOf.find("Solar");
    if (it == modularSystemOf.end() || it->second == nullptr) {
        cLog::get()->write("B5 \xc2\xa7" "6.9 pilot: flag_experimental_oort is set, but there is no "
            "\"Solar\" modular system node to attach the cloud to - the experimental oort "
            "was NOT instantiated (the old oort cloud is unaffected).", LOG_TYPE::L_WARNING);
        return;
    }
    ModularSystem *system = it->second;

    constexpr float OORT_REGIME_RADIUS_AU = 50.f;

    std::map<std::string, std::string> orbitParams;
    orbitParams["coord_func"] = "still_orbit";
    orbitParams["orbit_x"] = "0";
    orbitParams["orbit_y"] = "0";
    orbitParams["orbit_z"] = "0";
    ModularBodyCreateInfo info {
        .orbit = ModuleLoaderMgr::instance.loadOrbit(orbitParams),
        .englishName = "Oort",
        .re = {},          // identity rotation: no tilt; the surface-fold z-spin
                           // the near regime applies is azimuthally inert on this
                           // theta-uniform cloud (oortSamplePoint).
        .haloColor = {},
        .albedo = 0,
        .radius = OORT_REGIME_RADIUS_AU, // AU (regime low-edge peg; NOT the extent)
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::VOID,      // decoration, not a navigable body-type
        .isHaloEnabled = false,
    };
    ModularBody *oortBody = system->createChild(info, BodyRelation::ORBITING);

    std::map<std::string, std::string> modParams;
    modParams["oort"] = "true"; // the OORT-slot opt-in (OortLoader::isLikely)
    modParams["oort_elements"] = std::to_string(nbr);
    modParams["oort_color"] = std::to_string(color[0]) + "," + std::to_string(color[1])
                            + "," + std::to_string(color[2]);
    ModuleLoaderMgr::instance.loadModule(BodyModuleType::CUSTOM, oortBody, modParams, "OORT");
    oortBody->updateCache(); // bounding radius (cloud extent) now set
    experimentalOortInstantiated = true;

    cLog::get()->write("B5 \xc2\xa7" "6.9 pilot: experimental oort modular body instantiated at the "
        "SolarSystem floor (" + std::to_string(nbr) + " points).", LOG_TYPE::L_INFO);
}

void SSystemFactory::loadGalacticSystem(const std::string &path, const std::string &name)
{
    std::string dir = path;
    if (!dir.empty() && dir.back() != '/')
        dir += '/';

    stringHash_t params;

    std::ifstream file(dir + name);
    if (file) {
        std::string line, key, value;
        std::string section;
		while(getline(file , line)) {
            switch (IniLine::read(line, key, value)) {
                case IniLine::Kind::SECTION:
                    if (!params.empty())
                        loadSystem(dir, params, section);
                    section = key;
                    break;
                case IniLine::Kind::ENTRY:
                    params[key] = value;
                    break;
                case IniLine::Kind::MALFORMED:
                    cLog::get()->write("Ignoring line without '=' in " + dir + name + ": '"
                        + key + "' - a key/value line needs 'key = value'; write '#' first "
                        "to make it a comment.", LOG_TYPE::L_WARNING);
                    break;
                case IniLine::Kind::EMPTY:
                    break;
            }
    		}
        if (!params.empty())
            loadSystem(dir, params, section);
		file.close();
    } else {
        galacticAnchorMgr->addAnchor("Sun", std::make_shared<AnchorPointObservatory>(0, 0, 0));
    }
}

static const std::string *declaredParam(const stringHash_t &params, const char *key)
{
    const auto it = params.find(key);
    return (it == params.end() || it->second.empty()) ? nullptr : &it->second;
}

void SSystemFactory::loadSystem(const std::string &path, stringHash_t &params, const std::string &section)
{
    std::cout << "Params :\n";
    for (auto &p : params) {
        std::cout << p.first << " : " << p.second << '\n';
    }
    const std::string *nameKey = declaredParam(params, "name");
    const std::string label = !section.empty() ? ("[" + section + "]")
                            : nameKey ? ("section '" + *nameKey + "'")
                            : std::string("a section with no header");
    const auto reject = [&](const std::string &why) {
        cLog::get()->write("galactic.ini: skipping " + label + " - " + why
            + ". A star system needs name, x, y and z (galactic coordinates in "
            "light years); the section is ignored and the rest of the file is "
            "loaded normally.", LOG_TYPE::L_WARNING);
        params.clear();
    };
    if (!nameKey) {
        reject("no 'name' key");
        return;
    }
    double coord[3];
    static const char *const AXIS[3] = {"x", "y", "z"};
    for (int i = 0; i < 3; ++i) {
        const std::string *value = declaredParam(params, AXIS[i]);
        if (!value) {
            reject(std::string("no '") + AXIS[i] + "' key");
            return;
        }
        try {
            coord[i] = std::stod(*value);
        } catch (const std::exception &) {
            reject(std::string("'") + AXIS[i] + " = " + *value + "' is not a number");
            return;
        }
    }
    params["type"] = "observatory";
    galacticAnchorMgr->addAnchor(params);
    systemOffsets[*nameKey].set(coord[0], coord[1], coord[2]);
    if (!params["system"].empty())
        addSystem(*nameKey, path + params["system"]);
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
        drawExperimental();
    } else {
        bodytrace->draw(prj, nav);
        ssystemDisplay->draw(prj, nav, observatory, eye, drawHomePlanet);
    }
}

// Contract + rationale: ssystem_factory.hpp (drawExperimental).
void SSystemFactory::drawExperimental()
{
    if (!drawModularSystem)
        return;
    Context::instance->renderer.beginDraw(Context::instance->frameIdx);
    camera->draw(Context::instance->renderer);
}

void SSystemFactory::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
    ssystemTex->updateTesselation(delta_time);
    currentSystem->update(delta_time, nav, timeMgr);
    bodytrace->update(delta_time);

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
    camera->getCurrentSystem()->loadBody(param, nullptr, true);
}

// Contract + rationale: ssystem_factory.hpp (removeSupplementalBodies).
bool SSystemFactory::removeSupplementalBodies(const std::string &name)
{
    if (!currentSystem->removeSupplementalBodies(name))
        return false;
    camera->getCurrentSystem()->removeSupplementalBodies();
    return true;
}

// Contract + rationale: ssystem_factory.hpp (startTrails).
void SSystemFactory::startTrails(bool b)
{
    currentSystem->startTrails(b);
    if (camera)
        if (ModularSystem *system = camera->getCurrentSystem())
            system->startTrails(b);
}

// Contract + rationale: ssystem_factory.hpp (preloadBody).
void SSystemFactory::preloadBody(stringHash_t &param)
{
    currentSystem->preloadBody(param);
    if (ModularBody *body = ModularBody::findBodyOnce(param[W_NAME]))
        body->preload(Utility::strToInt(param[W_KEEPTIME], 1));
}

// Contract: ssystem_factory.hpp (setSelected). Out of line because resolving
// the new-path body from a ModularObject needs the bridge's complete type.
void SSystemFactory::setSelected(const Object &obj)
{
    ssystemSelected->setSelected(obj);
    if (ModularObject *bridge = obj.as<ModularObject>())
        newSelectedBody = bridge->body;
    else
        newSelectedBody = (obj.getType() == OBJECT_BODY)
                        ? ModularBody::findBodyOnce(obj.getEnglishName())
                        : nullptr;
}

// Contract + rationale: ssystem_factory.hpp (searchObjectByEnglishName).
Object SSystemFactory::searchObjectByEnglishName(const std::string &englishName) const
{
    if (auto body = currentSystem->searchByEnglishName(englishName))
        return Object(body.get());          // old path, byte-for-byte as before
    if (ModularBody *newOnly = ModularBody::findBodyOnce(englishName))
        return Object(new ModularObject(newOnly));
    return Object();
}

// Contract + rationale: ssystem_factory.hpp (searchNewOnlyObjectAt).
Object SSystemFactory::searchNewOnlyObjectAt(int x, int y) const
{
    if (!camera)
        return Object();
    ModularSystem *system = camera->getCurrentSystem();
    if (!system)
        return Object();
    auto rect = VulkanMgr::instance->screenToRect(
        {static_cast<uint16_t>(x), static_cast<uint16_t>(y)});
    ModularBody *hit = system->findBodyAt({rect.first, -rect.second});
    if (!hit || currentSystem->searchByEnglishName(hit->getEnglishName()))
        return Object();                    // nothing, or an old-tree body: old decides
    return Object(new ModularObject(hit));
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

    if (ModularBody *body = ModularBody::findBodyOnce(referenceName)) {
        camera->rebindReference(body);
    } else {
        cLog::get()->write("System reload: the observer's reference body '" + referenceName
            + "' is not defined by the reloaded data - the observer now references '"
            + camera->getReferenceBody()->getEnglishName()
            + "'. Restore that body in the data file, or move the observer with "
              "'set home_planet <body>'.", LOG_TYPE::L_ERROR);
    }
    restoreDisplayScaling();
    if (!trackedName.empty())
        camera->trackBody(ModularBody::findBodyOnce(trackedName));
    if (!selectedName.empty())
        newSelectedBody = ModularBody::findBodyOnce(selectedName);
    cLog::get()->write("System '" + system->getEnglishName() + "' reloaded (observer state kept)",
        LOG_TYPE::L_INFO);
    return true;
}

// Contract + rationale: ssystem_factory.hpp (saveCurrentSystem).
bool SSystemFactory::saveCurrentSystem(const std::string &filename)
{
    ModularSystem *system = camera->getCurrentSystem();
    const std::string node = system->getEnglishName();
    std::string target = composedPathOf(node);
    if (!filename.empty()) {
        if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
            cLog::get()->write("Command 'body action save': '" + filename + "' is a path, and this "
                "command takes a file NAME - a system file is written next to the others, in "
                + composedPathOf("<system>") + ". Nothing was written. (The legacy system files of "
                "this install are never written to, by design.) To fix: pass a plain name, e.g. "
                "'body action save filename " + node + "'.", LOG_TYPE::L_ERROR);
            return false;
        }
        if (filename.size() >= 9 && filename.compare(filename.size() - 9, 9, ".disabled") == 0) {
            cLog::get()->write("Command 'body action save': '" + filename + "' is the name of a "
                "MACHINE-OWNED file - spacecrafter regenerates it at every legacy load of this "
                "system, so a save there would be silently overwritten. Nothing was written. "
                "To fix: drop the '.disabled' (the name without it is the one that gets read), "
                "or pick another name.", LOG_TYPE::L_ERROR);
            return false;
        }
        target = "modularSystem/" + filename
               + ((filename.find('.') == std::string::npos) ? ".ini" : "");
    }
    std::error_code ec;
    std::filesystem::create_directories("modularSystem", ec);
    if (ec) {
        cLog::get()->write("Command 'body action save': can't create the modularSystem directory ("
            + ec.message() + ") - system '" + node + "' NOT saved.", LOG_TYPE::L_ERROR);
        return false;
    }
    return system->saveSystem(target);
}

void SSystemFactory::wireEnvironment(MilkyWay *milky, Atmosphere *atmosphere, ToneReproductor *eye)
{
    environment = std::make_unique<EnvironmentManager>(milky, atmosphere);
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

bool SSystemFactory::setBodyDatumRadius(const std::string &englishName, double km)
{
    // km -> AU with the loader's own factor (ModularSystem.cpp:842); the raw
    // ModularBody setter defers scaling to updateCache (single authority, I2).
    if (ModularBody *body = ModularBody::findBody(englishName)) {
        body->setDatumRadius(static_cast<float>(km / AU));
        return true;
    }
    return false;
}

bool SSystemFactory::setBodyGroundRadius(const std::string &englishName, double km)
{
    if (ModularBody *body = ModularBody::findBody(englishName)) {
        body->setGroundRadius(static_cast<float>(km / AU));
        return true;
    }
    return false;
}

void SSystemFactory::dumpTracePaths(const std::string &file,
                                    const std::function<void(std::ostream &)> &extraHeader)
{
    std::ofstream out(file.empty() ? "/tmp/dual_trace.json" : file);
    std::string oldSystemName = "unknown";
    if (currentSystem == ssystem.get())
        oldSystemName = "SolarSystem";
    else if (currentSystem == galacticSystem.get())
        oldSystemName = "galactic";
    else {
        for (const auto &sys : systems) {
            if (sys.second.get() == currentSystem) {
                oldSystemName = sys.first;
                break;
            }
        }
    }
    out << std::setprecision(17) << "{\"type\":\"header\",\"jd\":"
        << timeMgr->getJDay() << ",\"timeSpeed\":" << timeMgr->getTimeSpeedRaw()
        << ",\"timePaused\":" << (timeMgr->getTimePause() ? "true" : "false")
        << ",\"oldSystem\":\"" << oldSystemName << "\",\"inSystem\":"
        << (inSystem ? "true" : "false")
        << ",\"helioToEye\":[";
    {
        const Mat4d &h = navigation->getHelioToEyeMat();
        for (int i = 0; i < 16; ++i)
            out << h.r[i] << ((i < 15) ? "," : "");
    }
    {
        const Vec3d &lv = navigation->getLocalVision();
        out << "],\"oldLocalVision\":[" << lv[0] << ',' << lv[1] << ',' << lv[2];
    }
    out << "],\"camera\":";
    camera->dumpTrace(out);
    out << ",\"anchors\":";
    cameraAnchors->dumpState(out);
    out << ",\"gates\":{\"viewportRadius\":" << ModularBody::getViewportRadius()
        << ",\"px\":{\"early\":" << BODY_EARLY_VISIBILITY_BOUNDING_SIZE
        << ",\"full\":" << BODY_FULL_VISIBILITY_BOUNDING_SIZE
        << ",\"bigTexture\":" << BODY_BIG_TEXTURE_BOUNDING_SIZE
        << "},\"screenSize\":{\"early\":" << ModularBody::earlyVisibilityGate()
        << ",\"full\":" << ModularBody::fullVisibilityGate()
        << ",\"bigTexture\":" << ModularBody::bigTextureGate()
        << "}}";
    out << ",\"bigTextures\":";
    s_texture::dumpBigTextures(out);
    if (extraHeader)
        extraHeader(out);
    out << "}\n";
    std::ofstream navout(file.empty() ? "/tmp/dual_trace.json.navstr"
                                      : (file + ".navstr"));
    for (auto it = currentSystem->begin(); it != currentSystem->end(); ++it) {
        out << "{\"type\":\"body\",\"name\":\"" << it->first << "\",\"old\":";
        it->second.body->dumpTrace(out);
        out << ",\"new\":";
        ModularBody *nb = ModularBody::findBodyOnce(it->first);
        if (nb) {
            nb->useNow();
            nb->dumpTrace(out);
        }
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
    {
        std::set<std::string> dumped;
        for (auto it = currentSystem->begin(); it != currentSystem->end(); ++it)
            dumped.insert(it->first);
        ModularBody::forEach([&, this](ModularBody &nb) {
            if (dumped.count(nb.getEnglishName()))
                return;
            out << "{\"type\":\"body\",\"name\":\"" << nb.getEnglishName()
                << "\",\"old\":null,\"new\":";
            nb.useNow();
            nb.dumpTrace(out);
            out << "}\n";
            ModularObject bridge;
            bridge.body = &nb;
            navout << nb.getEnglishName()
                   << "\n  NEW nav: " << bridge.getShortInfoNavString(navigation, timeMgr, observatory)
                   << "\n  NEW inf: " << bridge.getInfoString(navigation) << "\n";
        });
    }
    for (const char *name : {"Earth", "Moon", "Sun", "Mercury", "Venus", "Mars",
                             "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto", "Charon",
                             "Iapetus", "Proteus", "Puck", "Janus", "Prometheus",
                             "Amalthea", "Thebe", "Telesto", "Pandora", "Helene",
                             "Epimetheus", "Juliet", "Portia", "Rosalind", "Belinda",
                             "Naiad", "Thalassa", "Despina", "Galatea", "Larissa"}) {
        if (ModularBody *nb = ModularBody::findBodyOnce(name)) {
            out << "{\"type\":\"hops\",\"name\":\"" << name << "\",\"new\":";
            nb->dumpHops(out);
            out << "}\n";
        }
    }
}
