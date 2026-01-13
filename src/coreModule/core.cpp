/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2021 of the LSS Team & Association Sirius
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

#include <algorithm>
#include "coreModule/core.hpp"
#include "tools/utility.hpp"
#include "tools/init_parser.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "tools/log.hpp"
#include "coreModule/ubo_cam.hpp"
#include "coreModule/core_common.hpp"
#include "navModule/anchor_manager.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_body.hpp"
#include "appModule/space_date.hpp"
#include "appModule/fontFactory.hpp"
#include "bodyModule/body_decor.hpp"
#include "inGalaxyModule/starNavigator.hpp"
#include "inGalaxyModule/cloudNavigator.hpp"
#include "inGalaxyModule/dsoNavigator.hpp"
#include "coreModule/tully.hpp"
#include "coreModule/oort.hpp"
#include "inGalaxyModule/dso3d.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "coreModule/landscape.hpp"
#include "mediaModule/media.hpp"
#include "coreModule/starLines.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "bodyModule/body_trace.hpp"
#include "eventModule/CoreEvent.hpp"
#include "eventModule/event_recorder.hpp"
#include "coreModule/meteor_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "coreModule/cardinals.hpp"
#include "coreModule/illuminate_mgr.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include "atmosphereModule/skybright.hpp"
#include "coreModule/time_mgr.hpp"
#include "coreModule/sky_localizer.hpp"
#include "ojmModule/ojm_mgr.hpp"
#include "coreModule/nebula_mgr.hpp"
#include "coreModule/constellation_mgr.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "tools/app_settings.hpp"
#include "inGalaxyModule/starGalaxy.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "coreModule/tully.hpp"
#include "coreModule/volumObj3D.hpp"
#include <filesystem>

Core::Core(int width, int height, std::shared_ptr<Media> _media, std::shared_ptr<FontFactory> _fontFactory, const mBoost::callback<void, std::string>& recordCallback, std::shared_ptr<Observer> _observatory) :
	skyTranslator(AppSettings::Instance()->getLanguageDir(), ""),
	projection(nullptr), selected_object(nullptr) // hip_stars(nullptr),
	// illuminates(nullptr), ssystemFactory(nullptr), milky_way(nullptr)
{
	vzm={0.,0.,0.,0.,0.,0.00025};
	recordActionCallback = recordCallback;
	media = _media;
	fontFactory = _fontFactory;
	projection = new Projector( width,height, 60 );
	media->setProjector(projection);
	// Set textures directory and suffix
	s_texture::setTexDir(AppSettings::Instance()->getTextureDir() );
	//set Shaders directory and suffix
	uboCam = std::make_unique<UBOCam>();
	timeMgr = std::make_shared<TimeMgr>();
	navigation = new Navigator();
	observatory = _observatory;

	currentToneConverter.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<ToneReproductor>());
	currentToneConverter.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<ToneReproductor>());
	currentToneConverter.setActive(CURRENT_MODE::NORMAL_MODE);

	currentAtmosphere.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<Atmosphere>());
	currentAtmosphere.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<Atmosphere>());
	currentAtmosphere.setActive(CURRENT_MODE::NORMAL_MODE);

	currentSsystemFactory.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<SSystemFactory>(observatory.get(), navigation, timeMgr.get()));
	currentSsystemFactory.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<SSystemFactory>(observatory.get(), navigation, timeMgr.get()));
	currentSsystemFactory.setActive(CURRENT_MODE::NORMAL_MODE);

	currentNebulas.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<NebulaMgr>());
	currentNebulas.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<NebulaMgr>());
	currentNebulas.setActive(CURRENT_MODE::NORMAL_MODE);

	currentMilkyWay.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<MilkyWay>());
	currentMilkyWay.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<MilkyWay>());
	currentMilkyWay.setActive(CURRENT_MODE::NORMAL_MODE);

	currentStarNav.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<StarNavigator>());
	currentStarNav.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<StarNavigator>());
	currentStarNav.setActive(CURRENT_MODE::NORMAL_MODE);

	currentCloudNav.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<CloudNavigator>());
	currentCloudNav.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<CloudNavigator>());
	currentCloudNav.setActive(CURRENT_MODE::NORMAL_MODE);

	universeCloudNav = std::make_unique<CloudNavigator>(AppSettings::Instance()->getConfigDir() + "gal3d.dat");

	currentStarGalaxy.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<StarGalaxy>(AppSettings::Instance()->getConfigDir() + "gal3d.dat"));
	currentStarGalaxy.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<StarGalaxy>(AppSettings::Instance()->getConfigDir() + "gal3d.dat"));
	currentStarGalaxy.setActive(CURRENT_MODE::NORMAL_MODE);

	if (std::filesystem::exists(s_texture::getTexDir() + "milkyway-vguerin-d128.png")) {
		currentVolumGalaxy.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<VolumObj3D>("milkyway-vguerin-d128.png", "", false));
		currentVolumGalaxy.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<VolumObj3D>("milkyway-vguerin-d128.png", "", false));
		currentVolumGalaxy.setActive(CURRENT_MODE::NORMAL_MODE);
	} else {
		currentVolumGalaxy.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<VolumObj3D>("mw_rgb_d8.jpg", "mw_d32.png", true));
		currentVolumGalaxy.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<VolumObj3D>("mw_rgb_d8.jpg", "mw_d32.png", true));
		currentVolumGalaxy.setActive(CURRENT_MODE::NORMAL_MODE);
	}

	currentDsoNav.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<DsoNavigator>());
	currentDsoNav.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<DsoNavigator>());
	currentDsoNav.setActive(CURRENT_MODE::NORMAL_MODE);

	currentStarLines.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<StarLines>());
	currentStarLines.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<StarLines>());
	currentStarLines.setActive(CURRENT_MODE::NORMAL_MODE);

	ojmMgr = std::make_unique<OjmMgr>();  // Manages mode internally

	currentBodyDecor.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<BodyDecor>(currentMilkyWay.get(CURRENT_MODE::NORMAL_MODE),  currentAtmosphere.get(CURRENT_MODE::NORMAL_MODE)));
	currentBodyDecor.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<BodyDecor>(currentMilkyWay.get(CURRENT_MODE::SANDBOX_MODE), currentAtmosphere.get(CURRENT_MODE::SANDBOX_MODE)));
	currentBodyDecor.setActive(CURRENT_MODE::NORMAL_MODE);

	currentSkyGridMgr.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<SkyGridMgr>());
	currentSkyGridMgr.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<SkyGridMgr>());
	currentSkyGridMgr.setActive(CURRENT_MODE::NORMAL_MODE);
	currentSkyGridMgr.applyToAll([](SkyGridMgr &mgr) {
		mgr.Create(SKYGRID_TYPE::GRID_EQUATORIAL);
		mgr.Create(SKYGRID_TYPE::GRID_ECLIPTIC);
		mgr.Create(SKYGRID_TYPE::GRID_GALACTIC);
		mgr.Create(SKYGRID_TYPE::GRID_ALTAZIMUTAL);
	});

	currentSkyLineMgr.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<SkyLineMgr>());
	currentSkyLineMgr.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<SkyLineMgr>());
	currentSkyLineMgr.setActive(CURRENT_MODE::NORMAL_MODE);
	currentSkyLineMgr.applyToAll([](SkyLineMgr &mgr) {
		mgr.Create(SKYLINE_TYPE::LINE_CIRCLE_POLAR);
		mgr.Create(SKYLINE_TYPE::LINE_POINT_POLAR);
		mgr.Create(SKYLINE_TYPE::LINE_ECLIPTIC_POLE);
		mgr.Create(SKYLINE_TYPE::LINE_GALACTIC_POLE);
		mgr.Create(SKYLINE_TYPE::LINE_ANALEMMA);
		mgr.Create(SKYLINE_TYPE::LINE_ANALEMMALINE);
		mgr.Create(SKYLINE_TYPE::LINE_CIRCUMPOLAR);

		mgr.Create(SKYLINE_TYPE::LINE_GALACTIC_CENTER);
		mgr.Create(SKYLINE_TYPE::LINE_VERNAL);
		mgr.Create(SKYLINE_TYPE::LINE_GREENWICH);
		mgr.Create(SKYLINE_TYPE::LINE_ARIES);
		mgr.Create(SKYLINE_TYPE::LINE_EQUATOR);
		mgr.Create(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR);

		mgr.Create(SKYLINE_TYPE::LINE_MERIDIAN);
		mgr.Create(SKYLINE_TYPE::LINE_TROPIC);
		mgr.Create(SKYLINE_TYPE::LINE_ECLIPTIC);
		mgr.Create(SKYLINE_TYPE::LINE_PRECESSION);
		mgr.Create(SKYLINE_TYPE::LINE_VERTICAL);
		mgr.Create(SKYLINE_TYPE::LINE_ZODIAC);
		mgr.Create(SKYLINE_TYPE::LINE_ZENITH);
	});

	currentSkyDisplayMgr.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<SkyDisplayMgr>());
	currentSkyDisplayMgr.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<SkyDisplayMgr>());
	currentSkyDisplayMgr.setActive(CURRENT_MODE::NORMAL_MODE);
	currentSkyDisplayMgr.applyToAll([](SkyDisplayMgr &mgr) {
		mgr.Create(SKYDISPLAY_NAME::SKY_PERSONAL);
		mgr.Create(SKYDISPLAY_NAME::SKY_PERSONEQ);
		mgr.Create(SKYDISPLAY_NAME::SKY_NAUTICAL);
		mgr.Create(SKYDISPLAY_NAME::SKY_NAUTICEQ);
		mgr.Create(SKYDISPLAY_NAME::SKY_OBJCOORDS);
		mgr.Create(SKYDISPLAY_NAME::SKY_MOUSECOORDS);
		mgr.Create(SKYDISPLAY_NAME::SKY_ANGDIST);
		mgr.Create(SKYDISPLAY_NAME::SKY_LOXODROMY);
		mgr.Create(SKYDISPLAY_NAME::SKY_ORTHODROMY);
	});

	cardinals_points = std::make_unique<Cardinals>();

	currentMeteors.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<MeteorMgr>(10, 60));
	currentMeteors.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<MeteorMgr>(10, 60));
	currentMeteors.setActive(CURRENT_MODE::NORMAL_MODE);

	currentLandscape.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<Landscape>());
	currentLandscape.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<Landscape>());
	currentLandscape.setActive(CURRENT_MODE::NORMAL_MODE);

	skyloc = std::make_unique<SkyLocalizer>(AppSettings::Instance()->getSkyCultureDir());

	currentHipStars.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<HipStarMgr>(VulkanMgr::instance->getScreenRect().extent.width, VulkanMgr::instance->getScreenRect().extent.height));
	currentHipStars.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<HipStarMgr>(VulkanMgr::instance->getScreenRect().extent.width, VulkanMgr::instance->getScreenRect().extent.height));
	currentHipStars.setActive(CURRENT_MODE::NORMAL_MODE);

	currentAsterisms.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<ConstellationMgr>(currentHipStars.get(CURRENT_MODE::NORMAL_MODE)));
	currentAsterisms.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<ConstellationMgr>(currentHipStars.get(CURRENT_MODE::SANDBOX_MODE)));
	currentAsterisms.setActive(CURRENT_MODE::NORMAL_MODE);

	currentIlluminates.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<IlluminateMgr>(currentHipStars.get(CURRENT_MODE::NORMAL_MODE),  navigation, currentAsterisms.get(CURRENT_MODE::NORMAL_MODE)));
	currentIlluminates.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<IlluminateMgr>(currentHipStars.get(CURRENT_MODE::SANDBOX_MODE), navigation, currentAsterisms.get(CURRENT_MODE::SANDBOX_MODE)));
	currentIlluminates.setActive(CURRENT_MODE::NORMAL_MODE);

	currentOort.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<Oort>());
	currentOort.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<Oort>());
	currentOort.setActive(CURRENT_MODE::NORMAL_MODE);

	currentDso3d.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<Dso3d>());
	currentDso3d.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<Dso3d>());
	currentDso3d.setActive(CURRENT_MODE::NORMAL_MODE);

	currentTully.set(CURRENT_MODE::NORMAL_MODE,  std::make_unique<Tully>());
	currentTully.set(CURRENT_MODE::SANDBOX_MODE, std::make_unique<Tully>());
	currentTully.setActive(CURRENT_MODE::NORMAL_MODE);

	object_pointer_visibility = 1;
}

void Core::registerCoreFont() const
{
	currentHipStars.applyToAll(&HipStarMgr::registerFont,           fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));
	currentNebulas.applyToAll(&NebulaMgr::registerFont,             fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));
	currentDso3d.applyToAll(&Dso3d::registerFont,                   fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));
	currentStarNav.applyToAll(&StarNavigator::registerFont,         fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));
	currentTully.applyToAll(&Tully::registerFont,                   fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));
	currentSsystemFactory.applyToAll(&SSystemFactory::registerFont, fontFactory->registerFont(CLASSEFONT::CLASS_SSYSTEM));
	currentSkyGridMgr.applyToAll(&SkyGridMgr::registerFont,         fontFactory->registerFont(CLASSEFONT::CLASS_SKYGRID));
	currentSkyLineMgr.applyToAll(&SkyLineMgr::registerFont,         fontFactory->registerFont(CLASSEFONT::CLASS_SKYLINE));
	currentSkyDisplayMgr.applyToAll(&SkyDisplayMgr::registerFont,   fontFactory->registerFont(CLASSEFONT::CLASS_SKYDISPLAY));
	currentNebulas.applyToAll(&NebulaMgr::registerFont,             fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));
	currentAsterisms.applyToAll(&ConstellationMgr::registerFont,    fontFactory->registerFont(CLASSEFONT::CLASS_ASTERIMS));

	cardinals_points->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_CARDINALS));
}

