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
	projection(nullptr), selected_object(nullptr), hip_stars(nullptr),
	illuminates(nullptr), ssystemFactory(NULL), milky_way(nullptr)
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
	tone_converter = new ToneReproductor();
	atmosphere = std::make_shared<Atmosphere>();
	timeMgr = std::make_shared<TimeMgr>();
	navigation = new Navigator();
	observatory = _observatory;

	ssystemFactory = new SSystemFactory(observatory.get(), navigation, timeMgr.get());
	sandboxSsystemFactory = new SSystemFactory(observatory.get(), navigation, timeMgr.get());  // Sandbox solar system

	nebulas = std::make_unique<NebulaMgr>();
	sandboxNebulas = std::make_unique<NebulaMgr>();  // Sandbox nebulas

	milky_way = std::make_unique<MilkyWay>();
	sandboxMilkyWay = std::make_unique<MilkyWay>();  // Sandbox milky way

	starNav = std::make_unique<StarNavigator>();
	sandboxStarNav = std::make_unique<StarNavigator>();  // Sandbox star navigator

	cloudNav = std::make_unique<CloudNavigator>();
	sandboxCloudNav = std::make_unique<CloudNavigator>();  // Sandbox cloud navigator

	universeCloudNav = std::make_unique<CloudNavigator>(AppSettings::Instance()->getConfigDir() + "gal3d.dat");

	starGalaxy = std::make_unique<StarGalaxy>(AppSettings::Instance()->getConfigDir() + "gal3d.dat");
	sandboxStarGalaxy = std::make_unique<StarGalaxy>(AppSettings::Instance()->getConfigDir() + "gal3d.dat");  // Sandbox star galaxy

	if (std::filesystem::exists(s_texture::getTexDir() + "milkyway-vguerin-d128.png")) {
		volumGalaxy = std::make_unique<VolumObj3D>("milkyway-vguerin-d128.png", "", false);
		sandboxVolumGalaxy = std::make_unique<VolumObj3D>("milkyway-vguerin-d128.png", "", false);  // Sandbox volum galaxy
	} else {
		volumGalaxy = std::make_unique<VolumObj3D>("mw_rgb_d8.jpg", "mw_d32.png", true);
		sandboxVolumGalaxy = std::make_unique<VolumObj3D>("mw_rgb_d8.jpg", "mw_d32.png", true);  // Sandbox volum galaxy
	}

	dsoNav = std::make_unique<DsoNavigator>();
	sandboxDsoNav = std::make_unique<DsoNavigator>();  // Sandbox dso navigator

	starLines = std::make_unique<StarLines>();
	sandboxStarLines = std::make_unique<StarLines>();  // Sandbox star lines

	ojmMgr = std::make_unique<OjmMgr>();  // Manages mode internally

	bodyDecor = std::make_unique<BodyDecor>(milky_way.get(), atmosphere); // Body decor only use milky_way so get raw pointer
	sandboxBodyDecor = std::make_unique<BodyDecor>(sandboxMilkyWay.get(), atmosphere); // Sandbox body decor

	skyGridMgr = std::make_unique<SkyGridMgr>();
	skyGridMgr->Create(SKYGRID_TYPE::GRID_EQUATORIAL);
	skyGridMgr->Create(SKYGRID_TYPE::GRID_ECLIPTIC);
	skyGridMgr->Create(SKYGRID_TYPE::GRID_GALACTIC);
	skyGridMgr->Create(SKYGRID_TYPE::GRID_ALTAZIMUTAL);

	// Sandbox skyGridMgr (separate collection for sandbox mode)
	sandboxSkyGridMgr = std::make_unique<SkyGridMgr>();
	sandboxSkyGridMgr->Create(SKYGRID_TYPE::GRID_EQUATORIAL);
	sandboxSkyGridMgr->Create(SKYGRID_TYPE::GRID_ECLIPTIC);
	sandboxSkyGridMgr->Create(SKYGRID_TYPE::GRID_GALACTIC);
	sandboxSkyGridMgr->Create(SKYGRID_TYPE::GRID_ALTAZIMUTAL);

	skyLineMgr = std::make_unique<SkyLineMgr>();
	skyLineMgr->Create(SKYLINE_TYPE::LINE_CIRCLE_POLAR);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_POINT_POLAR);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_ECLIPTIC_POLE);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_GALACTIC_POLE);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_ANALEMMA);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_ANALEMMALINE);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_CIRCUMPOLAR);

	skyLineMgr->Create(SKYLINE_TYPE::LINE_GALACTIC_CENTER);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_VERNAL);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_GREENWICH);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_ARIES);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_EQUATOR);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR);

	skyLineMgr->Create(SKYLINE_TYPE::LINE_MERIDIAN);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_TROPIC);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_ECLIPTIC);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_PRECESSION);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_VERTICAL);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_ZODIAC);
	skyLineMgr->Create(SKYLINE_TYPE::LINE_ZENITH);

	// Sandbox skyLineMgr (separate collection for sandbox mode)
	sandboxSkyLineMgr = std::make_unique<SkyLineMgr>();
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_CIRCLE_POLAR);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_POINT_POLAR);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_ECLIPTIC_POLE);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_GALACTIC_POLE);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_ANALEMMA);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_ANALEMMALINE);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_CIRCUMPOLAR);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_GALACTIC_CENTER);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_VERNAL);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_GREENWICH);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_ARIES);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_EQUATOR);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_MERIDIAN);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_TROPIC);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_ECLIPTIC);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_PRECESSION);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_VERTICAL);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_ZODIAC);
	sandboxSkyLineMgr->Create(SKYLINE_TYPE::LINE_ZENITH);

	skyDisplayMgr = std::make_unique<SkyDisplayMgr>();
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_PERSONAL);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_PERSONEQ);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_NAUTICAL);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_NAUTICEQ);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_OBJCOORDS);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_MOUSECOORDS);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_ANGDIST);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_LOXODROMY);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_ORTHODROMY);

	// Sandbox skyDisplayMgr (separate collection for sandbox mode)
	sandboxSkyDisplayMgr = std::make_unique<SkyDisplayMgr>();
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_PERSONAL);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_PERSONEQ);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_NAUTICAL);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_NAUTICEQ);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_OBJCOORDS);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_MOUSECOORDS);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_ANGDIST);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_LOXODROMY);
	sandboxSkyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_ORTHODROMY);

	cardinals_points = std::make_unique<Cardinals>();
	meteors = std::make_unique<MeteorMgr>(10, 60);
	sandboxMeteors = std::make_unique<MeteorMgr>(10, 60);  // Sandbox meteors
	landscape = new Landscape();
	skyloc = std::make_unique<SkyLocalizer>(AppSettings::Instance()->getSkyCultureDir());
	hip_stars = std::make_shared<HipStarMgr>(VulkanMgr::instance->getScreenRect().extent.width, VulkanMgr::instance->getScreenRect().extent.height);
	sandboxHipStars = std::make_shared<HipStarMgr>(VulkanMgr::instance->getScreenRect().extent.width, VulkanMgr::instance->getScreenRect().extent.height); // Sandbox hip stars
	asterisms = std::make_shared<ConstellationMgr>(hip_stars);
	sandboxAsterisms = std::make_shared<ConstellationMgr>(sandboxHipStars);  // Sandbox asterisms
	illuminates= std::make_unique<IlluminateMgr>(hip_stars, navigation, asterisms);
	sandboxIlluminates= std::make_unique<IlluminateMgr>(sandboxHipStars, navigation, sandboxAsterisms);  // Sandbox illuminates
	oort =  std::make_unique<Oort>();
	dso3d = std::make_unique<Dso3d>();
	sandboxDso3d = std::make_unique<Dso3d>();  // Sandbox dso3d
	tully = std::make_unique<Tully>();
	sandboxTully = std::make_unique<Tully>();  // Sandbox tully
	object_pointer_visibility = 1;
}

