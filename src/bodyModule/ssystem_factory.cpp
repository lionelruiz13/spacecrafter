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
#include <filesystem> // B24 composed-file candidacy + twin directory
#include <set> // B24 new-only dump sweep

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "experimentalModule/Camera.hpp"
#include "tools/app_settings.hpp"
#include "tools/ini_line.hpp" // the ONE .ini line grammar (INTENT §5.38/§5.39/D29)
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
        // datum/ground OMITTED ⇒ the NAV_RADIUS_UNSET sentinel ⇒ the ModularSystem
        // ctor's system class default (0) fires (B10-datum0, §11.75(a)). Realizes
        // the node's authored altitudeRelativeToRadius=false (centre-relative)
        // intent that the inert ctor never ran. Behaviourally moot here (radius 0
        // ⇒ 0 either way), but expressed via the class rule keyed off system
        // nature (I4), NOT a hardcoded value - MilkyWay (radius 3.2e9) is where it
        // bites, and it must bite off the same rule, not a per-name edit.
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::SYSTEM,
        .isHaloEnabled = false,
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
        // datum/ground OMITTED ⇒ NAV_RADIUS_UNSET sentinel ⇒ system class default
        // 0 (B10-datum0, §11.75(a) [vixy 2026-07-22], resolved off the node's
        // SYSTEM nature in the ModularSystem ctor - never a per-name MilkyWay
        // edit). This REALIZES the node's authored altitudeRelativeToRadius=false
        // intent (measure galaxy-scale altitude from the galactic CENTRE) that the
        // inert ctor never ran. USER-VISIBLE CHANGE, DECIDED knowingly: free-mode
        // getAltitudeReference() drops 3.2e9 AU -> 0, so `moveto altitude X` at a
        // MilkyWay reference lands at X, not 3.2e9 AU + X (§11.80 measured the old
        // value live). Override with the datum_radius/ground_radius data key or
        // the §11.84 runtime command if 3.2e9 is ever wanted back.
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::GALAXY,
        .isHaloEnabled = false,
    };
    milkyway = universe->createChildSystem(milkywayInfo, BodyRelation::INNER);
    // New-path named anchors (B4, §11.111). Built with the tree root: fixed-point
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
        // datum/ground OMITTED ⇒ NAV_RADIUS_UNSET sentinel ⇒ system class default
        // 0 (B10-datum0, §11.75(a)), same class rule as Universe/MilkyWay. Moot
        // here (radius 0 ⇒ 0 either way); expressed off system nature (I4), not a
        // hardcoded value - a per-system node given a non-zero radius would take
        // the same centre-relative default.
        .oblateness = 0,
        .solLocalDay = 0,
        .bodyType = BodyType::SYSTEM,
        .isHaloEnabled = false,
    };
    // Systems nest IN the tree as the milkyway's INNER children (G2):
    // shown while the camera is inside the galaxy, isolated roots for their
    // own content, registered in the milkyway's sorted body list.
    ModularSystem *system = milkyway->createChildSystem(info, BodyRelation::INNER);
    modularSystemOf[name] = system;
    // B24 candidacy (INTENT §11.51(a), §11.78(d)): an enabled composed file
    // (~/.spacecrafter/modularSystem/<node>.ini - cwd is ~/.spacecrafter,
    // main.cpp chdir, same convention as the legacy "ssystem.ini") always
    // wins over the legacy source. Shadowing self-names on every launch so a
    // user editing the legacy file never gets silence [§11.51(a) derived].
    // The .ini.disabled twin below is machine-owned and regenerated at every
    // legacy load; the extension-dropped copy is user-owned, never touched.
    // NB: keyed on the node name expression, NOT info.englishName - the
    // createChildSystem ctor above moved that string out (fired live: the
    // twin generated as ".ini.disabled", INTENT §11.78(f)).
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
        // B25 generation half: the machine-owned twin, written through the one
        // atomic writer (failure leaves any previous twin untouched, logged).
        std::error_code ec;
        std::filesystem::create_directories("modularSystem", ec);
        if (ec) {
            cLog::get()->write("Can't create the modularSystem directory ("
                + ec.message() + ") - composed twin of " + filename + " not generated.",
                LOG_TYPE::L_WARNING);
        } else {
            system->generateComposedTwin(filename, composedTwinPathOf(name + "System"));
        }
    }
}