std::string Core::getListMatchingObjects(const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> tmp;
	std::string msgToSend;
	tmp = listMatchingObjectsI18n(objPrefix, maxNbItem,true);
	for( std::vector<std::string>::const_iterator itr = tmp.begin(); itr != tmp.end(); ++itr ) {
		msgToSend = msgToSend + (*itr)+";";
	}
	return msgToSend;
}


Core::~Core()
{
	// release the previous Object:
	selected_object = Object();
	old_selected_object = Object();
	// delete bodyDecor;
	// bodyDecor = nullptr;
	delete navigation;
	navigation = nullptr;
	delete projection;
	projection = nullptr;
	delete geodesic_grid;
	geodesic_grid = nullptr;
	// s_font::deleteShader();
	//delete ssystem;
	//delete skyloc;
	//skyloc = nullptr;
	Object::deleteTextures(); // Unload the pointer textures
	ObjectBase::uninit();
	// Object::deleteShaders();
	//delete text_usr;
	//delete uboCam;
}

void Core::setFlagNav(bool a)
{
	flagNav=a;
	cardinals_points->setInternalNav(a);
	currentSkyGridMgr->setInternalNav(a);
	currentSkyLineMgr->setInternalNav(a);
}

void Core::setFlagAstronomical(bool a)
{
	flagAstronomical = a;
	cardinals_points->setInternalAstronomical(a);
	currentSkyGridMgr->setInternalAstronomical(a);
	currentSkyLineMgr->setInternalAstronomical(a);
}

//! Load core data and initialize with default values
void Core::init(const InitParser& conf)
{
	if (firstTime) {
		//s_font::initBaseFont(AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_GENERAL_NAME));
		s_texture::setLoadInLowResolution(conf.getBoolean(SCS_RENDERING, SCK_LOW_RES), conf.getInt(SCS_RENDERING, SCK_LOW_RES_MAX) );
		this->registerCoreFont();
	}

	flagNav= conf.getBoolean(SCS_NAVIGATION, SCK_FLAG_NAVIGATION);
	setFlagNav(flagNav);
	flagAstronomical = conf.getBoolean(SCS_NAVIGATION, SCK_FLAG_NAVIGATION);
	setFlagAstronomical(flagAstronomical);
	FlagAtmosphericRefraction = conf.getBoolean(SCS_VIEWING,SCK_FLAG_ATMOSPHERIC_REFRACTION);

	initialvalue.initial_landscapeName=conf.getStr(SCS_INIT_LOCATION,SCK_LANDSCAPE_NAME);
	currentIlluminates.applyToAll(&IlluminateMgr::setDefaultSize, conf.getDouble(SCS_STARS, SCK_ILLUMINATE_SIZE));

	// Start splash with no fonts due to font collection delays
	if (firstTime) {
		// Init the solar system first
		currentSsystemFactory.applyToAll([&conf](SSystemFactory &factory) {
			factory.iniColor(conf.getStr(SCS_COLOR, SCK_PLANET_HALO_COLOR),
									conf.getStr(SCS_COLOR, SCK_PLANET_NAMES_COLOR),
									conf.getStr(SCS_COLOR, SCK_PLANET_ORBITS_COLOR),
									conf.getStr(SCS_COLOR, SCK_OBJECT_TRAILS_COLOR));

			factory.iniTess(conf.getInt(SCS_RENDERING, SCK_MIN_TES_LEVEL),
									conf.getInt(SCS_RENDERING, SCK_MAX_TES_LEVEL),
									conf.getInt(SCS_RENDERING, SCK_PLANET_ALTIMETRY_LEVEL),
									conf.getInt(SCS_RENDERING, SCK_MOON_ALTIMETRY_LEVEL),
									conf.getInt(SCS_RENDERING, SCK_EARTH_ALTIMETRY_LEVEL));

			factory.modelRingInit(conf.getInt(SCS_RENDERING, SCK_RINGS_LOW),
										conf.getInt(SCS_RENDERING, SCK_RINGS_MEDIUM),
										conf.getInt(SCS_RENDERING, SCK_RINGS_HIGH));

			factory.iniTextures();
		});

		currentSsystemFactory.applyTo(CURRENT_MODE::NORMAL_MODE, [&conf](SSystemFactory &factory) {
			factory.load(AppSettings::Instance()->getUserDir() + "ssystem.ini");

			factory.anchorManagerInit(conf);
			//TODO Oli: remember to use file selection class.
			factory.loadGalacticSystem(AppSettings::Instance()->getUserDir(), "galactic.ini");
		});

		// Init stars
		currentHipStars.applyToAll([&conf](HipStarMgr &mgr) {
			mgr.iniColorTable();
			mgr.readColorTable();
		});// Set color table for all modes
		currentHipStars.applyTo(CURRENT_MODE::NORMAL_MODE, [&conf](HipStarMgr &mgr) {
			mgr.init(conf);
		}); // Initialize (get data) only for normal mode

		// Init nebulas
		currentNebulas.get(CURRENT_MODE::NORMAL_MODE)->loadDeepskyObject(AppSettings::Instance()->getUserDir() + "deepsky_objects.fab");

		Landscape::createSC_context();
		Landscape::setSlices(conf.getInt(SCS_RENDERING, SCK_LANDSCAPE_SLICES));
		Landscape::setStacks(conf.getInt(SCS_RENDERING, SCK_LANDSCAPE_STACKS));
		setLandscape(initialvalue.initial_landscapeName);

		currentStarNav.get(CURRENT_MODE::NORMAL_MODE)->loadData(AppSettings::Instance()->getUserDir() + "hip2007.txt", false);
		currentStarLines.get(CURRENT_MODE::NORMAL_MODE)->loadCat(AppSettings::Instance()->getUserDir() + "asterism.txt", false);
	}
	currentSsystemFactory.applyToAll(&SSystemFactory::reloadColors, AppSettings::Instance()->getUserDir() + "ssystem.ini");

	// Astro section
	currentHipStars.applyToAll([&conf](HipStarMgr &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STARS));
		mgr.setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
		mgr.setScale(conf.getDouble (SCS_STARS, SCK_STAR_SCALE));
		mgr.setFlagTwinkle(conf.getBoolean(SCS_STARS, SCK_FLAG_STAR_TWINKLE));
		mgr.setTwinkleAmount(conf.getDouble (SCS_STARS, SCK_STAR_TWINKLE_AMOUNT));
		mgr.setMaxMagName(conf.getDouble (SCS_STARS, SCK_MAX_MAG_STAR_NAME));
		mgr.setMagScale(conf.getDouble (SCS_STARS, SCK_STAR_MAG_SCALE));

		mgr.setMagConverterMaxFov(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAX_FOV));
		mgr.setMagConverterMinFov(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MIN_FOV));
		mgr.setMagConverterMagShift(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAG_SHIFT));
		mgr.setMagConverterMaxMag(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAX_MAG));
		mgr.setStarSizeLimit(conf.getDouble(SCS_ASTRO,SCK_STAR_SIZE_LIMIT));
		mgr.setMagConverterMaxScaled60DegMag(conf.getDouble(SCS_STARS,SCK_STAR_LIMITING_MAG));
	});

	currentStarNav.applyToAll([&conf](StarNavigator &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STARS));
		mgr.setMagConverterMagShift(conf.getDouble(SCS_STARS,SCK_MAG_CONVERTER_MAG_SHIFT));
		mgr.setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
		mgr.setMagConverterMaxMag(conf.getDouble(SCS_STARS,SCK_MAG_CONVERTER_MAX_MAG));
		mgr.setStarSizeLimit(conf.getDouble(SCS_ASTRO,SCK_STAR_SIZE_LIMIT));
		mgr.setScale(conf.getDouble (SCS_STARS, SCK_STAR_SCALE));
		mgr.setMaxMagName(conf.getDouble (SCS_STARS, SCK_MAX_MAG_STAR_NAME));
		mgr.setMagScale(conf.getDouble (SCS_STARS, SCK_STAR_MAG_SCALE));
	});

	currentDso3d.applyToAll([&conf](Dso3d &mgr) {
		mgr.setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
	});

	currentSsystemFactory.applyToAll([&conf](SSystemFactory &mgr) {
		mgr.setFlagPlanets(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS));
		mgr.setFlagHints(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_HINTS));
		mgr.setFlagPlanetsOrbits(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_ORBITS));
		mgr.setFlagLightTravelTime(conf.getBoolean(SCS_ASTRO, SCK_FLAG_LIGHT_TRAVEL_TIME));
		mgr.setFlagTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
		mgr.startTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
	});

	currentNebulas.applyToAll([&conf](NebulaMgr &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA));
		mgr.setFlagHints(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_HINTS));
		mgr.setNebulaNames(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_NAMES));
		mgr.setMaxMagHints(conf.getDouble(SCS_ASTRO, SCK_MAX_MAG_NEBULA_NAME));
	});

	currentMilkyWay.applyToAll([&conf](MilkyWay &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_MILKY_WAY));
		mgr.setFlagZodiacal(conf.getBoolean(SCS_ASTRO,SCK_FLAG_ZODIACAL_LIGHT));
	});

	currentStarLines.applyToAll([&conf](StarLines &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_STAR_LINES));
	});

	currentNebulas.applyToAll([&conf](NebulaMgr &mgr) {
		mgr.setPictoSize(conf.getInt(SCS_VIEWING,SCK_NEBULA_PICTO_SIZE));
		mgr.setFlagBright(conf.getBoolean(SCS_ASTRO,SCK_FLAG_BRIGHT_NEBULAE));
	});

	currentSsystemFactory.applyTo(CURRENT_MODE::NORMAL_MODE,  &SSystemFactory::setScale, currentHipStars.get(CURRENT_MODE::NORMAL_MODE)->getScale());
	currentSsystemFactory.applyTo(CURRENT_MODE::SANDBOX_MODE, &SSystemFactory::setScale, currentHipStars.get(CURRENT_MODE::SANDBOX_MODE)->getScale());

	setPlanetsSizeLimit(conf.getDouble(SCS_ASTRO, SCK_PLANET_SIZE_MARGINAL_LIMIT));

	currentSsystemFactory.applyToAll([](SSystemFactory &mgr) {
		mgr.setFlagClouds(true);
	});

	observatory->load(conf, SCS_INIT_LOCATION);
	observatory->setEyeRelativeMode(false);

	// make sure nothing selected or tracked
	deselect();
	setHomePlanet("Earth");
	navigation->setFlagTraking(0);
	navigation->setFlagLockEquPos(0);

	timeMgr->setTimeSpeed(JD_SECOND);  // reset to real time

	timeMgr->setJDay(SpaceDate::JulianFromSys());
	navigation->setLocalVision(Vec3f(1,1e-05,0.2));

	if (firstTime) {
		currentMilkyWay.applyToAll([&conf](MilkyWay &mgr) {
			mgr.needToUseIris(conf.getBoolean(SCS_MAIN, SCK_MILKYWAY_IRIS));
			mgr.defineInitialMilkywayState(AppSettings::Instance()->getTextureDir() , conf.getStr(SCS_ASTRO,SCK_MILKY_WAY_TEXTURE),
					conf.getStr(SCS_ASTRO,SCK_MILKY_WAY_IRIS_TEXTURE), conf.getDouble(SCS_ASTRO,SCK_MILKY_WAY_INTENSITY));
			mgr.defineZodiacalState(AppSettings::Instance()->getTextureDir() + conf.getStr(SCS_ASTRO,SCK_ZODIACAL_LIGHT_TEXTURE), conf.getDouble(SCS_ASTRO,SCK_ZODIACAL_INTENSITY));
			mgr.setFaderDuration(conf.getInt(SCS_ASTRO,SCK_MILKY_WAY_FADER_DURATION));
		});

		currentAtmosphere.applyToAll([this, &conf](Atmosphere &mgr) {
			mgr.initGridViewport(projection);
			mgr.initGridPos();
		});

		// Create a oort cloud in both modes
		// sandbox separated to allow different settings in future
		// (for now only used for the show/hide flag without altering normal mode)
		currentOort.applyToAll([&conf](Oort &mgr) {
			mgr.populate(conf.getInt("rendering","oort_elements"));
			mgr.build();
		});

		currentTully.applyToAll([&conf](Tully &mgr) {
			mgr.setTexture("typegals.png");
		});
		currentTully.applyTo(CURRENT_MODE::NORMAL_MODE, [&conf](Tully &mgr) {
			mgr.loadCatalog(AppSettings::Instance()->getUserDir() + "tully.dat");
			mgr.loadBigCatalog(AppSettings::Instance()->getUserDir() + "6df.dat", 5e+12);
		}); // Load data only in normal mode
		currentTully.applyToAll([&conf](Tully &mgr) {
			mgr.setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
		});

		currentDso3d.applyToAll([](Dso3d &mgr) {
			mgr.setTexture("dsocat.png");
		});
		currentDso3d.applyTo(CURRENT_MODE::NORMAL_MODE, [](Dso3d &mgr) {
			if (mgr.loadCatalog(AppSettings::Instance()->getUserDir() + "dso3d.dat"))
				mgr.build();
		}); // Load data only in normal mode

		ojmMgr->init();
		// 3D object integration test
		if (currentVolumGalaxy.get(CURRENT_MODE::NORMAL_MODE)->loaded()) {
			if (std::filesystem::exists(s_texture::getTexDir() + "milkyway-vguerin-d128.png")) {
				currentVolumGalaxy.get(CURRENT_MODE::NORMAL_MODE)->setModel(Mat4f::translation(Vec3f( -0.002, 0.0001, -0.005)) * Mat4f::yawPitchRoll(112, 0, 90) * Mat4f::scaling(0.01), Vec3f(1, 1, 1/8.));
			} else {
				currentVolumGalaxy.get(CURRENT_MODE::NORMAL_MODE)->setModel(Mat4f::translation(Vec3f( -0.002, 0.0001, -0.005)) * Mat4f::yawPitchRoll(112, 0, 0) * Mat4f::scaling(0.01), Vec3f(1, 1, 1/8.));
			}
		} else
			ojmMgr->load("in_universe", "Milkyway", AppSettings::Instance()->getModel3DDir() + "Milkyway/Milkyway.ojm",AppSettings::Instance()->getModel3DDir()+"Milkyway/", Vec3f(0.0000001,0.0000001,0.0000001), 0.01);

		// Load the pointer textures
		Object::initTextures();
		ObjectBase::createShaderStarPointeur();
		ObjectBase::createShaderPointeur();
		ObjectBase::setFontResolution(conf.getInt(SCS_FONT, SCK_FONT_RESOLUTION_SIZE));
		//Init of the text's shaders
		s_font::createSC_context();
	} else {
		currentMilkyWay.applyToAll([](MilkyWay &mgr) {
			mgr.restoreDefaultMilky();
		});
	}

	currentToneConverter.applyTo(CURRENT_MODE::NORMAL_MODE,  &ToneReproductor::setWorldAdaptationLuminance, 3.75f + currentAtmosphere.get(CURRENT_MODE::NORMAL_MODE )->getIntensity()*40000.f);
	currentToneConverter.applyTo(CURRENT_MODE::SANDBOX_MODE, &ToneReproductor::setWorldAdaptationLuminance, 3.75f + currentAtmosphere.get(CURRENT_MODE::SANDBOX_MODE)->getIntensity()*40000.f);

	// Compute planets data and init viewing position position of sun and all the satellites (ie planets)
	currentSsystemFactory.applyToAll([this](SSystemFactory &mgr) {
		mgr.computePositions(timeMgr->getJDay(), observatory.get());
	});

	// Compute transform matrices between coordinates systems
	navigation->updateTransformMatrices(observatory.get(), timeMgr->getJDay());
	navigation->updateViewMat(projection->getFov());

	currentSsystemFactory.applyToAll([](SSystemFactory &mgr) {
		mgr.setSelected("");
	});

	std::string skyLocaleName = conf.getStr(SCS_LOCALIZATION, SCK_SKY_LOCALE);
	initialvalue.initial_skyLocale=skyLocaleName;
	setSkyLanguage(skyLocaleName, true);

	int grid_level = currentHipStars.get(CURRENT_MODE::NORMAL_MODE)->getMaxGridLevel();
	geodesic_grid = new GeodesicGrid(grid_level);
	currentHipStars.applyToAll([this](HipStarMgr &mgr) {
		mgr.setGrid(geodesic_grid);
	});

	FlagEnableZoomKeys	= conf.getBoolean(SCS_NAVIGATION, SCK_FLAG_ENABLE_ZOOM_KEYS);
	FlagEnableMoveKeys  = conf.getBoolean(SCS_NAVIGATION, SCK_FLAG_ENABLE_MOVE_KEYS);
	setFlagManualAutoZoom( conf.getBoolean(SCS_NAVIGATION, SCK_FLAG_MANUAL_ZOOM) );

	setAutoMoveDuration( conf.getDouble (SCS_NAVIGATION, SCK_AUTO_MOVE_DURATION) );
	vzm.move_speed			= conf.getDouble(SCS_NAVIGATION, SCK_MOVE_SPEED);
	vzm.zoom_speed			= conf.getDouble(SCS_NAVIGATION, SCK_ZOOM_SPEED);

	// Viewing Mode
	std::string tmpstr = conf.getStr(SCS_NAVIGATION,SCK_VIEWING_MODE);
	if (tmpstr=="equator") 	navigation->setViewingMode(Navigator::VIEW_EQUATOR);
	else {
		if (tmpstr=="horizon") navigation->setViewingMode(Navigator::VIEW_HORIZON);
		else {
			std::cerr << "ERROR : Unknown viewing mode type : " << tmpstr << std::endl;
			assert(0);
		}
	}

	InitFov				= conf.getDouble (SCS_NAVIGATION,SCK_INIT_FOV);
	projection->setFov(InitFov);

	double heading = conf.getDouble (SCS_NAVIGATION,SCK_HEADING);
	navigation->setHeading(heading);
	navigation->setDefaultHeading(heading);

	currentMeteors.applyToAll([&conf](MeteorMgr &mgr) {
		mgr.setZHR(conf.getInt(SCS_ASTRO,SCK_METEOR_RATE));
	});

	InitViewPos = Utility::strToVec3f(conf.getStr(SCS_NAVIGATION,SCK_INIT_VIEW_POS).c_str());

	double viewOffset = conf.getDouble (SCS_NAVIGATION,SCK_VIEW_OFFSET);

	setViewOffset(viewOffset);

	// Load constellations from the correct sky culture
	std::string tmp = conf.getStr(SCS_LOCALIZATION, SCK_SKY_CULTURE);
	initialvalue.initial_skyCulture=tmp;
	setSkyCultureDir(tmp, true);
	skyCultureDir = tmp;

	// Landscape section
	currentLandscape.applyToAll([&conf](Landscape &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_LANDSCAPE, SCK_FLAG_LANDSCAPE));
		mgr.fogSetFlagShow(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_FOG));
	});

	currentBodyDecor.applyToAll([&conf](BodyDecor &mgr) {
		mgr.setAtmosphereState(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_ATMOSPHERE));
	});

	currentAtmosphere.applyToAll([&conf](Atmosphere &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_ATMOSPHERE));
		mgr.setFaderDuration(conf.getDouble(SCS_VIEWING,SCK_ATMOSPHERE_FADE_DURATION));
		mgr.setDefaultFaderDuration(conf.getDouble(SCS_VIEWING,SCK_ATMOSPHERE_FADE_DURATION));
		mgr.setDefaultMoonBrightness(conf.getDouble(SCS_VIEWING,SCK_MOON_BRIGHTNESS));
	});
	// Always disable atmosphere in sandbox mode at init since there is nothing at all
	currentAtmosphere.applyTo(CURRENT_MODE::SANDBOX_MODE, &Atmosphere::setFlagShow, false);

	// Sandbox mode has no sun at init, so we can't set its brightness, so only normal mode
	currentSsystemFactory.applyTo(CURRENT_MODE::NORMAL_MODE,
		static_cast<void (SSystemFactory::*)(double)>(&SSystemFactory::setDefaultSunBrightness),
		conf.getDouble(SCS_VIEWING,SCK_SUN_BRIGHTNESS));

	// Viewing section
	currentAsterisms.applyToAll([&conf](ConstellationMgr &mgr) {
		mgr.setFlagLines(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_DRAWING));
		mgr.setFlagNames(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_NAME));
		mgr.setFlagBoundaries(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_BOUNDARIES));
		mgr.setFlagArt(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_ART));
		mgr.setFlagIsolateSelected(conf.getBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_PICK));
		mgr.setArtIntensity(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_INTENSITY));
		mgr.setArtFadeDuration(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_FADE_DURATION));
	});

	currentSkyGridMgr.applyToAll([&conf](SkyGridMgr &mgr) {
		mgr.setFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_AZIMUTAL_GRID));
		mgr.setFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_EQUATORIAL_GRID));
		mgr.setFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_GRID));
		mgr.setFlagShow(SKYGRID_TYPE::GRID_GALACTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_GRID));
	});

	currentSkyLineMgr.applyToAll([&conf](SkyLineMgr &mgr) {
		mgr.setFlagShow(SKYLINE_TYPE::LINE_EQUATOR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_EQUATOR_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_PRECESSION, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PRECESSION_CIRCLE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_CIRCUMPOLAR_CIRCLE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_TROPIC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_TROPIC_LINES));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_MERIDIAN, conf.getBoolean(SCS_VIEWING,SCK_FLAG_MERIDIAN_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_ZENITH, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ZENITH_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_POLAR_CIRCLE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_POLAR_POINT));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_CENTER));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_POLE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_CENTER));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_VERNAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_VERNAL_POINTS));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANALEMMA_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_ANALEMMA, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANALEMMA));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_ARIES, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ARIES_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_ZODIAC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ZODIAC));
	});

	currentSkyDisplayMgr.applyToAll([&conf](SkyDisplayMgr &mgr) {
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_PERSONAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONAL));
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_PERSONEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONEQ));
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_ALT));
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_RA));

		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_OBJCOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_OBJECT_COORDINATES));
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_MOUSECOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_MOUSE_COORDINATES));
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_ANGDIST, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANGULAR_DISTANCE));
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_LOXODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_LOXODROMY));
		mgr.setFlagShow(SKYDISPLAY_NAME::SKY_ORTHODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ORTHODROMY));
	});

	currentSkyLineMgr.applyToAll([&conf](SkyLineMgr &mgr) {
		mgr.setFlagShow(SKYLINE_TYPE::LINE_GREENWICH, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GREENWICH_LINE));
		mgr.setFlagShow(SKYLINE_TYPE::LINE_VERTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_VERTICAL_LINE));
	});

	cardinals_points->setFlagShow(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CARDINAL_POINTS));

	currentSsystemFactory.applyTo(CURRENT_MODE::NORMAL_MODE, [&conf](SSystemFactory &mgr) {
		mgr.setFlagMoonScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_MOON_SCALED));
		mgr.setMoonScale(conf.getDouble (SCS_VIEWING,SCK_MOON_SCALE), true); //? always true TODO
		mgr.setFlagSunScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_SUN_SCALED));
		mgr.setSunScale(conf.getDouble (SCS_VIEWING,SCK_SUN_SCALE), true); //? always true TODO
	}); // Sandbox mode has no sun and moon at init, so we can't set their scale

	currentOort.applyToAll([&conf](Oort &mgr) {
		mgr.setFlagShow(conf.getBoolean(SCS_VIEWING,SCK_FLAG_OORT));
	});

	setLightPollutionLimitingMagnitude(conf.getDouble(SCS_VIEWING,SCK_LIGHT_POLLUTION_LIMITING_MAGNITUDE), true);

	// currentAtmosphere.applyToAll([&conf](Atmosphere &mgr) {
	// 	mgr.setFlagOptoma(conf.getBoolean(SCS_MAIN, SCK_FLAG_OPTOMA));
	// });

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Load bodies only in normal mode only (sandbox has nothing at init)
	currentSsystemFactory.applyTo(CURRENT_MODE::NORMAL_MODE, [](SSystemFactory &mgr) {
		mgr.initialSolarSystemBodies();
	});
	setBodyDecor(true);
	firstTime = 0;
}