void Core::registerCoreFont() const
{
	hip_stars->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));
	sandboxHipStars->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));

	nebulas->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));
	sandboxNebulas->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));

	dso3d->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));
	sandboxDso3d->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));

	starNav->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));

	tully->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));
	sandboxTully->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));

	ssystemFactory->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SSYSTEM));
	sandboxSsystemFactory->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SSYSTEM));

	skyGridMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYGRID));
	sandboxSkyGridMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYGRID));

	skyLineMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYLINE));
	sandboxSkyLineMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYLINE));

	skyDisplayMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYDISPLAY));
	sandboxSkyDisplayMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYDISPLAY));

	nebulas->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));
	sandboxNebulas->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));

	asterisms->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_ASTERIMS));
	sandboxAsterisms->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_ASTERIMS));

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
	// delete asterisms;
	// delete hip_stars;
	//delete nebulas;
	//delete illuminates;
	// skyGridMgr.reset(nullptr);
	// skyLineMgr.reset(nullptr);
	// skyDisplayMgr.reset(nullptr);
	delete landscape;
	// delete cardinals_points;
	landscape = nullptr;
	delete geodesic_grid;
	geodesic_grid = nullptr;
	// delete milky_way;
	//delete timeMgr;
	// delete meteors;
	// meteors = nullptr;
	//delete atmosphere;
	delete tone_converter;
	tone_converter = nullptr;
	// s_font::deleteShader();
	//delete ssystem;
	delete ssystemFactory;
	ssystemFactory = nullptr;
	delete sandboxSsystemFactory;
	sandboxSsystemFactory = nullptr;
	//delete skyloc;
	//skyloc = nullptr;
	Object::deleteTextures(); // Unload the pointer textures
	ObjectBase::uninit();
	// Object::deleteShaders();
	//delete text_usr;
	//delete uboCam;
	// delete oort;
	// delete dso3d;
	// delete tully;
	// delete ojmMgr;
	//delete starNav;
	//delete cloudNav;
	//delete universeCloudNav;
	//delete dsoNav;
	//delete starLines;
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
	illuminates->setDefaultSize(conf.getDouble(SCS_STARS, SCK_ILLUMINATE_SIZE));
	sandboxIlluminates->setDefaultSize(conf.getDouble(SCS_STARS, SCK_ILLUMINATE_SIZE));

	// Start splash with no fonts due to font collection delays
	if (firstTime) {
		// Init the solar system first
		ssystemFactory->iniColor( conf.getStr(SCS_COLOR, SCK_PLANET_HALO_COLOR),
							conf.getStr(SCS_COLOR, SCK_PLANET_NAMES_COLOR),
							conf.getStr(SCS_COLOR, SCK_PLANET_ORBITS_COLOR),
							conf.getStr(SCS_COLOR, SCK_OBJECT_TRAILS_COLOR));

		ssystemFactory->iniTess( conf.getInt(SCS_RENDERING, SCK_MIN_TES_LEVEL),
							conf.getInt(SCS_RENDERING, SCK_MAX_TES_LEVEL),
							conf.getInt(SCS_RENDERING, SCK_PLANET_ALTIMETRY_LEVEL),
							conf.getInt(SCS_RENDERING, SCK_MOON_ALTIMETRY_LEVEL),
							conf.getInt(SCS_RENDERING, SCK_EARTH_ALTIMETRY_LEVEL));

		ssystemFactory->modelRingInit(conf.getInt(SCS_RENDERING, SCK_RINGS_LOW),
		                         conf.getInt(SCS_RENDERING, SCK_RINGS_MEDIUM),
		                         conf.getInt(SCS_RENDERING, SCK_RINGS_HIGH));

		ssystemFactory->iniTextures();

		ssystemFactory->load(AppSettings::Instance()->getUserDir() + "ssystem.ini");

		ssystemFactory->anchorManagerInit(conf);
		//TODO Oli: remember to use file selection class.
		ssystemFactory->loadGalacticSystem(AppSettings::Instance()->getUserDir(), "galactic.ini");
		// Init stars
		hip_stars->iniColorTable();
		hip_stars->readColorTable();
		hip_stars->init(conf);

		// Init nebulas
		nebulas->loadDeepskyObject(AppSettings::Instance()->getUserDir() + "deepsky_objects.fab");

		Landscape::createSC_context();
		landscape->setSlices(conf.getInt(SCS_RENDERING, SCK_LANDSCAPE_SLICES));
		landscape->setStacks(conf.getInt(SCS_RENDERING, SCK_LANDSCAPE_STACKS));
		setLandscape(initialvalue.initial_landscapeName);

		starNav->loadData(AppSettings::Instance()->getUserDir() + "hip2007.txt", false);
		starLines->loadCat(AppSettings::Instance()->getUserDir() + "asterism.txt", false);
	}
	ssystemFactory->reloadColors(AppSettings::Instance()->getUserDir() + "ssystem.ini");

	// Astro section
	hip_stars->setFlagShow(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STARS));
	hip_stars->setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
	hip_stars->setScale(conf.getDouble (SCS_STARS, SCK_STAR_SCALE));
	hip_stars->setFlagTwinkle(conf.getBoolean(SCS_STARS, SCK_FLAG_STAR_TWINKLE));
	hip_stars->setTwinkleAmount(conf.getDouble (SCS_STARS, SCK_STAR_TWINKLE_AMOUNT));
	hip_stars->setMaxMagName(conf.getDouble (SCS_STARS, SCK_MAX_MAG_STAR_NAME));
	hip_stars->setMagScale(conf.getDouble (SCS_STARS, SCK_STAR_MAG_SCALE));

	hip_stars->setMagConverterMaxFov(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAX_FOV));
	hip_stars->setMagConverterMinFov(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MIN_FOV));
	hip_stars->setMagConverterMagShift(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAG_SHIFT));
	hip_stars->setMagConverterMaxMag(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAX_MAG));
	hip_stars->setStarSizeLimit(conf.getDouble(SCS_ASTRO,SCK_STAR_SIZE_LIMIT));
	hip_stars->setMagConverterMaxScaled60DegMag(conf.getDouble(SCS_STARS,SCK_STAR_LIMITING_MAG));

	sandboxHipStars->setFlagShow(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STARS));
	sandboxHipStars->setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
	sandboxHipStars->setScale(conf.getDouble (SCS_STARS, SCK_STAR_SCALE));
	sandboxHipStars->setFlagTwinkle(conf.getBoolean(SCS_STARS, SCK_FLAG_STAR_TWINKLE));
	sandboxHipStars->setTwinkleAmount(conf.getDouble (SCS_STARS, SCK_STAR_TWINKLE_AMOUNT));
	sandboxHipStars->setMaxMagName(conf.getDouble (SCS_STARS, SCK_MAX_MAG_STAR_NAME));
	sandboxHipStars->setMagScale(conf.getDouble (SCS_STARS, SCK_STAR_MAG_SCALE));

	sandboxHipStars->setMagConverterMaxFov(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAX_FOV));
	sandboxHipStars->setMagConverterMinFov(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MIN_FOV));
	sandboxHipStars->setMagConverterMagShift(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAG_SHIFT));
	sandboxHipStars->setMagConverterMaxMag(conf.getDouble(SCS_STARS, SCK_MAG_CONVERTER_MAX_MAG));
	sandboxHipStars->setStarSizeLimit(conf.getDouble(SCS_ASTRO,SCK_STAR_SIZE_LIMIT));
	sandboxHipStars->setMagConverterMaxScaled60DegMag(conf.getDouble(SCS_STARS,SCK_STAR_LIMITING_MAG));

	starNav->setFlagShow(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STARS));
	starNav->setMagConverterMagShift(conf.getDouble(SCS_STARS,SCK_MAG_CONVERTER_MAG_SHIFT));
	starNav->setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
	starNav->setMagConverterMaxMag(conf.getDouble(SCS_STARS,SCK_MAG_CONVERTER_MAX_MAG));
	starNav->setStarSizeLimit(conf.getDouble(SCS_ASTRO,SCK_STAR_SIZE_LIMIT));
	starNav->setScale(conf.getDouble (SCS_STARS, SCK_STAR_SCALE));
	starNav->setMaxMagName(conf.getDouble (SCS_STARS, SCK_MAX_MAG_STAR_NAME));
	starNav->setMagScale(conf.getDouble (SCS_STARS, SCK_STAR_MAG_SCALE));

	dso3d->setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
	sandboxDso3d->setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));

	ssystemFactory->setFlagPlanets(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS));
	ssystemFactory->setFlagHints(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_HINTS));
	ssystemFactory->setFlagPlanetsOrbits(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_ORBITS));
	ssystemFactory->setFlagLightTravelTime(conf.getBoolean(SCS_ASTRO, SCK_FLAG_LIGHT_TRAVEL_TIME));
	ssystemFactory->setFlagTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
	ssystemFactory->startTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
	sandboxSsystemFactory->setFlagPlanets(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS));
	sandboxSsystemFactory->setFlagHints(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_HINTS));
	sandboxSsystemFactory->setFlagPlanetsOrbits(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_ORBITS));
	sandboxSsystemFactory->setFlagLightTravelTime(conf.getBoolean(SCS_ASTRO, SCK_FLAG_LIGHT_TRAVEL_TIME));
	sandboxSsystemFactory->setFlagTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
	sandboxSsystemFactory->startTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
	nebulas->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA));
	nebulas->setFlagHints(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_HINTS));
	nebulas->setNebulaNames(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_NAMES));
	nebulas->setMaxMagHints(conf.getDouble(SCS_ASTRO, SCK_MAX_MAG_NEBULA_NAME));
	sandboxNebulas->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA));
	sandboxNebulas->setFlagHints(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_HINTS));
	sandboxNebulas->setNebulaNames(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_NAMES));
	sandboxNebulas->setMaxMagHints(conf.getDouble(SCS_ASTRO, SCK_MAX_MAG_NEBULA_NAME));

	milky_way->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_MILKY_WAY));
	milky_way->setFlagZodiacal(conf.getBoolean(SCS_ASTRO,SCK_FLAG_ZODIACAL_LIGHT));
	sandboxMilkyWay->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_MILKY_WAY));
	sandboxMilkyWay->setFlagZodiacal(conf.getBoolean(SCS_ASTRO,SCK_FLAG_ZODIACAL_LIGHT));

	starLines->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_STAR_LINES));

	nebulas->setPictoSize(conf.getInt(SCS_VIEWING,SCK_NEBULA_PICTO_SIZE));
	nebulas->setFlagBright(conf.getBoolean(SCS_ASTRO,SCK_FLAG_BRIGHT_NEBULAE));
	sandboxNebulas->setPictoSize(conf.getInt(SCS_VIEWING,SCK_NEBULA_PICTO_SIZE));
	sandboxNebulas->setFlagBright(conf.getBoolean(SCS_ASTRO,SCK_FLAG_BRIGHT_NEBULAE));

	ssystemFactory->setScale(hip_stars->getScale());
	sandboxSsystemFactory->setScale(sandboxHipStars->getScale());
	setPlanetsSizeLimit(conf.getDouble(SCS_ASTRO, SCK_PLANET_SIZE_MARGINAL_LIMIT));
	ssystemFactory->setFlagClouds(true);
	sandboxSsystemFactory->setFlagClouds(true);

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
		milky_way->needToUseIris(conf.getBoolean(SCS_MAIN, SCK_MILKYWAY_IRIS));
		milky_way->defineInitialMilkywayState(AppSettings::Instance()->getTextureDir() , conf.getStr(SCS_ASTRO,SCK_MILKY_WAY_TEXTURE),
				conf.getStr(SCS_ASTRO,SCK_MILKY_WAY_IRIS_TEXTURE), conf.getDouble(SCS_ASTRO,SCK_MILKY_WAY_INTENSITY));
		milky_way->defineZodiacalState(AppSettings::Instance()->getTextureDir() + conf.getStr(SCS_ASTRO,SCK_ZODIACAL_LIGHT_TEXTURE), conf.getDouble(SCS_ASTRO,SCK_ZODIACAL_INTENSITY));
		milky_way->setFaderDuration(conf.getInt(SCS_ASTRO,SCK_MILKY_WAY_FADER_DURATION));
		sandboxMilkyWay->needToUseIris(conf.getBoolean(SCS_MAIN, SCK_MILKYWAY_IRIS));
		sandboxMilkyWay->defineInitialMilkywayState(AppSettings::Instance()->getTextureDir() , conf.getStr(SCS_ASTRO,SCK_MILKY_WAY_TEXTURE),
				conf.getStr(SCS_ASTRO,SCK_MILKY_WAY_IRIS_TEXTURE), conf.getDouble(SCS_ASTRO,SCK_MILKY_WAY_INTENSITY));
		sandboxMilkyWay->defineZodiacalState(AppSettings::Instance()->getTextureDir() + conf.getStr(SCS_ASTRO,SCK_ZODIACAL_LIGHT_TEXTURE), conf.getDouble(SCS_ASTRO,SCK_ZODIACAL_INTENSITY));
		sandboxMilkyWay->setFaderDuration(conf.getInt(SCS_ASTRO,SCK_MILKY_WAY_FADER_DURATION));

		atmosphere->initGridViewport(projection);
		atmosphere->initGridPos();

		oort->populate(conf.getInt("rendering","oort_elements"));
		oort->build();

		tully->setTexture("typegals.png");
		tully->loadCatalog(AppSettings::Instance()->getUserDir() + "tully.dat");
		tully->loadBigCatalog(AppSettings::Instance()->getUserDir() + "6df.dat", 5e+12);
		tully->setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));
		sandboxTully->setTexture("typegals.png");
		// sandboxTully->loadCatalog(AppSettings::Instance()->getUserDir() + "tully.dat"); // We are in sandbox mode, we don't load data
		// sandboxTully->loadBigCatalog(AppSettings::Instance()->getUserDir() + "6df.dat", 5e+12); // We are in sandbox mode, we don't load data
		sandboxTully->setFlagNames(conf.getBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME));

		dso3d->setTexture("dsocat.png");
		if (dso3d->loadCatalog(AppSettings::Instance()->getUserDir() + "dso3d.dat"))
			dso3d->build();
		sandboxDso3d->setTexture("dsocat.png");
		if (sandboxDso3d->loadCatalog(AppSettings::Instance()->getUserDir() + "dso3d.dat"))
			sandboxDso3d->build();

		ojmMgr->init();
		// 3D object integration test
		if (volumGalaxy->loaded()) {
			if (std::filesystem::exists(s_texture::getTexDir() + "milkyway-vguerin-d128.png")) {
				volumGalaxy->setModel(Mat4f::translation(Vec3f( -0.002, 0.0001, -0.005)) * Mat4f::yawPitchRoll(112, 0, 90) * Mat4f::scaling(0.01), Vec3f(1, 1, 1/8.));
			} else {
				volumGalaxy->setModel(Mat4f::translation(Vec3f( -0.002, 0.0001, -0.005)) * Mat4f::yawPitchRoll(112, 0, 0) * Mat4f::scaling(0.01), Vec3f(1, 1, 1/8.));
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
		milky_way->restoreDefaultMilky();
		sandboxMilkyWay->restoreDefaultMilky();
	}

	tone_converter->setWorldAdaptationLuminance(3.75f + atmosphere->getIntensity()*40000.f);

	// Compute planets data and init viewing position position of sun and all the satellites (ie planets)
	ssystemFactory->computePositions(timeMgr->getJDay(), observatory.get());
	sandboxSsystemFactory->computePositions(timeMgr->getJDay(), observatory.get());

	// Compute transform matrices between coordinates systems
	navigation->updateTransformMatrices(observatory.get(), timeMgr->getJDay());
	navigation->updateViewMat(projection->getFov());

	ssystemFactory->setSelected(""); //setPlanetsSelected("");	// Fix a bug on macosX! Thanks Fumio!
	sandboxSsystemFactory->setSelected("");

	std::string skyLocaleName = conf.getStr(SCS_LOCALIZATION, SCK_SKY_LOCALE);
	initialvalue.initial_skyLocale=skyLocaleName;
	setSkyLanguage(skyLocaleName);

	int grid_level = hip_stars->getMaxGridLevel();
	geodesic_grid = new GeodesicGrid(grid_level);
	hip_stars->setGrid(geodesic_grid);
	sandboxHipStars->setGrid(geodesic_grid);

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

	meteors->setZHR(conf.getInt(SCS_ASTRO,SCK_METEOR_RATE));
	sandboxMeteors->setZHR(conf.getInt(SCS_ASTRO,SCK_METEOR_RATE));

	InitViewPos = Utility::strToVec3f(conf.getStr(SCS_NAVIGATION,SCK_INIT_VIEW_POS).c_str());

	double viewOffset = conf.getDouble (SCS_NAVIGATION,SCK_VIEW_OFFSET);

	setViewOffset(viewOffset);

	// Load constellations from the correct sky culture
	std::string tmp = conf.getStr(SCS_LOCALIZATION, SCK_SKY_CULTURE);
	initialvalue.initial_skyCulture=tmp;
	setSkyCultureDir(tmp);
	skyCultureDir = tmp;

	// Landscape section
	landscape->setFlagShow(conf.getBoolean(SCS_LANDSCAPE, SCK_FLAG_LANDSCAPE));
	landscape->fogSetFlagShow(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_FOG));

	bodyDecor->setAtmosphereState(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_ATMOSPHERE));
	sandboxBodyDecor->setAtmosphereState(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_ATMOSPHERE));

	atmosphere->setFlagShow(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_ATMOSPHERE));
	atmosphere->setFaderDuration(conf.getDouble(SCS_VIEWING,SCK_ATMOSPHERE_FADE_DURATION));
	atmosphere->setDefaultFaderDuration(conf.getDouble(SCS_VIEWING,SCK_ATMOSPHERE_FADE_DURATION));
	atmosphere->setDefaultMoonBrightness(conf.getDouble(SCS_VIEWING,SCK_MOON_BRIGHTNESS));
	ssystemFactory->setDefaultSunBrightness(conf.getDouble(SCS_VIEWING,SCK_SUN_BRIGHTNESS));
	// sandboxSsystemFactory->setDefaultSunBrightness(conf.getDouble(SCS_VIEWING,SCK_SUN_BRIGHTNESS)); // we don't have anything in sandbox at init

	// Viewing section
	asterisms->setFlagLines( conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_DRAWING));
	asterisms->setFlagNames(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_NAME));
	asterisms->setFlagBoundaries(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_BOUNDARIES));
	asterisms->setFlagArt(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_ART));
	asterisms->setFlagIsolateSelected(conf.getBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_PICK));
	asterisms->setArtIntensity(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_INTENSITY));
	asterisms->setArtFadeDuration(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_FADE_DURATION));
	sandboxAsterisms->setFlagLines( conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_DRAWING));
	sandboxAsterisms->setFlagNames(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_NAME));
	sandboxAsterisms->setFlagBoundaries(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_BOUNDARIES));
	sandboxAsterisms->setFlagArt(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_ART));
	sandboxAsterisms->setFlagIsolateSelected(conf.getBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_PICK));
	sandboxAsterisms->setArtIntensity(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_INTENSITY));
	sandboxAsterisms->setArtFadeDuration(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_FADE_DURATION));

	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_AZIMUTAL_GRID));
	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_EQUATORIAL_GRID));
	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_GRID));
	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_GALACTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_GRID));
	sandboxSkyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_AZIMUTAL_GRID));
	sandboxSkyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_EQUATORIAL_GRID));
	sandboxSkyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_GRID));
	sandboxSkyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_GALACTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_GRID));

	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_EQUATOR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_EQUATOR_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_PRECESSION, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PRECESSION_CIRCLE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_CIRCUMPOLAR_CIRCLE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_TROPIC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_TROPIC_LINES));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_MERIDIAN, conf.getBoolean(SCS_VIEWING,SCK_FLAG_MERIDIAN_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ZENITH, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ZENITH_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_POLAR_CIRCLE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_POLAR_POINT));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_CENTER));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_POLE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_CENTER));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_VERNAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_VERNAL_POINTS));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANALEMMA_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ANALEMMA, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANALEMMA));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ARIES, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ARIES_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ZODIAC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ZODIAC));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_EQUATOR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_EQUATOR_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_PRECESSION, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PRECESSION_CIRCLE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_CIRCUMPOLAR_CIRCLE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_TROPIC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_TROPIC_LINES));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_MERIDIAN, conf.getBoolean(SCS_VIEWING,SCK_FLAG_MERIDIAN_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ZENITH, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ZENITH_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_POLAR_CIRCLE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR, conf.getBoolean(SCS_VIEWING,SCK_FLAG_POLAR_POINT));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_CENTER));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_POLE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_CENTER));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_VERNAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_VERNAL_POINTS));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANALEMMA_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ANALEMMA, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANALEMMA));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ARIES, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ARIES_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_ZODIAC, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ZODIAC));

	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_PERSONAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONAL) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_PERSONEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONEQ) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_ALT) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_RA) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_PERSONAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONAL) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_PERSONEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONEQ) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_ALT) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_RA) );

	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_OBJCOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_OBJECT_COORDINATES) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_MOUSECOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_MOUSE_COORDINATES) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_ANGDIST, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANGULAR_DISTANCE) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_LOXODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_LOXODROMY) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_ORTHODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ORTHODROMY) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_OBJCOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_OBJECT_COORDINATES) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_MOUSECOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_MOUSE_COORDINATES) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_ANGDIST, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANGULAR_DISTANCE) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_LOXODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_LOXODROMY) );
	sandboxSkyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_ORTHODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ORTHODROMY) );

	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GREENWICH, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GREENWICH_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_VERTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_VERTICAL_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GREENWICH, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GREENWICH_LINE));
	sandboxSkyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_VERTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_VERTICAL_LINE));

	cardinals_points->setFlagShow(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CARDINAL_POINTS));

	ssystemFactory->setFlagMoonScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_MOON_SCALED));
	ssystemFactory->setMoonScale(conf.getDouble (SCS_VIEWING,SCK_MOON_SCALE), true); //? always true TODO
	ssystemFactory->setFlagSunScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_SUN_SCALED));
	ssystemFactory->setSunScale(conf.getDouble (SCS_VIEWING,SCK_SUN_SCALE), true); //? always true TODO
	// we don't have anything in sandbox at init
	// sandboxSsystemFactory->setFlagMoonScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_MOON_SCALED));
	// sandboxSsystemFactory->setMoonScale(conf.getDouble (SCS_VIEWING,SCK_MOON_SCALE), true); //? always true TODO
	// sandboxSsystemFactory->setFlagSunScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_SUN_SCALED));
	// sandboxSsystemFactory->setSunScale(conf.getDouble (SCS_VIEWING,SCK_SUN_SCALE), true); //? always true TODO

	oort->setFlagShow(conf.getBoolean(SCS_VIEWING,SCK_FLAG_OORT));

	setLightPollutionLimitingMagnitude(conf.getDouble(SCS_VIEWING,SCK_LIGHT_POLLUTION_LIMITING_MAGNITUDE), true);

	//atmosphere->setFlagOptoma(conf.getBoolean(SCS_MAIN, SCK_FLAG_OPTOMA));

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	ssystemFactory->initialSolarSystemBodies();
	// sandboxSsystemFactory->initialSolarSystemBodies(); // sandbox we don't want to create bodies at init (user will add them manually)
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
	atmosphere->setModel(observatory->getHomeBody()->getAtmosphereParams()->modelAtmosphere);

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
		atmosphere->setFlagShow(true);
		currentBodyDecor->setAtmosphereState(true);
	}

	//case of satellites of planets
	if (observatory->getHomeBody()->isSatellite())
		setLandscape("moon");

	//special case Earth
	if (observatory->isEarth())
		setLandscape(initialvalue.initial_landscapeName);

	currentBodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams()); //, observatory->getSpacecraft());
	std::cout << "Body : " << observatory->getHomeBody()->getEnglishName() << " Landscape : " << landscape->getName() << std::endl;
}