void SSystemFactory::createExperimentalOort(unsigned int nbr, const Vec3f &color)
{
    // The oort belongs to the MAIN solar system only (its geometry is
    // heliocentric); other created systems (createSystem) are foreign stars.
    auto it = modularSystemOf.find("Solar");
    if (it == modularSystemOf.end() || it->second == nullptr) {
        // D12 (F0, §11.102(e4)): the flag ACTS here - by failing - so it says so.
        // Silence made an enabled pilot indistinguishable from a disabled one at
        // the only place the difference is decided.
        cLog::get()->write("B5 §6.9 pilot: flag_experimental_oort is set, but there is no "
            "\"Solar\" modular system node to attach the cloud to - the experimental oort "
            "was NOT instantiated (the old oort cloud is unaffected).", LOG_TYPE::L_WARNING);
        return;
    }
    ModularSystem *system = it->second;

    // ---- Regime low-edge peg (PROVISIONAL, B5) -----------------------------
    // scaledRadius·BODY_SURFACE_HEIGHT (=·2) is the near/grounded->near regime
    // boundary in ModularBody::draw: the cloud (a NEAR component) is hidden for
    // observer distance < this, shown beyond. The old altitude gate turns the
    // cloud on around 1e13 m ≈ 67 AU (measured: off at refDist 66.8 AU, full by
    // 133.7 AU). Radius 50 AU ⇒ boundary 100 AU, inside that ramp. This is a
    // BODY-DATA value feeding the regime, NOT a distance test in the draw path
    // (the §2(a2) foreclosure the pilot avoids). Value TUNED empirically, flagged.
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

    cLog::get()->write("B5 §6.9 pilot: experimental oort modular body instantiated at the "
        "SolarSystem floor (" + std::to_string(nbr) + " points).", LOG_TYPE::L_INFO);
}