void Core::uboCamUpdate()
{
	uboCam->update();
	uboCam->setViewport(projection->getViewport());
	uboCam->setClippingFov(projection->getClippingFov());
	uboCam->setViewportCenter(projection->getViewportFloatCenter());
	uboCam->setMVP2D(projection->getMatProjectionOrtho2D());
}

std::string Core::getSkyCulture() const {
	return skyloc->directoryToSkyCultureI18(skyCultureDir);
}

std::string Core::getSkyCultureListI18() const {
	return skyloc->getSkyCultureListI18();
}

std::string Core::getSkyCultureHash() const {
	return skyloc->getSkyCultureHash();
}

//! set flag to display generic Hint or specific DSO type
void Core::setDsoPictograms (bool value) {
	currentNebulas->setDisplaySpecificHint(value);
}
//! get flag to display generic Hint or specific DSO type
bool Core::getDsoPictograms () {
	return currentNebulas->getDisplaySpecificHint();
}

//! Execute commun first drawing functions
void Core::applyClippingPlanes(float clipping_min, float clipping_max)
{
	// Init openGL viewing with fov, screen size and clip planes
	projection->setClippingPlanes(clipping_min ,clipping_max);
	// Init viewport to current projector values
	projection->applyViewport();
}

void Core::setLandscapeToBody()
{
	if (!observatory->isOnBody())
		return;

	// here you have to get the planet you are on to access the modelAtmosphere field in AtmosphereParams of the body in question
	currentAtmosphere->setModel(observatory->getHomeBody()->getAtmosphereParams()->modelAtmosphere);

	//std::cout << "Core::setLandscapeToBody()" << std::endl;
	if (!autoLandscapeMode)
		return;

	//std::cout << "Core::setLandscapeToBody() with automode" << std::endl;

	// case of the sun
	if (observatory->isSun()) {
		this->setLandscape("sun");
	}

	//case of the planets except the Earth
	if (!observatory->isEarth() && !observatory->getHomeBody()->isSatellite()){
		setLandscape(observatory->getHomeBody()->getEnglishName());
		currentAtmosphere->setFlagShow(true);
		currentBodyDecor->setAtmosphereState(true);
	}

	//case of satellites of planets
	if (observatory->getHomeBody()->isSatellite())
		setLandscape("moon");

	//special case Earth
	if (observatory->isEarth())
		setLandscape(initialvalue.initial_landscapeName);

	currentBodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams()); //, observatory->getSpacecraft());
	std::cout << "Body : " << observatory->getHomeBody()->getEnglishName() << " Landscape : " << currentLandscape->getName() << std::endl;
}