bool Core::setLandscape(const std::string& new_landscape_name)
{
	if (new_landscape_name.empty()) return 0;
	std::string l_min = landscape->getName();
	transform(l_min.begin(), l_min.end(), l_min.begin(), ::tolower);
	if (new_landscape_name == l_min) return 0;

	Landscape* newLandscape = Landscape::createFromFile(AppSettings::Instance()->getUserDir() + "landscapes.ini", new_landscape_name);
	if (!newLandscape) return 0;

	if (landscape) {
		// Copy parameters from previous landscape to new one
		newLandscape->setFlagShow(landscape->getFlagShow());
		newLandscape->fogSetFlagShow(landscape->fogGetFlagShow());
		delete landscape;
		landscape = newLandscape;
	}
	testLandscapeCompatibleWithAutoMode();
	return 1;
}

void Core::testLandscapeCompatibleWithAutoMode()
{
	//std::cout << "testLandscape :" << std::endl;
	if (landscape->getName().empty())	return;
	if (!observatory->isOnBody())		return;

	// by default the user is not trusted
	autoLandscapeMode = false;

	// a satellite must have Moon as its base landscape
	if (observatory->getHomeBody()->isSatellite() && landscape->getName() == "moon") {
		autoLandscapeMode = true;
		//std::cout << ": automode moon" << std::endl;
		return;
	}

	// case of the sun
	if (observatory->isSun() &&  landscape->getName() == "sun") {
		autoLandscapeMode = true;
		//std::cout << ": automode sun" << std::endl;
		return;
	}

	//case of planets except Earth
	if (!observatory->isEarth() && !observatory->getHomeBody()->isSatellite() && landscape->getName() == observatory->getHomeBody()->getEnglishName()) {
		autoLandscapeMode = true;
		//std::cout << ": automode planet" << std::endl;
		return;
	}

	//special case Earth
	if (observatory->isEarth() && landscape->getName() == initialvalue.initial_landscapeName) {
		//std::cout << ": automode earth" << std::endl;
		autoLandscapeMode = true;
		return;
	}
}