void SSystemFactory::loadGalacticSystem(const std::string &path, const std::string &name)
{
    // `path` is a DIRECTORY, and the separator that joins it to a file name
    // belongs HERE - one authority for the join, used by this open and by the
    // per-system open in loadSystem (INTENT §5.37). It was split between caller
    // and callee until `da858612c` (2025-09-20) replaced the caller's
    // `getUserDir()` - a path WITH its trailing '/' - by "." in a batch where
    // every other call had dropped its prefix entirely: from then on this
    // opened ".galactic.ini" and ".stellar_systems/<file>", so on EVERY install
    // no galactic entry was read, no foreign system created, no galactic anchor
    // added (measured 0/66 applogs carry a "Params :" block, §11.109(a)).
    std::string dir = path;
    if (!dir.empty() && dir.back() != '/')
        dir += '/';

    stringHash_t params;

    std::ifstream file(dir + name);
    if (file) {
        // ONE line grammar for the whole .ini family (tools/ini_line.hpp,
        // INTENT §5.38/§5.39/D29). The substr arithmetic this replaces assumed
        // exactly one space on each side of the '=', and the shipped
        // galactic.ini does not oblige: `z =-2.371937` lost its minus sign and
        // `y =1988.889006` its leading digit, putting six of seventeen systems
        // at wrong galactic coordinates the moment the path above is repaired.
        // That is why the two repairs are one commit and never two.
        std::string line, key, value;
        // The header of the section whose params are currently accumulating -
        // carried so a rejected section can be named by the name the AUTHOR
        // wrote, which is the only identifier left when the missing key is
        // `name` itself (§5.45 / §2(f)).
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

// "The author declared nothing" is ABSENT-OR-EMPTY, once: stringHash_t is a
// std::map and operator[] INSERTS an empty entry for an absent key, so a
// find()-only test is true for keys nobody wrote (the §11.103(b) trap).
static const std::string *declaredParam(const stringHash_t &params, const char *key)
{
    const auto it = params.find(key);
    return (it == params.end() || it->second.empty()) ? nullptr : &it->second;
}

// `path` is the directory prefix ALREADY terminated by its separator - the one
// caller (loadGalacticSystem, above) owns that normalization (INTENT §5.37).
//
// §5.45 (found by B40 §11.115(i), fixed 2026-07-30): a section missing `name`,
// `x`, `y` or `z` used to KILL THE APP AT STARTUP - `std::stod("")` throws
// std::invalid_argument and neither main.cpp nor core.cpp has a catch, so the
// process died before opening its port (returncode -6, measured). The shipped
// corpus is well-formed, so this cost nothing today; a paid galactic delivery
// with ONE malformed section refused to start the product.
// The shape is not invented: the addAnchor call below already DECLINES such a
// section and says so (AnchorPointCreator::handle, "x y or z parameter
// missing") - this is that same decision, taken once, at the top, with a
// diagnostic that names the section, the key and the fix (§2(f)) and logs the
// acting default (SKIP - §2.0 D12).
// Guarded on PARSEABILITY, not merely on presence: `x = ,5` (a decimal comma,
// the same author's likely next mistake) and `x = 1e999` throw from that same
// line - invalid_argument and out_of_range - so guarding presence alone would
// fix the instance and leave the class alive (I6). [measured 2026-07-30:
// stod("") / ("abc") / (",5") throw invalid_argument, stod("1e999") throws
// out_of_range; hence catching std::exception, not one of the two.]
// What this deliberately does NOT change: a value stod PARSES is accepted
// exactly as before, partial parses included (`x = 1,5` has always meant 1.0
// here, `x = 1.5 ly` 1.5). Rejecting those would be a new semantic on data
// that loads today - out of scope, recorded at §5.45.
void SSystemFactory::loadSystem(const std::string &path, stringHash_t &params, const std::string &section)
{
    std::cout << "Params :\n";
    for (auto &p : params) {
        std::cout << p.first << " : " << p.second << '\n';
    }
    // Name the section the way its author wrote it; fall back to the `name` key
    // and then to a positional label, so the message is actionable even when
    // the header itself is what is missing.
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

// Contract: ssystem_factory.hpp (setSelected). Out of line because resolving
// the new-path body from a ModularObject needs the bridge's complete type.
void SSystemFactory::setSelected(const Object &obj)
{
    ssystemSelected->setSelected(obj);
    // A ModularObject already HOLDS the body it was resolved from: use it
    // instead of a second lookup by name (I2 - one resolution, so a name
    // carried by two trees can never resolve to a different body here than
    // the one the selection was actually made on).
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
    // Window pixel -> ScreenRect, through the app's own authority (I2:
    // VulkanMgr::screenToRect is what the UI already uses for the mouse).
    // The rect's y grows DOWNWARD (window row 0 maps to -1) while
    // ModularBody::screenPos is the view-space up component, +1 at the TOP,
    // so the pick position is the rect with y negated. This is the only site
    // where the two conventions meet.
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
        // A name with no extension is a system name, not a file name: give it
        // the one every system file has, so `filename Mars` does what it reads
        // like instead of writing a file nothing will ever open.
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
    out << std::setprecision(17) << "{\"type\":\"header\",\"jd\":"
        // The rest of §2 group A beside the date, so the session gate can
        // witness what it restores (b31-design §6.2 T2 asks for field-by-field
        // equality and a dump that carries only the date cannot give it).
        // getTimeSpeedRaw, not getTimeSpeed: the latter reports 0 while a lock
        // is held, which is the rate time IS running at, not the one that was
        // set - and a session records what was set.
        << timeMgr->getJDay() << ",\"timeSpeed\":" << timeMgr->getTimeSpeedRaw()
        << ",\"timePaused\":" << (timeMgr->getTimePause() ? "true" : "false")
        << ",\"helioToEye\":[";
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
    // New-path anchor state (B4 §11.111): which named anchor the camera is
    // attached to, its kind and follow-rotation state, and the declared set.
    // The anchor BODY's own position/frame is in the per-body section below
    // (owned anchor bodies are new-path-only, so they come out with "old":null).
    out << ",\"anchors\":";
    cameraAnchors->dumpState(out);
    // The G4 regime gates as the app itself resolved them this frame (INTENT
    // §5.54). Both forms: the px authority (constants) and the screenSize form
    // the draw path compares in, plus the viewportRadius they were derived
    // from. Without this a harness measuring a regime boundary has to
    // reconstruct the conversion, i.e. assume the thing under test - and the
    // whole point of the px respelling is that the screenSize gates now MOVE
    // with the render width, which is a claim only an observable can carry.
    out << ",\"gates\":{\"viewportRadius\":" << ModularBody::getViewportRadius()
        << ",\"px\":{\"early\":" << BODY_EARLY_VISIBILITY_BOUNDING_SIZE
        << ",\"depthBucket\":" << BODY_DEPTH_BUCKET_BOUNDING_SIZE
        << ",\"full\":" << BODY_FULL_VISIBILITY_BOUNDING_SIZE
        << ",\"close\":" << BODY_CLOSE_RANGE_BOUNDING_SIZE
        << ",\"bigTexture\":" << BODY_BIG_TEXTURE_BOUNDING_SIZE
        << "},\"screenSize\":{\"early\":" << ModularBody::earlyVisibilityGate()
        << ",\"depthBucket\":" << ModularBody::depthBucketGate()
        << ",\"full\":" << ModularBody::fullVisibilityGate()
        << ",\"close\":" << ModularBody::closeRangeGate()
        << ",\"bigTexture\":" << ModularBody::bigTextureGate()
        << "}}";
    // The old path's own view state, written by the owner that has it (Core).
    // §5.63 asked for exactly this readback and it did not exist.
    if (extraHeader) {
        out << ",\"oldView\":";
        extraHeader(out);
    }
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
        if (nb) {
            // A DUMP IS A USE (D8 §11.76(b); B32/§11.93 established this for the
            // spin phase, B39/§11.117 extends it to the position of a body that
            // no longer ticks). Without this the instrument would report a hidden
            // body's hide-time position and call it "now" - i.e. it would measure
            // the freeze instead of the barrier.
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
    // B24 (INTENT 11.78): bodies that exist ONLY in the new path - composed
    // declarations (rover class) have no old-path twin, so the old-system
    // loop above never reaches them. Emitted with "old":null - the mirror of
    // the "new":null finding channel (11.3 class, both directions observable).
    {
        std::set<std::string> dumped;
        for (auto it = currentSystem->begin(); it != currentSystem->end(); ++it)
            dumped.insert(it->first);
        ModularBody::forEach([&, this](ModularBody &nb) {
            if (dumped.count(nb.getEnglishName()))
                return;
            out << "{\"type\":\"body\",\"name\":\"" << nb.getEnglishName()
                << "\",\"old\":null,\"new\":";
            nb.dumpTrace(out);
            out << "}\n";
            // The nav-string sidecar for new-only bodies too (B24-select,
            // INTENT §11.106): the info/nav readouts of a COMPOSED body are
            // now a product surface (it is selectable), and they had no
            // observable at all - the loop above only reaches names the old
            // tree carries. Same bridge, same methods, no OLD counterpart.
            ModularObject bridge;
            bridge.body = &nb;
            navout << nb.getEnglishName()
                   << "\n  NEW nav: " << bridge.getShortInfoNavString(navigation, timeMgr, observatory)
                   << "\n  NEW inf: " << bridge.getInfoString(navigation) << "\n";
        });
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
    // Iapetus (B14 pilot, INTENT 11.68): the first candidate for an
    // absolute_pole declaration on a NON-system-centered parent (Saturn). Its
    // self-hop `tilt` + the Saturn parent hop in the same chain are the
    // pieces the moon-absolute-pole harness composes; measuring it directly
    // closes B28's untested non-system-centered-parent case (§11.67 item 2).
    // Iapetus (Saturn, prograde, no periodic W) is the clean B14-W0 discriminator;
    // Proteus (Neptune, prograde) and Puck (Uranus, RETROGRADE Ẇ) exercise the
    // rot_pole_w0 -> offset conversion across the sign conventions (§11.79(a)).
    // Janus + Prometheus (Saturn, B14-sat6 §11.75(b)): the pole-axis + W0
    // meridian discriminators for the widened 6 non-cluster garbage-tilt moons -
    // Janus carries periodic pole+W nutation (S2 angle), Prometheus is pure
    // secular; both prograde. Instrument only (changes what dual_dump emits).
    // B14-periode (§11.86(d)/§11.87(e)): the FULL 20 pole-landed moons are hopped
    // so the dumped re.period rate observable covers the complete edited scope -
    // every corrected rot_periode is verifiable to its IAU 8640/Wdot value, and
    // Iapetus (unedited) + Venus/planets stay ULP-0 (the commutator control).
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