bool Core::setLandscape(const std::string& new_landscape_name)
{
	if (new_landscape_name.empty()) return 0;
	std::string l_min = currentLandscape->getName();
	transform(l_min.begin(), l_min.end(), l_min.begin(), ::tolower);
	if (new_landscape_name == l_min) return 0;

	std::unique_ptr<Landscape> newLandscape = Landscape::createFromFile(AppSettings::Instance()->getUserDir() + "landscapes.ini", new_landscape_name);
	if (!newLandscape) return 0;

	if (currentLandscape) {
		// Copy parameters from previous landscape to new one
		newLandscape->setFlagShow(currentLandscape->getFlagShow());
		newLandscape->fogSetFlagShow(currentLandscape->fogGetFlagShow());
		currentLandscape.set(std::move(newLandscape));
	}
	testLandscapeCompatibleWithAutoMode();
	return 1;
}

void Core::testLandscapeCompatibleWithAutoMode()
{
	//std::cout << "testLandscape :" << std::endl;
	if (currentLandscape->getName().empty())	return;
	if (!observatory->isOnBody())				return;

	// by default the user is not trusted
	autoLandscapeMode = false;

	// a satellite must have Moon as its base landscape
	if (observatory->getHomeBody()->isSatellite() && currentLandscape->getName() == "moon") {
		autoLandscapeMode = true;
		//std::cout << ": automode moon" << std::endl;
		return;
	}

	// case of the sun
	if (observatory->isSun() &&  currentLandscape->getName() == "sun") {
		autoLandscapeMode = true;
		//std::cout << ": automode sun" << std::endl;
		return;
	}

	//case of planets except Earth
	if (!observatory->isEarth() && !observatory->getHomeBody()->isSatellite() && currentLandscape->getName() == observatory->getHomeBody()->getEnglishName()) {
		autoLandscapeMode = true;
		//std::cout << ": automode planet" << std::endl;
		return;
	}

	//special case Earth
	if (observatory->isEarth() && currentLandscape->getName() == initialvalue.initial_landscapeName) {
		//std::cout << ": automode earth" << std::endl;
		autoLandscapeMode = true;
		return;
	}
}

void Core::setLandingLandscape(bool landing, float speed)
{
	if (currentLandscape->getFormat() == "spherical")
		currentLandscape->setLanding(landing, speed);
}

//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
//! and make it the current landscape
bool Core::loadLandscape(stringHash_t& param, int landing)
{

	std::unique_ptr<Landscape> newLandscape = Landscape::createFromHash(param, landing);
	if (!newLandscape) return 0;

	if (currentLandscape) {
		// Copy parameters from previous landscape to new one
		newLandscape->setFlagShow(currentLandscape->getFlagShow());
		newLandscape->fogSetFlagShow(currentLandscape->fogGetFlagShow());
		currentLandscape.set(std::move(newLandscape));
	}
	//std::cout << "Core::loadLandscape(stringHash_t& param)" << std::endl;
	autoLandscapeMode = false;
	return 1;
}

//! Load a solar system body based on a hash of parameters mirroring the ssystem.ini file
void Core::addSolarSystemBody(stringHash_t& param)
{
	currentSsystemFactory->addBody(param);
}

void Core::preloadSolarSystemBody(stringHash_t& param)
{
	auto &name = param["name"];
	if (name == "selected") {
		name = getSelectedPlanetEnglishName();
	} else if (name == "home_planet")
		name = getHomePlanetEnglishName();
	currentSsystemFactory->preloadBody(param);
}

void Core::removeSolarSystemBody(const std::string& name)
{
	// Make sure this object is not already selected so won't crash
	if (selected_object.getType()==OBJECT_BODY && selected_object.getEnglishName() == name) {
		unSelect();
	}
	// Make sure not standing on this object!
	std::shared_ptr<Body> p = observatory->getHomeBody();
	if (p!= nullptr && p->getEnglishName() == name) {
		cLog::get()->write("Can not delete current home planet " + name);
		return;
	}
	currentSsystemFactory->removeBody(name);
}

void Core::removeSupplementalSolarSystemBodies()
{
	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;
	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.getType()==OBJECT_BODY /*&& selected_object.isDeleteable() */) {
		unSelect();
	}
	currentSsystemFactory->removeSupplementalBodies(observatory->getHomePlanetEnglishName());
}


//! get selected object name if it's a planet only for setting home planet to selection with keystroke
std::string Core::getSelectedPlanetEnglishName() const
{
	// Make sure this object is a planet
	if (selected_object.getType()==OBJECT_BODY)
		return selected_object.getEnglishName();
	return ""; // not a planet
}

std::string Core::getHomePlanetEnglishName() const
{
	return observatory->getHomePlanetEnglishName();
}

Object Core::searchByNameI18n(const std::string &name) const
{
	Object rval;
	rval = currentSsystemFactory->searchByNamesI18(name);
	if (rval) return rval;
	rval = currentNebulas->searchByNameI18n(name);
	if (rval) return rval;
	rval = currentHipStars->searchByNameI18n(name).get();
	if (rval) return rval;
	rval = currentAsterisms->searchByNameI18n(name);
	return rval;
}

//! Find and select an object from its translated name
//! @param nameI18n the case sensitive object translated name
//! @return true if an object was found with the passed name
bool Core::findAndSelectI18n(const std::string &nameI18n)
{
	// Then look for another object
	Object obj = searchByNameI18n(nameI18n);
	if (!obj) return false;
	else return selectObject(obj);
}


//! Find and select an object based on selection type and standard name or number
//! @return true if an object was selected
bool Core::selectObject(const std::string &type, const std::string &id)
{
	if (type=="hp") {
		unsigned int hpnum;
		std::istringstream istr(id);
		istr >> hpnum;
		selectObject(currentHipStars->searchHP(hpnum).get());
		// currentAsterisms->setSelected(selected_object);
		// currentHipStars->setSelected(selected_object);
		// currentSsystemFactory->setSelected(""); //setPlanetsSelected("");

	} else if (type=="star") {
		selectObject(currentHipStars->search(id).get());
		// currentAsterisms->setSelected(selected_object);
		// currentHipStars->setSelected(selected_object);
		// currentSsystemFactory->setSelected(""); //setPlanetsSelected("");

	} else if (type=="planet") {
		selectObject(currentSsystemFactory->searchByEnglishName(id).get());

	} else if (type=="nebula") {
		selectObject(currentNebulas->search(id));
		// currentSsystemFactory->setSelected(""); //setPlanetsSelected("");
		// currentAsterisms->setSelected(Object());

	} else if (type=="constellation") {

		// Select only constellation, nothing else
		currentAsterisms->setSelected(id);

		selected_object = nullptr;
		currentSsystemFactory->setSelected(""); //setPlanetsSelected("");

	} else if (type=="constellation_star") {
		// For Find capability, select a star in constellation so can center view on constellation
		currentAsterisms->setSelected(id);
		selectObject(currentAsterisms->getSelected().getBrightestStarInConstellation().get());
		// what is this?
		// 1) Find the hp-number of the 1st star in the selected constellation,
		// 2) find the star of this hpnumber
		// 3) select the constellation of this star ???
		//		const unsigned int hpnum = currentAsterisms->getFirstSelectedHP();
		//		selected_object = currentHipStars->searchHP(hpnum);
		//		currentAsterisms->setSelected(selected_object);
		// currentSsystemFactory->setSelected(""); //setPlanetsSelected("");
		//		// Some stars are shared, so now force constellation
		//		currentAsterisms->setSelected(id);
	} else {
		std::cerr << "Invalid selection type specified: " << type << std::endl;
		std::cout << "Invalid selection type specified: " << type << std::endl;
		unSelect();
		return 0;
	}

	if (selected_object) {
		if (navigation->getFlagTraking())
			navigation->setFlagLockEquPos(1);

		navigation->setFlagTraking(0);
		return 1;
	}

	return 0;
}

void Core::setBodyDecor(bool fromCoreInit)
{
	if (!observatory->isOnBody())
		currentBodyDecor->anchorAssign();
	else
		currentBodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams());
}

void Core::selectZodiac()
{
	currentAsterisms->deselect();
	currentAsterisms->setSelected("Ari");
	currentAsterisms->setSelected("Tau");
	currentAsterisms->setSelected("Gem");
	currentAsterisms->setSelected("Cnc");
	currentAsterisms->setSelected("Leo");
	currentAsterisms->setSelected("Vir");
	currentAsterisms->setSelected("Sco");
	currentAsterisms->setSelected("Sgr");
	currentAsterisms->setSelected("Cap");
	currentAsterisms->setSelected("Aqr");
	currentAsterisms->setSelected("Psc");
	currentAsterisms->setSelected("Lib");
	selected_object = nullptr;
	currentSsystemFactory->setSelected(""); //setPlanetsSelected("");
}

//! Find and select an object near given equatorial position
bool Core::findAndSelect(const Vec3d& pos)
{
	Object tempselect = cleverFind(pos);
	return selectObject(tempselect);
}

//! Find and select an object near given screen position
bool Core::findAndSelect(int x, int y)
{
	Vec3d v;
	projection->unprojectEarthEqu(x, y, v);
	return findAndSelect(v);
}

//! Deselect all selected objects if any
//! Does deselect selected constellations
void Core::deselect(void)
{
	unSelect();
	currentAsterisms->deselect();
	currentHipStars->deselect();
	currentNebulas->deselect();
}

void Core::unsetSelectedConstellation(std::string constellation) {
	currentAsterisms->unsetSelected(constellation);
}


bool Core::getStarEarthEquPosition(int HP, double &az, double &alt) {
	Object star = currentHipStars->searchHP(HP).get();
	if (star) {
		Vec3d earthEqu = star.getEarthEquPos(navigation);
		Utility::rectToSphe(&az, &alt, earthEqu);
		return true;
	}
	return false;
}

// - allow selection of large nearby planets more easily and do not select hidden planets
//! Find an object in a "clever" way
Object Core::cleverFind(const Vec3d& v) const
{
	Object sobj;
	Object default_object;
	bool is_default_object = false;

	std::vector<Object> candidates;
	std::vector<Object> temp;
	Vec3d winpos;

	// Field of view for a 30 pixel diameter circle on screen
	float fov_around = projection->getFov()/std::min(projection->getViewportWidth(), projection->getViewportHeight()) * 30.f;

	float xpos, ypos;
	projection->projectEarthEqu(v, winpos);
	xpos = winpos[0];
	ypos = winpos[1];

	// Collect the planets inside the range
	if (currentSsystemFactory->getFlagShow() && (currentModule == MODULE::SOLAR_SYSTEM || currentModule == MODULE::STELLAR_SYSTEM)) {
		temp = currentSsystemFactory->searchAround(v, fov_around, navigation, observatory.get(), projection, &is_default_object, currentBodyDecor->canDrawBody()); //aboveHomePlanet);
		candidates.insert(candidates.begin(), temp.begin(), temp.end());

		if (is_default_object && temp.begin() != temp.end()) {
			std::vector<Object>::iterator iter = temp.end();
			iter--;
			default_object = (*iter);
		} else {
			// should never get here
			is_default_object = false;
		}
	}

	// nebulas and stars used precessed equ coords
	Vec3d p = navigation->earthEquToJ2000(v);

	// The nebulas inside the range
	if (currentNebulas->getFlagShow() && (currentModule == MODULE::SOLAR_SYSTEM || currentModule == MODULE::STELLAR_SYSTEM)) {
		temp = currentNebulas->searchAround(p, fov_around);
		candidates.insert(candidates.begin(), temp.begin(), temp.end());
	}

	// And the stars inside the range
	if (currentHipStars->getFlagShow() && currentModule == MODULE::SOLAR_SYSTEM) {
		std::vector<ObjectBaseP > tmp = currentHipStars->searchAround(p, fov_around, geodesic_grid);
		for( std::vector<ObjectBaseP >::const_iterator itr = tmp.begin(); itr != tmp.end(); ++itr ) {
			candidates.push_back( Object(itr->get()) );
		}
	}
	if (currentStarNav->getFlagStars() && (currentModule == MODULE::IN_GALAXY || currentModule == MODULE::STELLAR_SYSTEM)) {
		std::vector<ObjectBaseP > tmp = currentStarNav->searchAround(v, fov_around, navigation);
		for( std::vector<ObjectBaseP >::const_iterator itr = tmp.begin(); itr != tmp.end(); ++itr ) {
			candidates.push_back( Object(itr->get()) );
		}
	}

	if (currentTully->getFlagShow() && currentModule == MODULE::IN_UNIVERSE) {
		std::vector<ObjectBaseP > tmp = currentTully->searchAround(v, fov_around, navigation);
		for( std::vector<ObjectBaseP >::const_iterator itr = tmp.begin(); itr != tmp.end(); ++itr ) {
			candidates.push_back( Object(itr->get()) );
		}
	}

	// Now select the object minimizing the function y = distance(in pixel) + magnitude
	float best_object_value;
	float best_object_distance = 10000000.f;
	best_object_value = 100000.f;
	std::vector<Object>::iterator iter = candidates.begin();
	while (iter != candidates.end()) {
		projection->projectEarthEqu((*iter).getEarthEquPos(navigation), winpos);

		float distance = sqrt((xpos-winpos[0])*(xpos-winpos[0]) + (ypos-winpos[1])*(ypos-winpos[1]));
		float mag = (*iter).getMag(navigation);

		if ((*iter).getType()==OBJECT_NEBULA) {
			if ( currentNebulas->getFlagHints() ) {
				// make very easy to select IF LABELED
				mag = -1;

			}
		}
		if ((*iter).getType()==OBJECT_BODY) {
			if ( currentSsystemFactory->getFlag(BODY_FLAG::F_HINTS)) {
				// easy to select, especially pluto
				mag -= 15.f;
			} else {
				mag -= 8.f;
			}
		}
		if ((*iter).getType()==OBJECT_STAR_CLUSTER) {
			if (distance < best_object_distance) {
				best_object_distance = distance;
				sobj = *iter;
			}
		}
		else if (distance + mag < best_object_value) {
			best_object_value = distance + mag;
			best_object_distance = distance;
			sobj = *iter;
		}
		iter++;
	}

	// when large planet disk is hiding anything else
	if (is_default_object && sobj.getType()!=OBJECT_BODY)
		return default_object;

	return sobj;
}