void Core::setLandingLandscape(bool landing, float speed)
{
	if (landscape->getFormat() == "spherical")
		landscape->setLanding(landing, speed);
}

//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
//! and make it the current landscape
bool Core::loadLandscape(stringHash_t& param, int landing)
{

	Landscape* newLandscape = Landscape::createFromHash(param, landing);
	if (!newLandscape) return 0;

	if (landscape) {
		// Copy parameters from previous landscape to new one
		newLandscape->setFlagShow(landscape->getFlagShow());
		newLandscape->fogSetFlagShow(landscape->fogGetFlagShow());
		delete landscape;
		landscape = newLandscape;
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

void Core::setBodyDecor(bool init)
{
	if (init) {
		if (!observatory->isOnBody()) {
			bodyDecor->anchorAssign();
			sandboxBodyDecor->anchorAssign();
		} else {
			bodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams());
			sandboxBodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams());
		}
	} else {
		if (!observatory->isOnBody())
			currentBodyDecor->anchorAssign();
		else
			currentBodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams());
	}
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
	if (starNav->getFlagStars() && (currentModule == MODULE::IN_GALAXY || currentModule == MODULE::STELLAR_SYSTEM)) {
		std::vector<ObjectBaseP > tmp = starNav->searchAround(v, fov_around, navigation);
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
bool Core::setSkyCultureDir(const std::string& cultureDir)
{
	if (skyCultureDir == cultureDir) return 1;
	// make sure culture definition exists before attempting or will die
	// Do not comment this out! Rob
	if (skyloc->directoryToSkyCultureEnglish(cultureDir) == "") {
		std::cerr << "Invalid sky culture directory: " << cultureDir << std::endl;
		return 0;
	}
	skyCultureDir = cultureDir;
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
	starNav->loadCommonNames(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir + "/star_names.fab");
	// Turn on sci names for western culture only
	currentHipStars->setFlagSciNames( skyCultureDir.compare(0, 7, "western") ==0 );

	// translate
	currentHipStars->updateI18n(skyTranslator);
	starNav->updateI18n(skyTranslator);

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
	starNav->loadCommonNames(culturePath + "/star_names.fab");

	// translate
	currentHipStars->updateI18n(skyTranslator);
	starNav->updateI18n(skyTranslator);

	return 1;
}



//! @brief Set the sky locale and reload the sky objects names for gettext translation
void Core::setSkyLanguage(const std::string& newSkyLocaleName)
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
	currentSkyLineMgr->translateLabels(skyTranslator); //ecliptic_line
	currentAsterisms->translateNames(skyTranslator);
	currentSsystemFactory->translateNames(skyTranslator);
	currentNebulas->translateNames(skyTranslator);
	currentHipStars->updateI18n(skyTranslator);
	starNav->updateI18n(skyTranslator);
	setLanguage();
}