Object Core::cleverFind(int x, int y) const
{
	Vec3d v;
	projection->unprojectEarthEqu(x,y,v);
	return cleverFind(v);
}

//! Go and zoom to the selected object.
void Core::autoZoomIn(float move_duration, bool allow_manual_zoom)
{
	float manual_move_duration;

	if (!selected_object) return;

	if (!navigation->getFlagTraking()) {
		navigation->setFlagTraking(true);
		navigation->moveTo(selected_object.getEarthEquPos(navigation), move_duration, false, 1);
		manual_move_duration = move_duration;
	} else {
		// faster zoom in manual zoom mode once object is centered
		manual_move_duration = move_duration*.66f;
	}

	if ( allow_manual_zoom && FlagManualZoom ) {
		// if manual zoom mode, user can zoom in incrementally
		float newfov = projection->getFov()*0.5f;
		projection->zoomTo(newfov, manual_move_duration);

	} else {
		float satfov = selected_object.getSatellitesFov(navigation);

		if (satfov>0.0 && projection->getFov()*0.9>satfov)
			projection->zoomTo(satfov, move_duration);
		else {
			float closefov = selected_object.getCloseFov(navigation);
			if (projection->getFov()>closefov)
				projection->zoomTo(closefov, move_duration);
		}
	}
}


//! Unzoom and go to the init position
void Core::autoZoomOut(float move_duration, bool full, bool allow_manual_zoom)
{
	if (selected_object && !full) {
		// Handle manual unzoom
		if ( allow_manual_zoom && FlagManualZoom ) {
			// if manual zoom mode, user can zoom out incrementally
			float newfov = projection->getFov()*2.f;
			if (newfov >= InitFov ) {
				// Need to go to init fov/direction
				projection->zoomTo(InitFov, move_duration);
				navigation->moveTo(InitViewPos, move_duration, true, -1);
				navigation->setFlagTraking(false);
				navigation->setFlagLockEquPos(0);
				return;
			} else {
				// faster zoom in manual zoom with object centered
				float manual_move_duration = move_duration*.66f;
				projection->zoomTo(newfov, manual_move_duration);
				return;
			}
		}

		// If the selected object has satellites, unzoom to satellites view unless specified otherwise
		float satfov = selected_object.getSatellitesFov(navigation);

		// Saturn wasn't untracking from moon issue
		if (satfov>0.0 && projection->getFov()<=satfov*0.9 && satfov < .9*InitFov) {
			projection->zoomTo(satfov, move_duration);
			return;
		}

		// If the selected object is part of a Planet subsystem (other than sun), unzoom to subsystem view
		satfov = selected_object.getParentSatellitesFov(navigation);
		//    cout << "Unzoom to parent sat fov: " << satfov << endl;

		// Charon wasn't untracking from Pluto issue
		if (satfov>0.0 && projection->getFov()<=satfov*0.9 && satfov < .9*InitFov) {
			projection->zoomTo(satfov, move_duration);
			return;
		}
	}
	//  cout << "Unzoom to initfov\n";
	projection->zoomTo(InitFov, move_duration);
	navigation->moveTo(InitViewPos, move_duration, true, -1);
	navigation->setFlagTraking(false);
	navigation->setFlagLockEquPos(0);
}

//! Set the current sky culture according to passed name
bool Core::setSkyCulture(const std::string& cultureName)
{
	return setSkyCultureDir(skyloc->skyCultureToDirectory(cultureName));
}

//! Set the current sky culture from the passed directory
bool Core::setSkyCultureDir(const std::string& cultureDir, bool fromCoreInit)
{
	if (skyCultureDir == cultureDir) return 1;
	// make sure culture definition exists before attempting or will die
	// Do not comment this out! Rob
	if (skyloc->directoryToSkyCultureEnglish(cultureDir) == "") {
		std::cerr << "Invalid sky culture directory: " << cultureDir << std::endl;
		return 0;
	}
	skyCultureDir = cultureDir;

	if (fromCoreInit) {
		if (!currentAsterisms.get(CURRENT_MODE::NORMAL_MODE) || !currentAsterisms.get(CURRENT_MODE::SANDBOX_MODE)) {
			// objects not initialized yet
			return 0;
		}

		// Only load lines and art for normal mode at init
		currentAsterisms.applyTo(CURRENT_MODE::NORMAL_MODE, &ConstellationMgr::loadLinesAndArt, AppSettings::Instance()->getSkyCultureDir() + cultureDir);
		currentAsterisms.applyToAll([this, &cultureDir](ConstellationMgr &mgr) {
			mgr.loadNames(AppSettings::Instance()->getSkyCultureDir() + cultureDir + "/constellation_names.eng.fab");

			// Re-translated constellation names
			mgr.translateNames(skyTranslator);
		});

		currentHipStars.applyToAll([this, &cultureDir](HipStarMgr &mgr) {
			// Load culture star names in english
			mgr.loadCommonNames(AppSettings::Instance()->getSkyCultureDir() + cultureDir + "/star_names.fab");

			// Turn on sci names for western culture only
			mgr.setFlagSciNames(cultureDir.compare(0, 7, "western") == 0);

			// translate
			mgr.updateI18n(skyTranslator);
		});
		currentStarNav.applyToAll([this, &cultureDir](StarNavigator &mgr) {
			// Load culture star names in english
			mgr.loadCommonNames(AppSettings::Instance()->getSkyCultureDir() + cultureDir + "/star_names.fab");

			// translate
			mgr.updateI18n(skyTranslator);
		});

		return 1;
	}

	if (!currentAsterisms) return 0;

	currentAsterisms->loadLinesAndArt(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir);
	currentAsterisms->loadNames(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir + "/constellation_names.eng.fab");
	// Re-translated constellation names
	currentAsterisms->translateNames(skyTranslator);

	// as constellations have changed, clear out any selection and retest for match!
	if (selected_object && selected_object.getType()==OBJECT_STAR) {
		currentAsterisms->setSelected(selected_object);
	}
	// else {
		// currentAsterisms->setSelected(Object());
	// }

	// Load culture star names in english
	currentHipStars->loadCommonNames(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir + "/star_names.fab");
	currentStarNav->loadCommonNames(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir + "/star_names.fab");
	// Turn on sci names for western culture only
	currentHipStars->setFlagSciNames( skyCultureDir.compare(0, 7, "western") ==0 );

	// translate
	currentHipStars->updateI18n(skyTranslator);
	currentStarNav->updateI18n(skyTranslator);

	return 1;
}


//! For loading custom sky cultures from scripts, use any path
//! Set the current sky culture from the arbitrary path
bool Core::loadSkyCulture(const std::string& culturePath)
{
	// TODO: how to deal with culture hash and current value
	skyCultureDir = "Custom";  // This allows reloading defaults correctly
	if (!currentAsterisms) return 0;

	currentAsterisms->loadLinesAndArt(culturePath);
	currentAsterisms->loadNames(culturePath + "/constellation_names.eng.fab");

	// Re-translated constellation names
	currentAsterisms->translateNames(skyTranslator);

	// as constellations have changed, clear out any selection and retest for match!
	if (selected_object && selected_object.getType()==/*ObjectRecord::*/OBJECT_STAR) {
		currentAsterisms->setSelected(selected_object);
	} else {
		currentAsterisms->setSelected(Object());
	}

	// Load culture star names in english
	currentHipStars->loadCommonNames(culturePath + "/star_names.fab");
	currentStarNav->loadCommonNames(culturePath + "/star_names.fab");

	// translate
	currentHipStars->updateI18n(skyTranslator);
	currentStarNav->updateI18n(skyTranslator);

	return 1;
}



//! @brief Set the sky locale and reload the sky objects names for gettext translation
void Core::setSkyLanguage(const std::string& newSkyLocaleName, bool fromCoreInit)
{
	if ( !currentHipStars || !cardinals_points || !currentAsterisms || ! currentSkyLineMgr->isExist(SKYLINE_TYPE::LINE_ECLIPTIC)) return; // objects not initialized yet

	std::string oldLocale = getSkyLanguage();
	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	// Update the translator with new locale name
	skyTranslator = Translator(AppSettings::Instance()->getLanguageDir(), newSkyLocaleName);
	cLog::get()->write("Sky locale is " + skyTranslator.getLocaleName(), LOG_TYPE::L_INFO);
	//printf("SkyLocale : %s\n", newSkyLocaleName.c_str());
	std::string language = skyTranslator.getLocaleName();
	if (language[0] == 'z' && language[1] == 'h')
		fontFactory->updateAllFont("/home/planetarium/.spacecrafter/fonts/HanWangHeiHeavy.ttf");
	else if (language[0] == 'j' && language[1] == 'a')
		fontFactory->updateAllFont("/home/planetarium/.spacecrafter/fonts/PretendardJPVariable.ttf");
	else {
		fontFactory->updateFont("text", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_TEXT_NAME), conf.getStr(SCS_FONT, SCK_FONT_TEXT_SIZE));
		fontFactory->updateFont("menu", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_MENU_NAME), conf.getStr(SCS_FONT, SCK_FONT_MENUTUI_SIZE));
		fontFactory->updateFont("planets", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_PLANET_NAME), conf.getStr(SCS_FONT, SCK_FONT_PLANET_SIZE));
		fontFactory->updateFont("constellations", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_CONSTELLATION_NAME), conf.getStr(SCS_FONT, SCK_FONT_CONSTELLATION_SIZE));
		fontFactory->updateFont("cardinal_points", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_CARDINALPOINTS_NAME), conf.getStr(SCS_FONT, SCK_FONT_CARDINALPOINTS_SIZE));
		fontFactory->updateFont("grids", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_GRID_NAME), conf.getStr(SCS_FONT, SCK_FONT_GRID_SIZE));
		fontFactory->updateFont("lines", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_LINES_NAME), conf.getStr(SCS_FONT, SCK_FONT_LINE_SIZE));
		fontFactory->updateFont("displays", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_DISPLAY_NAME), conf.getStr(SCS_FONT, SCK_FONT_DISPLAY_SIZE));
		fontFactory->updateFont("stars", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_HIPSTARS_NAME), conf.getStr(SCS_FONT, SCK_FONT_HIPSTARS_SIZE));
		fontFactory->updateFont("nebulae", AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_NEBULAS_NAME), conf.getStr(SCS_FONT, SCK_FONT_NEBULAS_SIZE));
	}

	// Translate all labels with the new language
	cardinals_points->translateLabels(skyTranslator);
	if (fromCoreInit) {
		currentSkyLineMgr.applyToAll(&SkyLineMgr::translateLabels, skyTranslator); //ecliptic_line
		currentAsterisms.applyToAll(&ConstellationMgr::translateNames, skyTranslator);
		currentSsystemFactory.applyToAll(&SSystemFactory::translateNames, skyTranslator);
		currentNebulas.applyToAll(&NebulaMgr::translateNames, skyTranslator);
		currentHipStars.applyToAll(&HipStarMgr::updateI18n, skyTranslator);
		currentStarNav.applyToAll(&StarNavigator::updateI18n, skyTranslator);
	} else {
		currentSkyLineMgr->translateLabels(skyTranslator); //ecliptic_line
		currentAsterisms->translateNames(skyTranslator);
		currentSsystemFactory->translateNames(skyTranslator);
		currentNebulas->translateNames(skyTranslator);
		currentHipStars->updateI18n(skyTranslator);
		currentStarNav->updateI18n(skyTranslator);
	}
	setLanguage();
}


//! Please keep saveCurrentAppSettings::Instance() up to date with any new color AppSettings::Instance() added here
void Core::setColorScheme(const std::string& skinFile, const std::string& section)
{
	InitParser conf;
	conf.load(skinFile);
	// simple default color, rather than black which doesn't show up
	// Load colors from config file
	currentSkyGridMgr.applyToAll([&conf, &section](SkyGridMgr &mgr) {
		mgr.setColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL, Utility::strToVec3f(conf.getStr(section,SCK_AZIMUTHAL_COLOR)));
		mgr.setColor(SKYGRID_TYPE::GRID_EQUATORIAL, Utility::strToVec3f(conf.getStr(section,SCK_EQUATORIAL_COLOR)));
		mgr.setColor(SKYGRID_TYPE::GRID_ECLIPTIC, Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_COLOR)));
		mgr.setColor(SKYGRID_TYPE::GRID_GALACTIC, Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_COLOR)));
	});

	currentSkyLineMgr.applyToAll([&conf, &section](SkyLineMgr &mgr) {
		mgr.setColor(SKYLINE_TYPE::LINE_ECLIPTIC, Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));
	});

	currentNebulas.applyToAll([&conf, &section](NebulaMgr &mgr) {
		mgr.setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_LABEL_COLOR)));
		mgr.setCircleColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_CIRCLE_COLOR)));
	});

	currentDso3d.applyToAll([&conf, &section](Dso3d &mgr) {
		mgr.setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_LABEL_COLOR)));
	});

	currentSkyLineMgr.applyToAll([&conf, &section](SkyLineMgr &mgr) {
		mgr.setColor(SKYLINE_TYPE::LINE_PRECESSION, Utility::strToVec3f(conf.getStr(section,SCK_PRECESSION_CIRCLE_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_CIRCUMPOLAR, Utility::strToVec3f(conf.getStr(section,SCK_CIRCUMPOLAR_CIRCLE_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_VERNAL, Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_TROPIC, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));
	});

	currentSsystemFactory.applyToAll([&conf, &section](SSystemFactory &mgr) {
		mgr.setDefaultBodyColor(conf.getStr(section,SCK_PLANET_NAMES_COLOR), conf.getStr(section,SCK_PLANET_NAMES_COLOR),
								conf.getStr(section,SCK_PLANET_ORBITS_COLOR), conf.getStr(section,SCK_OBJECT_TRAILS_COLOR));
	});

	// default color override
	currentStarLines.applyToAll([&conf, &section](StarLines &mgr) {
		mgr.setColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_LINES3D_COLOR)));
	});

	currentAsterisms.applyToAll([&conf, &section](ConstellationMgr &mgr) {
		mgr.setLineColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_LINES_COLOR)));
		mgr.setBoundaryColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
		mgr.setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
		mgr.setArtColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));
	});

	cardinals_points->setColor(Utility::strToVec3f(conf.getStr(section,SCK_CARDINAL_COLOR)));

	currentSkyLineMgr.applyToAll([&conf, &section](SkyLineMgr &mgr) {
		mgr.setColor(SKYLINE_TYPE::LINE_ANALEMMALINE, Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_ANALEMMA, Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_ARIES,Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_GREENWICH,Utility::strToVec3f(conf.getStr(section,SCK_GREENWICH_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_MERIDIAN,Utility::strToVec3f(conf.getStr(section,SCK_MERIDIAN_COLOR)));
	});

	currentSkyDisplayMgr.applyToAll([&conf, &section](SkyDisplayMgr &mgr) {
		mgr.setColor(SKYDISPLAY_NAME::SKY_PERSONAL,Utility::strToVec3f(conf.getStr(section,SCK_PERSONAL_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_PERSONEQ,Utility::strToVec3f(conf.getStr(section,SCK_PERSONEQ_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_NAUTICAL,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_ALT_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_NAUTICEQ,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_RA_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_OBJCOORDS,Utility::strToVec3f(conf.getStr(section,SCK_OBJECT_COORDINATES_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_MOUSECOORDS,Utility::strToVec3f(conf.getStr(section,SCK_MOUSE_COORDINATES_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_ANGDIST,Utility::strToVec3f(conf.getStr(section,SCK_ANGULAR_DISTANCE_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_LOXODROMY,Utility::strToVec3f(conf.getStr(section,SCK_LOXODROMY_COLOR)));
		mgr.setColor(SKYDISPLAY_NAME::SKY_ORTHODROMY,Utility::strToVec3f(conf.getStr(section,SCK_ORTHODROMY_COLOR)));
	});

	media->setTextColor(Utility::strToVec3f(conf.getStr(section,SCK_TEXT_USR_COLOR)));

	currentSkyLineMgr.applyToAll([&conf, &section](SkyLineMgr &mgr) {
		mgr.setColor(SKYLINE_TYPE::LINE_CIRCLE_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_POINT_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_VERNAL,Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_VERTICAL,Utility::strToVec3f(conf.getStr(section,SCK_VERTICAL_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_ZENITH,Utility::strToVec3f(conf.getStr(section,SCK_ZENITH_COLOR)));
		mgr.setColor(SKYLINE_TYPE::LINE_ZODIAC,Utility::strToVec3f(conf.getStr(section,SCK_ZODIAC_COLOR)));
	});

	currentOort.applyToAll([&conf, &section](Oort &mgr) {
		mgr.setColor(Utility::strToVec3f(conf.getStr(section,SCK_OORT_COLOR)));
	});
}

//! For use by TUI - saves all current AppSettings::Instance()
void Core::saveCurrentConfig(InitParser &conf)
{
	// localization section
	conf.setStr(SCS_LOCALIZATION, SCK_SKY_CULTURE, getSkyCultureDir());
	conf.setStr(SCS_LOCALIZATION, SCK_SKY_LOCALE, getSkyLanguage());
	// viewing section
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_DRAWING, currentAsterisms->getFlagLines());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_NAME, currentAsterisms->getFlagNames());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_ART, currentAsterisms->getFlagArt());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_BOUNDARIES, currentAsterisms->getFlagBoundaries());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_PICK, currentAsterisms->getFlagIsolateSelected());
	conf.setDouble(SCS_VIEWING, SCK_MOON_SCALE, currentSsystemFactory->getMoonScale());
	conf.setDouble(SCS_VIEWING, SCK_SUN_SCALE, currentSsystemFactory->getSunScale());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_EQUATORIAL_GRID, currentSkyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ECLIPTIC_GRID, currentSkyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GALACTIC_GRID, currentSkyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_GALACTIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_AZIMUTAL_GRID, currentSkyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_EQUATOR_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_EQUATOR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ECLIPTIC_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CARDINAL_POINTS, cardinals_points->getFlagShow());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ZENITH_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ZENITH));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_POLAR_CIRCLE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_POLAR_POINT, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ECLIPTIC_CENTER, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GALACTIC_POLE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GALACTIC_CENTER, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_VERNAL_POINTS, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_VERNAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ANALEMMA, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ANALEMMA));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ANALEMMA_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ARIES_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ARIES));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ZODIAC, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ZODIAC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GREENWICH_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_GREENWICH));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_VERTICAL_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_VERTICAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_MERIDIAN_LINE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_MERIDIAN));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_PRECESSION_CIRCLE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_PRECESSION));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CIRCUMPOLAR_CIRCLE, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_TROPIC_LINES, currentSkyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_TROPIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_MOON_SCALED, currentSsystemFactory->getFlagMoonScale());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_SUN_SCALED, currentSsystemFactory->getFlagSunScale());
	conf.setDouble (SCS_VIEWING, SCK_CONSTELLATION_ART_INTENSITY, currentAsterisms->getArtIntensity());
	conf.setDouble (SCS_VIEWING, SCK_CONSTELLATION_ART_FADE_DURATION, currentAsterisms->getArtFadeDuration());
	conf.setDouble(SCS_VIEWING, SCK_LIGHT_POLLUTION_LIMITING_MAGNITUDE, getLightPollutionLimitingMagnitude());
	// Landscape section
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_LANDSCAPE, currentLandscape->getFlagShow());
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_ATMOSPHERE, currentBodyDecor->getAtmosphereState());
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_FOG, currentLandscape->fogGetFlagShow());
	// Star section
	conf.setDouble (SCS_STARS , SCK_STAR_SCALE, currentHipStars->getScale());
	conf.setDouble (SCS_STARS , SCK_STAR_MAG_SCALE, currentHipStars->getMagScale());
	conf.setDouble(SCS_STARS , SCK_MAX_MAG_STAR_NAME, currentHipStars->getMaxMagName());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_STAR_PICK, currentHipStars->getFlagIsolateSelected());
	conf.setBoolean(SCS_STARS , SCK_FLAG_STAR_TWINKLE, currentHipStars->getFlagTwinkle());
	conf.setDouble(SCS_STARS , SCK_STAR_TWINKLE_AMOUNT, currentHipStars->getTwinkleAmount());
	conf.setDouble(SCS_STARS , SCK_STAR_LIMITING_MAG, currentHipStars->getMagConverterMaxScaled60DegMag());
	// Color section
	conf.setStr    (SCS_COLOR, SCK_AZIMUTHAL_COLOR, Utility::vec3fToStr(currentSkyGridMgr->getColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL)));
	conf.setStr    (SCS_COLOR, SCK_EQUATORIAL_COLOR, Utility::vec3fToStr(currentSkyGridMgr->getColor(SKYGRID_TYPE::GRID_EQUATORIAL)));
	conf.setStr    (SCS_COLOR, SCK_ECLIPTIC_COLOR, Utility::vec3fToStr(currentSkyGridMgr->getColor(SKYGRID_TYPE::GRID_ECLIPTIC)));
	conf.setStr    (SCS_COLOR, SCK_EQUATOR_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_EQUATOR)));
	conf.setStr    (SCS_COLOR, SCK_ECLIPTIC_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_ECLIPTIC)));
	conf.setStr    (SCS_COLOR, SCK_MERIDIAN_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_MERIDIAN)));
	conf.setStr    (SCS_COLOR, SCK_ZENITH_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_ZENITH)));
	conf.setStr    (SCS_COLOR, SCK_POLAR_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_CIRCLE_POLAR)));
	conf.setStr    (SCS_COLOR, SCK_POLAR_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_POINT_POLAR)));
	conf.setStr    (SCS_COLOR, SCK_ECLIPTIC_CENTER_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE)));
	conf.setStr    (SCS_COLOR, SCK_GALACTIC_POLE_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_GALACTIC_POLE)));
	conf.setStr    (SCS_COLOR, SCK_GALACTIC_CENTER_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER)));
	conf.setStr    (SCS_COLOR, SCK_VERNAL_POINTS_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_VERNAL)));
	conf.setStr    (SCS_COLOR, SCK_ANALEMMA_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_ANALEMMA)));
	conf.setStr    (SCS_COLOR, SCK_ANALEMMA_LINE_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_ANALEMMALINE)));
	conf.setStr    (SCS_COLOR, SCK_ARIES_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_ARIES)));
	conf.setStr    (SCS_COLOR, SCK_ZODIAC_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_ZODIAC)));
	conf.setStr    (SCS_COLOR, SCK_PERSONAL_COLOR,     Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_PERSONAL)));
	conf.setStr    (SCS_COLOR, SCK_PERSONEQ_COLOR,     Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_PERSONEQ)));
	conf.setStr    (SCS_COLOR, SCK_NAUTICAL_ALT,       Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_NAUTICAL)));
	conf.setStr    (SCS_COLOR, SCK_NAUTICAL_RA,        Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_NAUTICEQ)));
	conf.setStr    (SCS_COLOR, SCK_OBJECT_COORDINATES, Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_OBJCOORDS)));
	conf.setStr    (SCS_COLOR, SCK_MOUSE_COORDINATES,  Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_MOUSECOORDS)));
	conf.setStr    (SCS_COLOR, SCK_ANGULAR_DISTANCE,   Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_ANGDIST)));
	conf.setStr    (SCS_COLOR, SCK_LOXODROMY,          Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_LOXODROMY)));
	conf.setStr    (SCS_COLOR, SCK_ORTHODROMY,         Utility::vec3fToStr(currentSkyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_ORTHODROMY)));
	conf.setStr    (SCS_COLOR, SCK_GREENWICH_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_GREENWICH)));
	conf.setStr    (SCS_COLOR, SCK_VERTICAL_LINE, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_VERTICAL)));
	conf.setStr    (SCS_COLOR, SCK_CONST_LINES_COLOR, Utility::vec3fToStr(currentAsterisms->getLineColor()));
	conf.setStr    (SCS_COLOR, SCK_CONST_NAMES_COLOR, Utility::vec3fToStr(currentAsterisms->getLabelColor()));
	conf.setStr    (SCS_COLOR, SCK_CONST_ART_COLOR, Utility::vec3fToStr(currentAsterisms->getArtColor()));
	conf.setStr    (SCS_COLOR, SCK_CONST_BOUNDARY_COLOR, Utility::vec3fToStr(currentAsterisms->getBoundaryColor()));
	conf.setStr	   (SCS_COLOR, SCK_NEBULA_LABEL_COLOR, Utility::vec3fToStr(currentNebulas->getLabelColor()));
	conf.setStr	   (SCS_COLOR, SCK_NEBULA_CIRCLE_COLOR, Utility::vec3fToStr(currentNebulas->getCircleColor()));
	conf.setStr	   (SCS_COLOR, SCK_PRECESSION_CIRCLE_COLOR, Utility::vec3fToStr(currentSkyLineMgr->getColor(SKYLINE_TYPE::LINE_PRECESSION)));
	conf.setStr    (SCS_COLOR, SCK_CARDINAL_COLOR, Utility::vec3fToStr(cardinals_points->getColor()));
	// Navigation section
	conf.setBoolean(SCS_NAVIGATION, SCK_FLAG_MANUAL_ZOOM, getFlagManualAutoZoom());
	conf.setDouble (SCS_NAVIGATION, SCK_AUTO_MOVE_DURATION, getAutoMoveDuration());
	conf.setDouble (SCS_NAVIGATION, SCK_ZOOM_SPEED, vzm.zoom_speed);
	conf.setDouble (SCS_NAVIGATION, SCK_HEADING, navigation->getHeading());
	// Astro section
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS, currentSsystemFactory->getFlag(BODY_FLAG::F_TRAIL));
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_BRIGHT_NEBULAE, currentNebulas->getFlagBright());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_STARS, currentHipStars->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME, currentHipStars->getFlagNames());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_STAR_PICK, currentHipStars->getFlagIsolateSelected());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA, currentNebulas->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA_NAMES, currentNebulas->getNebulaNames());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA_HINTS, currentNebulas->getFlagHints());
	conf.setDouble(SCS_ASTRO, SCK_MAX_MAG_NEBULA_NAME, currentNebulas->getMaxMagHints());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS, currentSsystemFactory->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_HINTS, currentSsystemFactory->getFlag(BODY_FLAG::F_HINTS));
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_ORBITS, currentSsystemFactory->getFlagPlanetsOrbits());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_LIGHT_TRAVEL_TIME, currentSsystemFactory->getFlagLightTravelTime());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_MILKY_WAY, currentMilkyWay->getFlagShow());
	conf.setDouble(SCS_ASTRO, SCK_MILKY_WAY_INTENSITY, currentMilkyWay->getIntensity());
	conf.setDouble(SCS_ASTRO, SCK_STAR_SIZE_LIMIT, starGetSizeLimit());
	conf.setDouble(SCS_ASTRO, SCK_PLANET_SIZE_MARGINAL_LIMIT, getPlanetsSizeLimit());
	conf.setStr(SCS_INIT_LOCATION , SCK_LANDSCAPE_NAME, currentLandscape->getName() );
	conf.setStr(SCS_INIT_LOCATION , SCK_HOME_PLANET, observatory->getHomeBody()->getEnglishName());
}