//! Please keep saveCurrentAppSettings::Instance() up to date with any new color AppSettings::Instance() added here
void Core::setColorScheme(const std::string& skinFile, const std::string& section)
{
	InitParser conf;
	conf.load(skinFile);
	// simple default color, rather than black which doesn't show up
	// Load colors from config file
	skyGridMgr->setColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL, Utility::strToVec3f(conf.getStr(section,SCK_AZIMUTHAL_COLOR)));
	skyGridMgr->setColor(SKYGRID_TYPE::GRID_EQUATORIAL, Utility::strToVec3f(conf.getStr(section,SCK_EQUATORIAL_COLOR)));
	skyGridMgr->setColor(SKYGRID_TYPE::GRID_ECLIPTIC, Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_COLOR)));
	skyGridMgr->setColor(SKYGRID_TYPE::GRID_GALACTIC, Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_COLOR)));
	sandboxSkyGridMgr->setColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL, Utility::strToVec3f(conf.getStr(section,SCK_AZIMUTHAL_COLOR)));
	sandboxSkyGridMgr->setColor(SKYGRID_TYPE::GRID_EQUATORIAL, Utility::strToVec3f(conf.getStr(section,SCK_EQUATORIAL_COLOR)));
	sandboxSkyGridMgr->setColor(SKYGRID_TYPE::GRID_ECLIPTIC, Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_COLOR)));
	sandboxSkyGridMgr->setColor(SKYGRID_TYPE::GRID_GALACTIC, Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_COLOR)));

	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC, Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC, Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));

	nebulas->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_LABEL_COLOR)));
	nebulas->setCircleColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_CIRCLE_COLOR)));
	sandboxNebulas->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_LABEL_COLOR)));
	sandboxNebulas->setCircleColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_CIRCLE_COLOR)));

	dso3d->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_LABEL_COLOR)));
	sandboxDso3d->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_LABEL_COLOR)));

	skyLineMgr->setColor(SKYLINE_TYPE::LINE_PRECESSION, Utility::strToVec3f(conf.getStr(section,SCK_PRECESSION_CIRCLE_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_CIRCUMPOLAR, Utility::strToVec3f(conf.getStr(section,SCK_CIRCUMPOLAR_CIRCLE_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_VERNAL, Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_TROPIC, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_PRECESSION, Utility::strToVec3f(conf.getStr(section,SCK_PRECESSION_CIRCLE_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_CIRCUMPOLAR, Utility::strToVec3f(conf.getStr(section,SCK_CIRCUMPOLAR_CIRCLE_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_VERNAL, Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_TROPIC, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));

	ssystemFactory->setDefaultBodyColor(conf.getStr(section,SCK_PLANET_NAMES_COLOR), conf.getStr(section,SCK_PLANET_NAMES_COLOR),
								conf.getStr(section,SCK_PLANET_ORBITS_COLOR), conf.getStr(section,SCK_OBJECT_TRAILS_COLOR));
	sandboxSsystemFactory->setDefaultBodyColor(conf.getStr(section,SCK_PLANET_NAMES_COLOR), conf.getStr(section,SCK_PLANET_NAMES_COLOR),
								conf.getStr(section,SCK_PLANET_ORBITS_COLOR), conf.getStr(section,SCK_OBJECT_TRAILS_COLOR));

	// default color override
	starLines-> setColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_LINES3D_COLOR)));

	asterisms->setLineColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_LINES_COLOR)));
	asterisms->setBoundaryColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
	asterisms->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
	asterisms->setArtColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));
	sandboxAsterisms->setLineColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_LINES_COLOR)));
	sandboxAsterisms->setBoundaryColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
	sandboxAsterisms->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
	sandboxAsterisms->setArtColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));

	cardinals_points->setColor(Utility::strToVec3f(conf.getStr(section,SCK_CARDINAL_COLOR)));

	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ANALEMMALINE, Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ANALEMMA, Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ARIES,Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GREENWICH,Utility::strToVec3f(conf.getStr(section,SCK_GREENWICH_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_MERIDIAN,Utility::strToVec3f(conf.getStr(section,SCK_MERIDIAN_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ANALEMMALINE, Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ANALEMMA, Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ARIES,Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_GREENWICH,Utility::strToVec3f(conf.getStr(section,SCK_GREENWICH_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_MERIDIAN,Utility::strToVec3f(conf.getStr(section,SCK_MERIDIAN_COLOR)));

	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_PERSONAL,Utility::strToVec3f(conf.getStr(section,SCK_PERSONAL_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_PERSONEQ,Utility::strToVec3f(conf.getStr(section,SCK_PERSONEQ_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_NAUTICAL,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_ALT_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_NAUTICEQ,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_RA_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_OBJCOORDS,Utility::strToVec3f(conf.getStr(section,SCK_OBJECT_COORDINATES_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_MOUSECOORDS,Utility::strToVec3f(conf.getStr(section,SCK_MOUSE_COORDINATES_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_ANGDIST,Utility::strToVec3f(conf.getStr(section,SCK_ANGULAR_DISTANCE_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_LOXODROMY,Utility::strToVec3f(conf.getStr(section,SCK_LOXODROMY_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_ORTHODROMY,Utility::strToVec3f(conf.getStr(section,SCK_ORTHODROMY_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_PERSONAL,Utility::strToVec3f(conf.getStr(section,SCK_PERSONAL_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_PERSONEQ,Utility::strToVec3f(conf.getStr(section,SCK_PERSONEQ_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_NAUTICAL,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_ALT_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_NAUTICEQ,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_RA_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_OBJCOORDS,Utility::strToVec3f(conf.getStr(section,SCK_OBJECT_COORDINATES_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_MOUSECOORDS,Utility::strToVec3f(conf.getStr(section,SCK_MOUSE_COORDINATES_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_ANGDIST,Utility::strToVec3f(conf.getStr(section,SCK_ANGULAR_DISTANCE_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_LOXODROMY,Utility::strToVec3f(conf.getStr(section,SCK_LOXODROMY_COLOR)));
	sandboxSkyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_ORTHODROMY,Utility::strToVec3f(conf.getStr(section,SCK_ORTHODROMY_COLOR)));

	media->setTextColor(Utility::strToVec3f(conf.getStr(section,SCK_TEXT_USR_COLOR)));

	skyLineMgr->setColor(SKYLINE_TYPE::LINE_CIRCLE_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_POINT_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_VERNAL,Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_VERTICAL,Utility::strToVec3f(conf.getStr(section,SCK_VERTICAL_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ZENITH,Utility::strToVec3f(conf.getStr(section,SCK_ZENITH_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ZODIAC,Utility::strToVec3f(conf.getStr(section,SCK_ZODIAC_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_CIRCLE_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_POINT_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_VERNAL,Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_VERTICAL,Utility::strToVec3f(conf.getStr(section,SCK_VERTICAL_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ZENITH,Utility::strToVec3f(conf.getStr(section,SCK_ZENITH_COLOR)));
	sandboxSkyLineMgr->setColor(SKYLINE_TYPE::LINE_ZODIAC,Utility::strToVec3f(conf.getStr(section,SCK_ZODIAC_COLOR)));

	oort->setColor(Utility::strToVec3f(conf.getStr(section,SCK_OORT_COLOR)));
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
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_LANDSCAPE, landscape->getFlagShow());
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_ATMOSPHERE, currentBodyDecor->getAtmosphereState());
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_FOG, landscape->fogGetFlagShow());
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
	conf.setStr(SCS_INIT_LOCATION , SCK_LANDSCAPE_NAME, landscape->getName() );
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

void Core::setLightPollutionLimitingMagnitude(float mag, bool init) {
	lightPollutionLimitingMagnitude = mag;
	float ln = log(mag);
	float lum = 30.0842967491175 -19.9408790405749*ln +2.12969160094949*ln*ln - .2206;
	atmosphere->setLightPollutionLuminance(lum);
	float pollum = (5.0-mag)*0.1;

	// This function is called by Core::init and by sts script command (set)
	if (init) {
		// If we are being called by Core::init, set both normal and sandbox milky way
		milky_way->setPollum((pollum < 0) ? 0 : pollum);
		sandboxMilkyWay->setPollum((pollum < 0) ? 0 : pollum);
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
			if (starLines->getFlagSelected()) {
				auto selected_stars = currentHipStars->getSelected();
				std::string starLinesCommand = "customConst " + std::to_string(selected_stars.size()-1);
				for (std::size_t i = 0; i + 1 < selected_stars.size(); i++) {
					starLinesCommand += " " + std::to_string(selected_stars[i]);
					starLinesCommand += " " + std::to_string(selected_stars[i+1]);
				}
				starLines->loadStringData(starLinesCommand);
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
		currentHipStars = sandboxHipStars.get();
		currentAsterisms = sandboxAsterisms.get();
		currentNebulas = sandboxNebulas.get();
		currentIlluminates = sandboxIlluminates.get();
		currentSsystemFactory = sandboxSsystemFactory;
		currentSkyGridMgr = sandboxSkyGridMgr.get();
		currentSkyLineMgr = sandboxSkyLineMgr.get();
		currentSkyDisplayMgr = sandboxSkyDisplayMgr.get();
		currentDso3d = sandboxDso3d.get();
		currentTully = sandboxTully.get();
		currentMilkyWay = sandboxMilkyWay.get();
		currentBodyDecor = sandboxBodyDecor.get();
		currentMeteors = sandboxMeteors.get();
		currentStarNav = sandboxStarNav.get();
		currentCloudNav = sandboxCloudNav.get();
		currentStarGalaxy = sandboxStarGalaxy.get();
		currentVolumGalaxy = sandboxVolumGalaxy.get();
		currentDsoNav = sandboxDsoNav.get();
		currentStarLines = sandboxStarLines.get();
	} else {
		// TODO: Remove all non-currentXXX usage in core.cpp (except init (should use XXX and sandboxXXX there to init both versions))
		// TODO: Once all done, check the init section to correctly init sandboxXXX pointers too
		// TODO: Check if there is some other "pointer to duplicate" for the sandbox module
		// Done
		currentHipStars = hip_stars.get();
		currentAsterisms = asterisms.get();
		currentNebulas = nebulas.get();
		currentIlluminates = illuminates.get();
		currentSsystemFactory = ssystemFactory;
		currentSkyGridMgr = skyGridMgr.get();
		currentSkyLineMgr = skyLineMgr.get();
		currentSkyDisplayMgr = skyDisplayMgr.get();
		currentDso3d = dso3d.get();
		currentTully = tully.get();
		currentMilkyWay = milky_way.get();
		currentBodyDecor = bodyDecor.get();
		currentMeteors = meteors.get();
		// TODO
		currentStarNav = starNav.get();
		currentCloudNav = cloudNav.get();
		currentStarGalaxy = starGalaxy.get();
		currentVolumGalaxy = volumGalaxy.get();
		currentDsoNav = dsoNav.get();
		currentStarLines = starLines.get();
	}
}