//! Get a color used to display info about the currently selected object
// @TODO reafire que l'objet sélectionné renvoie sa propore couleur
Vec3f Core::getSelectedObjectInfoColor(void) const
{
	if (!selected_object) {
		std::cerr << "WARNING: Core::getSelectedObjectInfoColor was called while no object is currently selected!!" << std::endl;
		return Vec3f(1, 1, 1);
	}
	if (selected_object.getType()==OBJECT_NEBULA) return currentNebulas->getLabelColor();
	if (selected_object.getType()==OBJECT_BODY) return currentSsystemFactory->getDefaultBodyColor("label");
	if (selected_object.getType()==OBJECT_STAR) return selected_object.getRGB();
	return Vec3f(1, 1, 1);
}


std::string Core::getCursorPos(int x, int y)
{
	Vec3d v;
	projection->unprojectEarthEqu(x,y,v);
	float tempDE, tempRA;
	Utility::rectToSphe(&tempRA,&tempDE,v);
	return std::string("RA : ") + Utility::printAngleHMS(tempRA) + "\n" +"DE : " + Utility::printAngleDMS(tempDE);
}

Vec3f Core::getCursorPosEqu(int x, int y)
{
	Vec3d v;
	projection->unprojectEarthEqu(x, y,v);
	return v;
}

void Core::turnHorizontal(float s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAz = s;
		vzm.coefAz = abs(s);
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else {
		vzm.deltaAz = 0;
		vzm.coefAz = 1;
	}
}

void Core::turnVertical(float s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAlt = s;
		vzm.coefAlt = abs(s);
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else {
		vzm.deltaAlt = 0;
		vzm.coefAlt = 1;
	}
}

void Core::turnRight(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAz = 1;
		vzm.coefAz = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else if (vzm.deltaAz > 0)
		vzm.deltaAz = 0;
}

void Core::turnLeft(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAz = -1;
		vzm.coefAz = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else if (vzm.deltaAz < 0)
		vzm.deltaAz = 0;
}

void Core::turnUp(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAlt = 1;
		vzm.coefAlt = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else  if (vzm.deltaAlt > 0)
		vzm.deltaAlt = 0;
}

void Core::turnDown(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAlt = -1;
		vzm.coefAlt = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else if (vzm.deltaAlt < 0)
	 	vzm.deltaAlt = 0;
}


void Core::zoomIn(int s)
{
	if (FlagEnableZoomKeys) vzm.deltaFov = -1*(s!=0);
}

void Core::zoomOut(int s)
{
	if (FlagEnableZoomKeys) vzm.deltaFov = (s!=0);
}

void Core::raiseHeight(int s)
{
	vzm.deltaHeight = 1.01*(s!=0);
}

void Core::lowerHeight(int s)
{
	vzm.deltaHeight = 0.99*(s!=0);
}

//! Make the first screen position correspond to the second (useful for mouse dragging)
void Core::dragView(int x1, int y1, int x2, int y2)
{
	Vec3d tempvec1, tempvec2;
	double az1, alt1, az2, alt2;
	if (navigation->getViewingMode()==Navigator::VIEW_HORIZON) {
		projection->unprojectLocal(x2, y2, tempvec2);
		projection->unprojectLocal(x1, y1, tempvec1);
	} else {
		projection->unprojectEarthEqu(x2, y2, tempvec2);
		projection->unprojectEarthEqu(x1, y1, tempvec1);
	}
	Utility::rectToSphe(&az1, &alt1, tempvec1);
	Utility::rectToSphe(&az2, &alt2, tempvec2);
	navigation->updateMove(az2-az1, alt1-alt2, projection->getFov());
	setFlagTracking(false);
	setFlagLockSkyPosition(false);
}

//! Increment/decrement smoothly the vision field and position
void Core::updateMove(int delta_time)
{
	// the more it is zoomed, the more the mooving speed is low (in angle)
	double depl=vzm.move_speed*delta_time*projection->getFov();
	double deplzoom=vzm.zoom_speed*delta_time*projection->getFov();

	if (vzm.deltaAz<0) {
		vzm.deltaAz = -vzm.coefAz*depl/30;
		if (vzm.deltaAz<-0.2) vzm.deltaAz = -0.2;
	} else {
		if (vzm.deltaAz>0) {
			vzm.deltaAz = vzm.coefAz*(depl/30);
			if (vzm.deltaAz>0.2) vzm.deltaAz = 0.2;
		}
	}
	if (vzm.deltaAlt<0) {
		vzm.deltaAlt = -vzm.coefAlt*depl/30;
		if (vzm.deltaAlt<-0.2) vzm.deltaAlt = -0.2;
	} else {
		if (vzm.deltaAlt>0) {
			vzm.deltaAlt = vzm.coefAlt*depl/30;
			if (vzm.deltaAlt>0.2) vzm.deltaAlt = 0.2;
		}
	}

	if (vzm.deltaFov<0) {
		vzm.deltaFov = -deplzoom*5;
		if (vzm.deltaFov<-0.15*projection->getFov()) vzm.deltaFov = -0.15*projection->getFov();
	} else {
		if (vzm.deltaFov>0) {
			vzm.deltaFov = deplzoom*5;
			if (vzm.deltaFov>20) vzm.deltaFov = 20;
		}
	}

	if (vzm.deltaHeight!=0) {
		observatory->multAltitude(vzm.deltaHeight);
	}

	if (vzm.deltaFov != 0 ) {
		projection->changeFov(vzm.deltaFov);
		std::ostringstream oss;
		oss << "zoom delta_fov " << vzm.deltaFov;
		if (!recordActionCallback.empty()) recordActionCallback(oss.str());
	}

	if (vzm.deltaAz != 0 || vzm.deltaAlt != 0) {
		navigation->updateMove(vzm.deltaAz, vzm.deltaAlt, projection->getFov());
		std::ostringstream oss;
		oss << "look delta_az " << vzm.deltaAz << " delta_alt " << vzm.deltaAlt;
		if (!recordActionCallback.empty()) recordActionCallback(oss.str());
	} else {
		// must perform call anyway, but don't record!
		navigation->updateMove(vzm.deltaAz, vzm.deltaAlt, projection->getFov());
	}
}


void Core::setHomePlanet(const std::string &planet)
{
	// reset planet trails due to changed perspective
	currentSsystemFactory->startTrails( currentSsystemFactory->getFlag(BODY_FLAG::F_TRAIL));
	Event* event= new ObserverEvent(planet);
	EventRecorder::getInstance()->queue(event);
	bool result = false;
	if (planet=="selected")
		result =  currentSsystemFactory->switchToAnchor(selected_object);
	else
		result =  currentSsystemFactory->switchToAnchor(planet);
	if (result)
		bindHomePlanet();
}

void Core::bindHomePlanet()
{
	setLandscapeToBody();
}

void Core::setLightPollutionLimitingMagnitude(float mag, bool fromCoreInit) {
	lightPollutionLimitingMagnitude = mag;
	float ln = log(mag);
	float lum = 30.0842967491175 -19.9408790405749*ln +2.12969160094949*ln*ln - .2206;
	currentAtmosphere->setLightPollutionLuminance(lum);
	float pollum = (5.0-mag)*0.1;

	if (fromCoreInit) {
		currentMilkyWay.applyToAll([pollum](MilkyWay &mgr) {
			mgr.setPollum((pollum < 0) ? 0 : pollum);
		});
	} else {
		// Otherwise only set the current milky way
		currentMilkyWay->setPollum((pollum < 0) ? 0 : pollum);
	}

	//int nb = int(mag);
	//if (nb == 0) nb = 1;
	//milky_way->changeMilkywayStateWithoutIntensity(AppSettings::Instance()->getTextureDir() + "milkyway" + std::to_string(nb) + ".png");
}


// For use by TUI
std::string Core::getPlanetHashString()
{
	return currentSsystemFactory->getPlanetHashString();
}

//! Set simulation time to current real world time
void Core::setTimeNow()
{
	timeMgr->setJDay(SpaceDate::JulianFromSys());
}

//! Get wether the current simulation time is the real world time
bool Core::getIsTimeNow(void) const
{
	// cache last time to prevent to much slow system call
	static double lastJD = timeMgr->getJDay();
	static bool previousResult = (fabs(timeMgr->getJDay()-SpaceDate::JulianFromSys())<JD_SECOND);
	if (fabs(lastJD-timeMgr->getJDay())>JD_SECOND/4) {
		lastJD = timeMgr->getJDay();
		previousResult = (fabs(timeMgr->getJDay()-SpaceDate::JulianFromSys())<JD_SECOND);
	}
	return previousResult;
}

//set name as number for language
void Core::setLanguage()
{
	int j = 0;
	std::string lang = getSkyLanguage();
	language = 0;
	for (int i = 0; i < 2; i++) {
		for (j = 1; j <= 26 && lang[i] != j + 96; j++);
		language = language * 100 + j;
	}
}

//set name as number for selected_object
void Core::setSelectedBodyName(const Object &selected_object)
{
	std::string object_name = selected_object.getEnglishName();
	if (object_name == "Sun")
		selected_body_name = 0;
	else if (object_name == "Mercury")
		selected_body_name = 100;
	else if (object_name == "Venus")
		selected_body_name = 200;
	else if (object_name == "Earth")
		selected_body_name = 300;
	else if (object_name == "Moon")
		selected_body_name = 301;
	else if (object_name == "Mars")
		selected_body_name = 400;
	else if (object_name == "Phobos")
		selected_body_name = 401;
	else if (object_name == "Deimos")
		selected_body_name = 402;
	else if (object_name == "Jupiter")
		selected_body_name = 500;
	else if (object_name == "Io")
		selected_body_name = 501;
	else if (object_name == "Europa")
		selected_body_name = 502;
	else if (object_name == "Ganymed")
		selected_body_name = 503;
	else if (object_name == "Callisto")
		selected_body_name = 504;
	else if (object_name == "Satun")
		selected_body_name = 600;
	else if (object_name == "Tethys")
		selected_body_name = 601;
	else if (object_name == "Rhea")
		selected_body_name = 602;
	else if (object_name == "Dione")
		selected_body_name = 603;
	else if (object_name == "Titan")
		selected_body_name = 604;
	else if (object_name == "Uranus")
		selected_body_name = 700;
	else if (object_name == "Neptune")
		selected_body_name = 800;
	else if (object_name == "Triton")
		selected_body_name = 801;
	else if (object_name == "Pluto")
		selected_body_name = 900;
	else if (object_name == "Charon")
		selected_body_name = 901;
	else
		selected_body_name = 999;
}

// -----------------------------------------------------------------------------
// Same-object logic (module-agnostic): compare by type + J2000 position,
// with optional magnitude tie-breaker.
// -----------------------------------------------------------------------------

// Angular separation (in radians) between two *normalized* vectors
static inline double angularSeparationRadians(const Vec3d& a, const Vec3d& b) {
	// Clamp dot product into [-1, 1] for numeric stability before acos
    return std::acos(std::min(1.0, std::max(-1.0, a * b)));
}

// Per-type matching tolerances:
// - pos_arcsec: angular tolerance in arcseconds (how close on the sky)
// - mag:        magnitude tolerance (secondary tie-breaker)
struct MatchTol { double pos_arcsec; float mag; };

static inline MatchTol matchTolerancesFor(OBJECT_TYPE t) {
    switch (t) {
        case OBJECT_STAR:          return { 0.30, 0.15f }; // very tight for stars
        case OBJECT_BODY:          return { 0.10, 0.50f }; // planets/satellites
        case OBJECT_NEBULA:        return { 5.00, 0.50f }; // extended objects
        case OBJECT_STAR_CLUSTER:  return { 5.00, 0.50f };
        case OBJECT_CONSTELLATION: return { 30.0, 1.00f }; // very loose, if ever used
        case OBJECT_UNINITIALIZED:
        default:                   return { 1.00, 0.50f };
    }
}

// Compare two Object instances for logical equality across modules.
// Fast path: pointer equality (operator==). Otherwise: type + J2000 position,
// with optional magnitude tie-breaker. No name comparison.
static inline bool isSameLogicalObject(const Object& a,
                                       const Object& b,
                                       const Navigator* nav,
                                       bool useMagnitudeTieBreaker = true)
{
    // 0) Fast path: identical underlying pointer → same object
    if (a == b) return true;

    // 1) Different types => different objects
    const OBJECT_TYPE ta = a.getType();
    const OBJECT_TYPE tb = b.getType();
    if (ta != tb) return false;

    // 2) Compare J2000 positions (observer-independent, stable across modules)
    Vec3d ja = a.getObsJ2000Pos(nav);
    Vec3d jb = b.getObsJ2000Pos(nav);

    // Reject degenerate vectors (defensive)
    if (ja.length() == 0.0 || jb.length() == 0.0) return false;

    ja.normalize();
    jb.normalize();

    const MatchTol tol = matchTolerancesFor(ta);

    // Convert arcseconds to radians inline:
    // radians = arcseconds * (π / (180 * 3600))
    const double tolRad = tol.pos_arcsec * (M_PI / (180.0 * 3600.0));

    // If the angular separation is larger than tolerance, not the same object
    if (angularSeparationRadians(ja, jb) > tolRad) return false;

    // 3) Optional tie-breaker: compare magnitudes if both are finite
    if (useMagnitudeTieBreaker) {
        const float ma = a.getMag(nav);
        const float mb = b.getMag(nav);
        if (std::isfinite((double)ma) && std::isfinite((double)mb)) {
            if (std::fabs(ma - mb) > tol.mag) return false;
        }
    }

    return true;
}

//! Select passed object
//! @return true if the object was selected (false if the same was already selected)
bool Core::selectObject(const Object &obj)
{
	// Unselect if it is empty or the same object
	if (!obj) {
		unSelect();
		return false;
	} else if (selected_object && isSameLogicalObject(obj, selected_object, navigation)) {
		// unSelect(); // Keep the object selected if it is already selected
		// Toggle the pointer visibility
		setFlagSelectedObjectPointer(!object_pointer_visibility);
		return true;
	}
	// Make sure object pointer is turned on (script may have turned off)
	// (if this is called from script pointer will be redefined after this function, if pointer isn't defined it will be turned on here)
	setFlagSelectedObjectPointer(true);
	switch (obj.getType()) {
		case OBJECT_CONSTELLATION:
			return selectObject(obj.getBrightestStarInConstellation().get());
		case OBJECT_STAR:
			break;
		default:
			currentAsterisms->setSelected(Object());
	}
	if (obj.getType() == OBJECT_CONSTELLATION)
		return selectObject(obj.getBrightestStarInConstellation().get());

	old_selected_object = selected_object;
	selected_object = obj;
	setSelectedBodyName(selected_object);
	// If an object was selected keep the earth following
	if (getFlagTracking())
		navigation->setFlagLockEquPos(1);
	setFlagTracking(false);

	switch (obj.getType()) {
		case OBJECT_STAR:
			currentSsystemFactory->setSelectedObject(selected_object);
			currentAsterisms->setSelected(selected_object);
			currentHipStars->setSelected(selected_object);

			// Build a constellation with the currently selected stars
			if (currentStarLines->getFlagSelected()) {
				auto selected_stars = currentHipStars->getSelected();
				std::string starLinesCommand = "customConst " + std::to_string(selected_stars.size()-1);
				for (std::size_t i = 0; i + 1 < selected_stars.size(); i++) {
					starLinesCommand += " " + std::to_string(selected_stars[i]);
					starLinesCommand += " " + std::to_string(selected_stars[i+1]);
				}
				currentStarLines->loadStringData(starLinesCommand);
			}

			// potentially record this action
			if (!recordActionCallback.empty())
				recordActionCallback("select " + selected_object.getEnglishName());
			break;
		case OBJECT_BODY:
			currentSsystemFactory->setSelected(selected_object);
			// potentially record this action
			if (!recordActionCallback.empty())
				recordActionCallback("select planet " + selected_object.getEnglishName());
			break;
		case OBJECT_NEBULA:
			currentNebulas->setSelected(selected_object);
			// potentially record this action
			if (!recordActionCallback.empty())
				recordActionCallback("select nebula \"" + selected_object.getEnglishName() + "\"");
			break;
		case OBJECT_STAR_CLUSTER:
			// TODO select it
			break;
		default:
			assert(0);
	}
	return true;
}


//! Find and return the list of at most maxNbItem objects auto-completing passed object I18 name
//! @param objPrefix the first letters of the searched object
//! @param maxNbItem the maximum number of returned object names
//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
std::vector<std::string> Core::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem, bool withType) const
{
	std::vector<std::string> result;
	std::vector <std::string>::const_iterator iter;

	// Get matching planets
	std::vector<std::string> matchingPlanets = currentSsystemFactory->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingPlanets.begin(); iter != matchingPlanets.end(); ++iter)
		withType ? result.push_back(*iter+"(P)") : result.push_back(*iter);
	maxNbItem-=matchingPlanets.size();

	// Get matching constellations
	std::vector<std::string> matchingConstellations = currentAsterisms->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingConstellations.begin(); iter != matchingConstellations.end(); ++iter)
		withType ? result.push_back(*iter+"(C)") : result.push_back(*iter);
	maxNbItem-=matchingConstellations.size();

	// Get matching nebulae
	std::vector<std::string> matchingNebulae = currentNebulas->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingNebulae.begin(); iter != matchingNebulae.end(); ++iter)
		withType ? result.push_back(*iter+"(N)") : result.push_back(*iter);
	maxNbItem-=matchingNebulae.size();

	// Get matching stars
	std::vector<std::string> matchingStars = currentHipStars->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingStars.begin(); iter != matchingStars.end(); ++iter)
		withType ? result.push_back(*iter+"(S)") : result.push_back(*iter);
	maxNbItem-=matchingStars.size();

	std::sort(result.begin(), result.end());

	return result;
}

void Core::setFlagTracking(bool b)
{
	if (!b || !selected_object) {
		navigation->setFlagTraking(0);
	} else if ( !navigation->getFlagTraking()) {
		navigation->moveTo(selected_object.getEarthEquPos(navigation), getAutoMoveDuration());
		navigation->setFlagTraking(1);
	}
}

float Core::starGetSizeLimit(void) const
{
	return currentHipStars->getStarSizeLimit();
}

void Core::setStarSizeLimit(float f)
{
	float planet_limit = getPlanetsSizeLimit();
	currentHipStars->setStarSizeLimit(f);
	setPlanetsSizeLimit(planet_limit);
}

//! Set base planets display scaling factor
//! This is additive to star size limit above
//! since makes no sense to be less
//! ONLY SET THROUGH THIS METHOD
void Core::setPlanetsSizeLimit(float f)
{
	currentSsystemFactory->setSizeLimit(f + starGetSizeLimit());
	currentHipStars->setObjectSizeLimit(f);
}

// set zoom/center offset (percent of fov radius)
void Core::setViewOffset(double offset)
{
	double off = offset;

	// Error checking for allowed limits
	if (offset < -0.5) off = -0.5;
	if (offset > 0.5)  off =  0.5;

	// Update default view vector
	navigation->setViewOffset(off);

	// adjust view direction (if tracking, should be corrected before render)
	navigation->setLocalVision(InitViewPos);

}

std::string Core::getSkyLanguage() {
	return skyTranslator.getLocaleName();
}

bool Core::loadNebula(double ra, double de, double magnitude, double angular_size, double rotation,
                      std::string name, std::string filename, std::string credit, double texture_luminance_adjust,
                      double distance , std::string constellation, std::string type)
{
	std::string tmp_type= type;
	if (tmp_type == "")
		tmp_type = "GENRC";
	return currentNebulas->loadDeepskyObject(name, tmp_type, constellation, ra,de, magnitude, angular_size, "-", distance, filename, true,
	                                  angular_size, rotation, credit, texture_luminance_adjust, true);
}

void Core::removeNebula(const std::string& name)
{
	bool updateSelection = false;

	// Make sure this object is not already selected so won't crash
	if (selected_object.getType()==OBJECT_NEBULA && selected_object.getEnglishName()==name /*&& selected_object.isDeleteable()*/) {
		updateSelection = true;
		selected_object=nullptr;
	}

	currentNebulas->removeNebula(name, true);
	// Try to find original version, if any
	if( updateSelection ) selected_object = currentNebulas->search(name);
}

void Core::removeSupplementalNebulae()
{
	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;
	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.getType()==OBJECT_NEBULA /*&& selected_object.isDeleteable()*/ ) {
		unSelect();
	}
	currentNebulas->removeSupplementalNebulae();
}

bool Core::loadDso2d(int typeDso, std::string name, float size, float alpha, float delta, float distance, int xyz)
{
	return currentDso3d->loadCommand(typeDso, name, size, alpha, delta, distance, xyz);
}

void Core::removeSupplementalDso()
{
	currentDso3d->removeSupplementalDso();
}

void Core::setJDayRelative(int year, int month)
{
	double jd = timeMgr->getJDay();
	ln_date current_date;
	SpaceDate::JulianToDate(jd,&current_date);
	timeMgr->setJDay(SpaceDate::JulianDayFromDateTime(current_date.years+year,current_date.months+month,current_date.days,current_date.hours,current_date.minutes,current_date.seconds));
}

void Core::unSelect(void) {
	selected_object=nullptr;
	old_selected_object=nullptr;
	currentSsystemFactory->setSelected(Object());
}

float Core::getPlanetsSizeLimit(void) const {
	return (currentSsystemFactory->getSizeLimit()-starGetSizeLimit());
}

void Core::update(int delta_time) {
	if (flagEnableTransition) {
		const float deltaSeconds = delta_time / 1000.f;
	   	updateList.remove_if([deltaSeconds](auto *obj){return obj->update(deltaSeconds);});
	}
}

void Core::lookAnchor(const std::string &name, double duration)
{
	// if (name == "observatory") {
	navigation->moveTo(navigation->helioToEarthPosEqu(observatory->getObserverCenterPoint()), duration);
	// } else {
	// 	navigation->moveTo(currentSsystemFactory->, duration);
	// }
}

void Core::setPredictibleRendering(bool enable, int framerate)
{
	predictibleRendering = enable;
	media->setRenderFramerate(framerate);
}

void Core::updateCurrentModulePointers(MODULE newModule)
{
	if (newModule == MODULE::IN_SANDBOX) {
		currentHipStars.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentAsterisms.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentNebulas.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentIlluminates.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentSsystemFactory.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentAtmosphere.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentSkyGridMgr.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentSkyLineMgr.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentSkyDisplayMgr.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentOort.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentDso3d.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentTully.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentMilkyWay.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentBodyDecor.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentMeteors.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentStarNav.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentLandscape.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentToneConverter.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentCloudNav.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentStarGalaxy.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentVolumGalaxy.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentDsoNav.setActive(CURRENT_MODE::SANDBOX_MODE);
		currentStarLines.setActive(CURRENT_MODE::SANDBOX_MODE);
	} else {
		// TODO: Remove all non-currentXXX usage in core.cpp (except init (should use XXX and sandboxXXX there to init both versions))
		// TODO: Once all done, check the init section to correctly init sandboxXXX pointers too
		// TODO: Check if there is some other "pointer to duplicate" for the sandbox module
		// Done
		currentHipStars.setActive(CURRENT_MODE::NORMAL_MODE);
		currentAsterisms.setActive(CURRENT_MODE::NORMAL_MODE);
		currentNebulas.setActive(CURRENT_MODE::NORMAL_MODE);
		currentIlluminates.setActive(CURRENT_MODE::NORMAL_MODE);
		currentSsystemFactory.setActive(CURRENT_MODE::NORMAL_MODE);
		currentAtmosphere.setActive(CURRENT_MODE::NORMAL_MODE);
		currentSkyGridMgr.setActive(CURRENT_MODE::NORMAL_MODE);
		currentSkyLineMgr.setActive(CURRENT_MODE::NORMAL_MODE);
		currentSkyDisplayMgr.setActive(CURRENT_MODE::NORMAL_MODE);
		currentOort.setActive(CURRENT_MODE::NORMAL_MODE);
		currentDso3d.setActive(CURRENT_MODE::NORMAL_MODE);
		currentTully.setActive(CURRENT_MODE::NORMAL_MODE);
		currentMilkyWay.setActive(CURRENT_MODE::NORMAL_MODE);
		currentBodyDecor.setActive(CURRENT_MODE::NORMAL_MODE);
		currentMeteors.setActive(CURRENT_MODE::NORMAL_MODE);
		currentStarNav.setActive(CURRENT_MODE::NORMAL_MODE);
		currentLandscape.setActive(CURRENT_MODE::NORMAL_MODE);
		currentToneConverter.setActive(CURRENT_MODE::NORMAL_MODE);
		currentCloudNav.setActive(CURRENT_MODE::NORMAL_MODE);
		currentStarGalaxy.setActive(CURRENT_MODE::NORMAL_MODE);
		currentVolumGalaxy.setActive(CURRENT_MODE::NORMAL_MODE);
		currentDsoNav.setActive(CURRENT_MODE::NORMAL_MODE);
		currentStarLines.setActive(CURRENT_MODE::NORMAL_MODE);
		// TODO
		// All Done, next:
		// Checking init section
		// Creating helper class to manage unique_ptr (XXX / sandboxXXX) + raw pointer (currentXXX) (Done):
		//     - Easier to manage active pointer
		//     - Easier to init both versions (one function to init both XXX and sandboxXXX)
	}
}
