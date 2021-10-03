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
#include "bodyModule/body_trace.hpp"
#include "eventModule/CoreEvent.hpp"
#include "eventModule/event_recorder.hpp"
#include "coreModule/meteor_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "coreModule/cardinals.hpp"
#include "coreModule/illuminate_mgr.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include "coreModule/time_mgr.hpp"
#include "coreModule/sky_localizer.hpp"
#include "ojmModule/ojm_mgr.hpp"


#include "vulkanModule/VirtualSurface.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/ComputePipeline.hpp"
#include "vulkanModule/Texture.hpp"
#include "vulkanModule/TextureMgr.hpp"

Core::Core(ThreadContext *_context, int width, int height, std::shared_ptr<Media> _media, std::shared_ptr<FontFactory> _fontFactory, const mBoost::callback<void, std::string>& recordCallback, std::shared_ptr<Observer> _observatory) :
	skyTranslator(AppSettings::Instance()->getLanguageDir(), ""),
	projection(nullptr), selected_object(nullptr), hip_stars(nullptr),
	nebulas(nullptr), illuminates(nullptr), ssystemFactory(NULL), milky_way(nullptr)
{
	vzm={0.,0.,0.,0.,0.,0.00025};
	recordActionCallback = recordCallback;
	context = _context;
	media = _media;
	fontFactory = _fontFactory;
	projection = new Projector( width,height, 60 );
	media->setProjector(projection);
	// Set textures directory and suffix
	s_texture::setTexDir(AppSettings::Instance()->getTextureDir() );
	Texture::setTextureDir(AppSettings::Instance()->getTextureDir());
	//set Shaders directory and suffix
	Pipeline::setShaderDir(AppSettings::Instance()->getShaderDir() );
	ComputePipeline::setShaderDir(AppSettings::Instance()->getShaderDir() );
	context->global->textureMgr->initCustomMipmap(context->surface);

	ubo_cam = new UBOCam(context, "cam_block");
	tone_converter = new ToneReproductor();
	atmosphere = std::make_shared<Atmosphere>(context);
	timeMgr = std::make_shared<TimeMgr>();
	navigation = new Navigator();
	observatory = _observatory;
	ssystemFactory = new SSystemFactory(context, observatory.get(), navigation, timeMgr.get());
	nebulas = new NebulaMgr(context);
	milky_way = std::make_shared<MilkyWay>(context);
	starNav = new StarNavigator(context);
	cloudNav = std::make_unique<CloudNavigator>(context);
	universeCloudNav = std::make_unique<CloudNavigator>(context);
	dsoNav = std::make_unique<DsoNavigator>(context, "dso3d-color.png");
	starLines = std::make_unique<StarLines>(context);
	ojmMgr = std::make_unique<OjmMgr>(context);
	bodyDecor = new BodyDecor(milky_way, atmosphere);

	skyGridMgr = std::make_unique<SkyGridMgr>(context);
	skyGridMgr->Create(SKYGRID_TYPE::GRID_EQUATORIAL);
	skyGridMgr->Create(SKYGRID_TYPE::GRID_ECLIPTIC);
	skyGridMgr->Create(SKYGRID_TYPE::GRID_GALACTIC);
	skyGridMgr->Create(SKYGRID_TYPE::GRID_ALTAZIMUTAL);

	skyLineMgr = std::make_unique<SkyLineMgr>(context);
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

	skyDisplayMgr = std::make_unique<SkyDisplayMgr>(context);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_PERSONAL);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_PERSONEQ);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_NAUTICAL);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_NAUTICEQ);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_OBJCOORDS);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_MOUSECOORDS);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_ANGDIST);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_LOXODROMY);
	skyDisplayMgr->Create(SKYDISPLAY_NAME::SKY_ORTHODROMY);

	cardinals_points = std::make_unique<Cardinals>();
	meteors = std::make_unique<MeteorMgr>(10, 60, context);
	landscape = new Landscape();
	skyloc = std::make_unique<SkyLocalizer>(AppSettings::Instance()->getSkyCultureDir());
	hip_stars = new HipStarMgr(width,height, context);
	asterisms = new ConstellationMgr(hip_stars, context);
	illuminates= std::make_unique<IlluminateMgr>(hip_stars, navigation, asterisms, context);
	oort =  std::make_unique<Oort>(context);
	dso3d = std::make_unique<Dso3d>(context);
	tully = std::make_unique<Tully>(context);
	object_pointer_visibility = 1;
}

void Core::registerCoreFont() const
{
	hip_stars->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_HIPSTARS));
	nebulas->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));

	ssystemFactory->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SSYSTEM));
	skyGridMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYGRID));
	skyLineMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYLINE));
	skyDisplayMgr->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_SKYDISPLAY));

	nebulas->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_NEBULAE));
	asterisms->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_ASTERIMS));
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
	delete bodyDecor;
	delete navigation;
	delete projection;
	delete asterisms;
	delete hip_stars;
	delete nebulas;
	//delete illuminates;
	// skyGridMgr.reset(nullptr);
	// skyLineMgr.reset(nullptr);
	// skyDisplayMgr.reset(nullptr);
	delete landscape;
	// delete cardinals_points;
	landscape = nullptr;
	delete geodesic_grid;
	// delete milky_way;
	//delete timeMgr;
	// delete meteors;
	meteors = nullptr;
	//delete atmosphere;
	delete tone_converter;
	// s_font::deleteShader();
	//delete ssystem;
	delete ssystemFactory;
	//delete skyloc;
	//skyloc = nullptr;
	Object::deleteTextures(); // Unload the pointer textures
	// Object::deleteShaders();
	//delete text_usr;
	delete ubo_cam;
	// delete oort;
	// delete dso3d;
	// delete tully;
	// delete ojmMgr;
	delete starNav;
	//delete cloudNav;
	//delete universeCloudNav;
	//delete dsoNav;
	//delete starLines;
}

void Core::setFlagNav(bool a)
{
	flagNav=a;
	cardinals_points->setInternalNav(a);
	skyGridMgr->setInternalNav(a);
	skyLineMgr->setInternalNav(a);
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
	FlagAtmosphericRefraction = conf.getBoolean(SCS_VIEWING,SCK_FLAG_ATMOSPHERIC_REFRACTION);

	initialvalue.initial_landscapeName=conf.getStr(SCS_INIT_LOCATION,SCK_LANDSCAPE_NAME);
	illuminates->setDefaultSize(conf.getDouble(SCS_STARS, SCK_ILLUMINATE_SIZE));

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
		//TODO Oli: pense a utiliser classe de selection de fichiers.
		ssystemFactory->addSystem("Sirius", AppSettings::Instance()->getUserDir() +"sirius_system.ini");
		ssystemFactory->addSystem("51peg", AppSettings::Instance()->getUserDir() + "51peg_system.ini");
		//ssystemFactory->changeSystem("51peg");
		// Init stars
		hip_stars->iniColorTable();
		hip_stars->readColorTable();
		hip_stars->init(conf);

		// Init nebulas
		nebulas->loadDeepskyObject(AppSettings::Instance()->getUserDir() + "deepsky_objects.fab");

		Landscape::createSC_context(context);
		landscape->setSlices(conf.getInt(SCS_RENDERING, SCK_LANDSCAPE_SLICES));
		landscape->setStacks(conf.getInt(SCS_RENDERING, SCK_LANDSCAPE_STACKS));
		setLandscape(initialvalue.initial_landscapeName);

		starNav->loadData(AppSettings::Instance()->getUserDir() + "hip2007.txt", false);
		starLines->loadCat(AppSettings::Instance()->getUserDir() + "asterism.txt", false);
	}

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

	starNav->setMagConverterMagShift(conf.getDouble(SCS_STARS,SCK_MAG_CONVERTER_MAG_SHIFT));
	starNav->setMagConverterMaxMag(conf.getDouble(SCS_STARS,SCK_MAG_CONVERTER_MAX_MAG));
	starNav->setStarSizeLimit(conf.getDouble(SCS_ASTRO,SCK_STAR_SIZE_LIMIT));
	starNav->setScale(conf.getDouble (SCS_STARS, SCK_STAR_SCALE));
	starNav->setMagScale(conf.getDouble (SCS_STARS, SCK_STAR_MAG_SCALE));

	ssystemFactory->setFlagPlanets(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS));
	ssystemFactory->setFlagHints(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_HINTS));
	ssystemFactory->setFlagPlanetsOrbits(conf.getBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_ORBITS));
	ssystemFactory->setFlagLightTravelTime(conf.getBoolean(SCS_ASTRO, SCK_FLAG_LIGHT_TRAVEL_TIME));
	ssystemFactory->setFlagTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
	ssystemFactory->startTrails(conf.getBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS));
	nebulas->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA));
	nebulas->setFlagHints(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_HINTS));
	nebulas->setNebulaNames(conf.getBoolean(SCS_ASTRO,SCK_FLAG_NEBULA_NAMES));
	nebulas->setMaxMagHints(conf.getDouble(SCS_ASTRO, SCK_MAX_MAG_NEBULA_NAME));

	milky_way->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_MILKY_WAY));
	milky_way->setFlagZodiacal(conf.getBoolean(SCS_ASTRO,SCK_FLAG_ZODIACAL_LIGHT));
	starLines->setFlagShow(conf.getBoolean(SCS_ASTRO,SCK_FLAG_STAR_LINES));

	nebulas->setPictoSize(conf.getInt(SCS_VIEWING,SCK_NEBULA_PICTO_SIZE));
	nebulas->setFlagBright(conf.getBoolean(SCS_ASTRO,SCK_FLAG_BRIGHT_NEBULAE));

	ssystemFactory->setScale(hip_stars->getScale());
	setPlanetsSizeLimit(conf.getDouble(SCS_ASTRO, SCK_PLANET_SIZE_MARGINAL_LIMIT));
	ssystemFactory->setFlagClouds(true);

	observatory->load(conf, SCS_INIT_LOCATION);

	// make sure nothing selected or tracked
	deselect();
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

		atmosphere->initGridViewport(projection);
		atmosphere->initGridPos();

		oort->populate(conf.getInt("rendering","oort_elements"));
		oort->build();
		tully->setTexture("typegals.png");
		tully->loadCatalog(AppSettings::Instance()->getUserDir() + "tully.dat");
		dso3d->setTexture("dsocat.png");
		if (dso3d->loadCatalog(AppSettings::Instance()->getUserDir() + "dso3d.dat"))
			dso3d->build();

		ojmMgr->init(context);
		// 3D object integration test
		ojmMgr-> load("in_universe", "Milkyway", AppSettings::Instance()->getModel3DDir() + "Milkyway/Milkyway.ojm",AppSettings::Instance()->getModel3DDir()+"Milkyway/", Vec3f(0.0000001,0.0000001,0.0000001), 0.01);

		// Load the pointer textures
		Object::initTextures();
		ObjectBase::createShaderStarPointeur(context);
		ObjectBase::createShaderPointeur(context);
		//Init of the text's shaders
		s_font::createSC_context(context);
	}

	tone_converter->setWorldAdaptationLuminance(3.75f + atmosphere->getIntensity()*40000.f);

	// Compute planets data and init viewing position position of sun and all the satellites (ie planets)
	ssystemFactory->computePositions(timeMgr->getJDay(), observatory.get());

	// Compute transform matrices between coordinates systems
	navigation->updateTransformMatrices(observatory.get(), timeMgr->getJDay());
	navigation->updateViewMat(projection->getFov());

	ssystemFactory->setSelected(""); //setPlanetsSelected("");	// Fix a bug on macosX! Thanks Fumio!

	std::string skyLocaleName = conf.getStr(SCS_LOCALIZATION, SCK_SKY_LOCALE);
	initialvalue.initial_skyLocale=skyLocaleName;
	setSkyLanguage(skyLocaleName);

	int grid_level = hip_stars->getMaxGridLevel();
	geodesic_grid = new GeodesicGrid(grid_level);
	hip_stars->setGrid(geodesic_grid);

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
	atmosphere->setFlagShow(conf.getBoolean(SCS_LANDSCAPE,SCK_FLAG_ATMOSPHERE));
	atmosphere->setFaderDuration(conf.getDouble(SCS_VIEWING,SCK_ATMOSPHERE_FADE_DURATION));

	// Viewing section
	asterisms->setFlagLines( conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_DRAWING));
	asterisms->setFlagNames(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_NAME));
	asterisms->setFlagBoundaries(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_BOUNDARIES));
	asterisms->setFlagArt(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CONSTELLATION_ART));
	asterisms->setFlagIsolateSelected(conf.getBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_PICK));
	asterisms->setArtIntensity(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_INTENSITY));
	asterisms->setArtFadeDuration(conf.getDouble(SCS_VIEWING,SCK_CONSTELLATION_ART_FADE_DURATION));

	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_AZIMUTAL_GRID));
	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL,conf.getBoolean(SCS_VIEWING,SCK_FLAG_EQUATORIAL_GRID));
	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_ECLIPTIC_GRID));
	skyGridMgr->setFlagShow(SKYGRID_TYPE::GRID_GALACTIC,conf.getBoolean(SCS_VIEWING,SCK_FLAG_GALACTIC_GRID));

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

	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_PERSONAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONAL) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_PERSONEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_PERSONEQ) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_ALT) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_NAUTICEQ, conf.getBoolean(SCS_VIEWING,SCK_FLAG_NAUTICAL_RA) );

	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_OBJCOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_OBJECT_COORDINATES) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_MOUSECOORDS, conf.getBoolean(SCS_VIEWING,SCK_FLAG_MOUSE_COORDINATES) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_ANGDIST, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ANGULAR_DISTANCE) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_LOXODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_LOXODROMY) );
	skyDisplayMgr->setFlagShow(SKYDISPLAY_NAME::SKY_ORTHODROMY, conf.getBoolean(SCS_VIEWING,SCK_FLAG_ORTHODROMY) );
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_GREENWICH, conf.getBoolean(SCS_VIEWING,SCK_FLAG_GREENWICH_LINE));
	skyLineMgr->setFlagShow(SKYLINE_TYPE::LINE_VERTICAL, conf.getBoolean(SCS_VIEWING,SCK_FLAG_VERTICAL_LINE));
	cardinals_points->setFlagShow(conf.getBoolean(SCS_VIEWING,SCK_FLAG_CARDINAL_POINTS));

	ssystemFactory->setFlagMoonScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_MOON_SCALED));
	ssystemFactory->setMoonScale(conf.getDouble (SCS_VIEWING,SCK_MOON_SCALE), true); //? toujours true TODO
	ssystemFactory->setFlagSunScale(conf.getBoolean(SCS_VIEWING, SCK_FLAG_SUN_SCALED));
	ssystemFactory->setSunScale(conf.getDouble (SCS_VIEWING,SCK_SUN_SCALE), true); //? toujours true TODO

	oort->setFlagShow(conf.getBoolean(SCS_VIEWING,SCK_FLAG_OORT));

	setLightPollutionLimitingMagnitude(conf.getDouble(SCS_VIEWING,SCK_LIGHT_POLLUTION_LIMITING_MAGNITUDE));

	//atmosphere->setFlagOptoma(conf.getBoolean(SCS_MAIN, SCK_FLAG_OPTOMA));

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	ssystemFactory->initialSolarSystemBodies();
	setBodyDecor();
	firstTime = 0;
}

void Core::uboCamUpdate()
{
	ubo_cam->setViewport(projection->getViewport());
	ubo_cam->setClippingFov(projection->getClippingFov());
	ubo_cam->setViewportCenter(projection->getViewportFloatCenter());
	ubo_cam->setMVP2D(projection->getMatProjectionOrtho2D());
	ubo_cam->update();
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
	//std::cout << "Core::setLandscapeToBody()" << std::endl;
	if (!autoLandscapeMode)
		return;

	//std::cout << "Core::setLandscapeToBody() with automode" << std::endl;
	if (!observatory->isOnBody())
		return;

	// cas du soleil
	if (observatory->isSun()) {
		this->setLandscape("sun");
	}

	//cas des planetes sauf la Terre
	if (!observatory->isEarth() && !observatory->getHomeBody()->isSatellite()){
		setLandscape(observatory->getHomeBody()->getEnglishName());
		atmosphere->setFlagShow(true);
		bodyDecor->setAtmosphereState(true);
	}

	//cas des satellites des planetes
	if (observatory->getHomeBody()->isSatellite())
		setLandscape("moon");

	//cas spécial Earth
	if (observatory->isEarth())
		setLandscape(initialvalue.initial_landscapeName);

	bodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams()); //, observatory->getSpacecraft());
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

	// par défaut on ne fait pas confiance à l'utilisateur
	autoLandscapeMode = false;

	// un satellite doit avoir Moon comme landscape de base
	if (observatory->getHomeBody()->isSatellite() && landscape->getName() == "moon") {
		autoLandscapeMode = true;
		//std::cout << ": automode moon" << std::endl;
		return;
	}

	// cas du soleil
	if (observatory->isSun() &&  landscape->getName() == "sun") {
		autoLandscapeMode = true;
		//std::cout << ": automode sun" << std::endl;
		return;
	}

	//cas des planetes sauf la Terre
	if (!observatory->isEarth() && !observatory->getHomeBody()->isSatellite() && landscape->getName() == observatory->getHomeBody()->getEnglishName()) {
		autoLandscapeMode = true;
		//std::cout << ": automode planet" << std::endl;
		return;
	}

	//cas spécial Earth
	if (observatory->isEarth() && landscape->getName() == initialvalue.initial_landscapeName) {
		//std::cout << ": automode earth" << std::endl;
		autoLandscapeMode = true;
		return;
	}
}

//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
//! and make it the current landscape
bool Core::loadLandscape(stringHash_t& param)
{

	Landscape* newLandscape = Landscape::createFromHash(param);
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
	ssystemFactory->addBody(param);
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
	ssystemFactory->removeBody(name);
}

void Core::removeSupplementalSolarSystemBodies()
{
	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;
	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.getType()==OBJECT_BODY /*&& selected_object.isDeleteable() */) {
		unSelect();
	}
	ssystemFactory->removeSupplementalBodies(observatory->getHomePlanetEnglishName());
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
	rval = ssystemFactory->searchByNamesI18(name);
	if (rval) return rval;
	rval = nebulas->searchByNameI18n(name);
	if (rval) return rval;
	rval = hip_stars->searchByNameI18n(name).get();
	if (rval) return rval;
	rval = asterisms->searchByNameI18n(name);
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
		selected_object = hip_stars->searchHP(hpnum).get();
		asterisms->setSelected(selected_object);
		hip_stars->setSelected(selected_object);
		ssystemFactory->setSelected(""); //setPlanetsSelected("");

	} else if (type=="star") {
		selected_object = hip_stars->search(id).get();
		asterisms->setSelected(selected_object);
		hip_stars->setSelected(selected_object);
		ssystemFactory->setSelected(""); //setPlanetsSelected("");

	} else if (type=="planet") {
		ssystemFactory->setSelected(id); //setPlanetsSelected(id);
		selected_object = ssystemFactory->getSelected();
		asterisms->setSelected(Object());

	} else if (type=="nebula") {
		selected_object = nebulas->search(id);
		ssystemFactory->setSelected(""); //setPlanetsSelected("");
		asterisms->setSelected(Object());

	} else if (type=="constellation") {

		// Select only constellation, nothing else
		asterisms->setSelected(id);

		selected_object = nullptr;
		ssystemFactory->setSelected(""); //setPlanetsSelected("");

	} else if (type=="constellation_star") {
		// For Find capability, select a star in constellation so can center view on constellation
		asterisms->setSelected(id);
		selected_object = asterisms->getSelected().getBrightestStarInConstellation().get();
		// what is this?
		// 1) Find the hp-number of the 1st star in the selected constellation,
		// 2) find the star of this hpnumber
		// 3) select the constellation of this star ???
		//		const unsigned int hpnum = asterisms->getFirstSelectedHP();
		//		selected_object = hip_stars->searchHP(hpnum);
		//		asterisms->setSelected(selected_object);
		ssystemFactory->setSelected(""); //setPlanetsSelected("");
		//		// Some stars are shared, so now force constellation
		//		asterisms->setSelected(id);
	} else {
		std::cerr << "Invalid selection type specified: " << type << std::endl;
		std::cout << "Invalid selection type specified: " << type << std::endl;
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

void Core::setBodyDecor()
{
	if (!observatory->isOnBody())
		bodyDecor->anchorAssign();
	else
		bodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams());
}

void Core::selectZodiac()
{
	asterisms->deselect();
	asterisms->setSelected("Ari");
	asterisms->setSelected("Tau");
	asterisms->setSelected("Gem");
	asterisms->setSelected("Cnc");
	asterisms->setSelected("Leo");
	asterisms->setSelected("Vir");
	asterisms->setSelected("Sco");
	asterisms->setSelected("Sgr");
	asterisms->setSelected("Cap");
	asterisms->setSelected("Aqr");
	asterisms->setSelected("Psc");
	asterisms->setSelected("Lib");
	selected_object = nullptr;
	ssystemFactory->setSelected(""); //setPlanetsSelected("");
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
	projection->unprojectEarthEqu(x,projection->getViewportHeight()-y,v);
	return findAndSelect(v);
}

//! Deselect all selected objects if any
//! Does deselect selected constellations
void Core::deselect(void)
{
	unSelect();
	asterisms->deselect();
	hip_stars->deselect();
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
	if (ssystemFactory->getFlagShow()) {
		temp = ssystemFactory->searchAround(v, fov_around, navigation, observatory.get(), projection, &is_default_object, bodyDecor->canDrawBody()); //aboveHomePlanet);
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
	if (nebulas->getFlagShow()) {
		temp = nebulas->searchAround(p, fov_around);
		candidates.insert(candidates.begin(), temp.begin(), temp.end());
	}

	// And the stars inside the range
	if (hip_stars->getFlagShow()) {
		std::vector<ObjectBaseP > tmp = hip_stars->searchAround(p, fov_around, geodesic_grid);
		for( std::vector<ObjectBaseP >::const_iterator itr = tmp.begin(); itr != tmp.end(); ++itr ) {
			candidates.push_back( Object(itr->get()) );
		}
	}

	// Now select the object minimizing the function y = distance(in pixel) + magnitude
	float best_object_value;
	best_object_value = 100000.f;
	std::vector<Object>::iterator iter = candidates.begin();
	while (iter != candidates.end()) {
		projection->projectEarthEqu((*iter).getEarthEquPos(navigation), winpos);

		float distance = sqrt((xpos-winpos[0])*(xpos-winpos[0]) + (ypos-winpos[1])*(ypos-winpos[1]));
		float mag = (*iter).getMag(navigation);

		if ((*iter).getType()==OBJECT_NEBULA) {
			if ( nebulas->getFlagHints() ) {
				// make very easy to select IF LABELED
				mag = -1;

			}
		}
		if ((*iter).getType()==OBJECT_BODY) {
			if ( ssystemFactory->getFlag(BODY_FLAG::F_HINTS)) {
				// easy to select, especially pluto
				mag -= 15.f;
			} else {
				mag -= 8.f;
			}
		}
		if (distance + mag < best_object_value) {
			best_object_value = distance + mag;
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
	if (!asterisms) return 0;

	asterisms->loadLinesAndArt(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir);
	asterisms->loadNames(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir + "/constellation_names.eng.fab");
	// Re-translated constellation names
	asterisms->translateNames(skyTranslator);

	// as constellations have changed, clear out any selection and retest for match!
	if (selected_object && selected_object.getType()==OBJECT_STAR) {
		asterisms->setSelected(selected_object);
	} else {
		asterisms->setSelected(Object());
	}

	// Load culture star names in english
	hip_stars->loadCommonNames(AppSettings::Instance()->getSkyCultureDir() + skyCultureDir + "/star_names.fab");
	// Turn on sci names for western culture only
	hip_stars->setFlagSciNames( skyCultureDir.compare(0, 7, "western") ==0 );

	// translate
	hip_stars->updateI18n(skyTranslator);

	return 1;
}


//! For loading custom sky cultures from scripts, use any path
//! Set the current sky culture from the arbitrary path
bool Core::loadSkyCulture(const std::string& culturePath)
{
	// TODO: how to deal with culture hash and current value
	skyCultureDir = "Custom";  // This allows reloading defaults correctly
	if (!asterisms) return 0;

	asterisms->loadLinesAndArt(culturePath);
	asterisms->loadNames(culturePath + "/constellation_names.eng.fab");

	// Re-translated constellation names
	asterisms->translateNames(skyTranslator);

	// as constellations have changed, clear out any selection and retest for match!
	if (selected_object && selected_object.getType()==/*ObjectRecord::*/OBJECT_STAR) {
		asterisms->setSelected(selected_object);
	} else {
		asterisms->setSelected(Object());
	}

	// Load culture star names in english
	hip_stars->loadCommonNames(culturePath + "/star_names.fab");

	// translate
	hip_stars->updateI18n(skyTranslator);

	return 1;
}



//! @brief Set the sky locale and reload the sky objects names for gettext translation
void Core::setSkyLanguage(const std::string& newSkyLocaleName)
{
	if ( !hip_stars || !cardinals_points || !asterisms || ! skyLineMgr->isExist(SKYLINE_TYPE::LINE_ECLIPTIC)) return; // objects not initialized yet

	std::string oldLocale = getSkyLanguage();

	// Update the translator with new locale name
	skyTranslator = Translator(AppSettings::Instance()->getLanguageDir(), newSkyLocaleName);
	cLog::get()->write("Sky locale is " + skyTranslator.getLocaleName(), LOG_TYPE::L_INFO);
	//printf("SkyLocale : %s\n", newSkyLocaleName.c_str());

	// Translate all labels with the new language
	cardinals_points->translateLabels(skyTranslator);
	skyLineMgr->translateLabels(skyTranslator); //ecliptic_line
	asterisms->translateNames(skyTranslator);
	ssystemFactory->translateNames(skyTranslator);
	nebulas->translateNames(skyTranslator);
	hip_stars->updateI18n(skyTranslator);
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
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC, Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));

	nebulas->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_LABEL_COLOR)));
	nebulas->setCircleColor(Utility::strToVec3f(conf.getStr(section,SCK_NEBULA_CIRCLE_COLOR)));

	skyLineMgr->setColor(SKYLINE_TYPE::LINE_PRECESSION, Utility::strToVec3f(conf.getStr(section,SCK_PRECESSION_CIRCLE_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_CIRCUMPOLAR, Utility::strToVec3f(conf.getStr(section,SCK_CIRCUMPOLAR_CIRCLE_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_VERNAL, Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_EQUATOR, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_TROPIC, Utility::strToVec3f(conf.getStr(section,SCK_EQUATOR_COLOR)));

	ssystemFactory->setDefaultBodyColor(conf.getStr(section,SCK_PLANET_NAMES_COLOR), conf.getStr(section,SCK_PLANET_NAMES_COLOR), 
								conf.getStr(section,SCK_PLANET_ORBITS_COLOR), conf.getStr(section,SCK_OBJECT_TRAILS_COLOR));

	// default color override
	asterisms->setLineColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_LINES_COLOR)));
	starLines-> setColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_LINES3D_COLOR)));
	asterisms->setBoundaryColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
	asterisms->setLabelColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
	asterisms->setArtColor(Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ANALEMMALINE, Utility::strToVec3f(conf.getStr(section,SCK_CONST_BOUNDARY_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ANALEMMA, Utility::strToVec3f(conf.getStr(section,SCK_CONST_NAMES_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ARIES,Utility::strToVec3f(conf.getStr(section,SCK_CONST_ART_COLOR)));
	cardinals_points->setColor(Utility::strToVec3f(conf.getStr(section,SCK_CARDINAL_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_ECLIPTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_POLE,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_POLE_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER,Utility::strToVec3f(conf.getStr(section,SCK_GALACTIC_CENTER_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_GREENWICH,Utility::strToVec3f(conf.getStr(section,SCK_GREENWICH_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_MERIDIAN,Utility::strToVec3f(conf.getStr(section,SCK_MERIDIAN_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_PERSONAL,Utility::strToVec3f(conf.getStr(section,SCK_PERSONAL_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_PERSONEQ,Utility::strToVec3f(conf.getStr(section,SCK_PERSONEQ_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_NAUTICAL,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_ALT_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_NAUTICEQ,Utility::strToVec3f(conf.getStr(section,SCK_NAUTICAL_RA_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_OBJCOORDS,Utility::strToVec3f(conf.getStr(section,SCK_OBJECT_COORDINATES_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_MOUSECOORDS,Utility::strToVec3f(conf.getStr(section,SCK_MOUSE_COORDINATES_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_ANGDIST,Utility::strToVec3f(conf.getStr(section,SCK_ANGULAR_DISTANCE_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_LOXODROMY,Utility::strToVec3f(conf.getStr(section,SCK_LOXODROMY_COLOR)));
	skyDisplayMgr->setColor(SKYDISPLAY_NAME::SKY_ORTHODROMY,Utility::strToVec3f(conf.getStr(section,SCK_ORTHODROMY_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_CIRCLE_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_POINT_POLAR, Utility::strToVec3f(conf.getStr(section,SCK_POLAR_COLOR)));
	media->setTextColor(Utility::strToVec3f(conf.getStr(section,SCK_TEXT_USR_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_VERNAL,Utility::strToVec3f(conf.getStr(section,SCK_VERNAL_POINTS_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_VERTICAL,Utility::strToVec3f(conf.getStr(section,SCK_VERTICAL_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ZENITH,Utility::strToVec3f(conf.getStr(section,SCK_ZENITH_COLOR)));
	skyLineMgr->setColor(SKYLINE_TYPE::LINE_ZODIAC,Utility::strToVec3f(conf.getStr(section,SCK_ZODIAC_COLOR)));

	oort->setColor(Utility::strToVec3f(conf.getStr(section,SCK_OORT_COLOR)));
}

//! For use by TUI - saves all current AppSettings::Instance()
void Core::saveCurrentConfig(InitParser &conf)
{
	// localization section
	conf.setStr(SCS_LOCALIZATION, SCK_SKY_CULTURE, getSkyCultureDir());
	conf.setStr(SCS_LOCALIZATION, SCK_SKY_LOCALE, getSkyLanguage());
	// viewing section
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_DRAWING, asterisms->getFlagLines());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_NAME, asterisms->getFlagNames());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_ART, asterisms->getFlagArt());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_BOUNDARIES, asterisms->getFlagBoundaries());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CONSTELLATION_PICK, asterisms->getFlagIsolateSelected());
	conf.setDouble(SCS_VIEWING, SCK_MOON_SCALE, ssystemFactory->getMoonScale());
	conf.setDouble(SCS_VIEWING, SCK_SUN_SCALE, ssystemFactory->getSunScale());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_EQUATORIAL_GRID, skyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ECLIPTIC_GRID, skyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GALACTIC_GRID, skyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_GALACTIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_AZIMUTAL_GRID, skyGridMgr->getFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_EQUATOR_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_EQUATOR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ECLIPTIC_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CARDINAL_POINTS, cardinals_points->getFlagShow());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ZENITH_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ZENITH));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_POLAR_CIRCLE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_POLAR_POINT, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ECLIPTIC_CENTER, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GALACTIC_POLE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GALACTIC_CENTER, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_VERNAL_POINTS, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_VERNAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ANALEMMA, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ANALEMMA));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ANALEMMA_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ARIES_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ARIES));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_ZODIAC, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_ZODIAC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_GREENWICH_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_GREENWICH));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_VERTICAL_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_VERTICAL));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_MERIDIAN_LINE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_MERIDIAN));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_PRECESSION_CIRCLE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_PRECESSION));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_CIRCUMPOLAR_CIRCLE, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_TROPIC_LINES, skyLineMgr->getFlagShow(SKYLINE_TYPE::LINE_TROPIC));
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_MOON_SCALED, ssystemFactory->getFlagMoonScale());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_SUN_SCALED, ssystemFactory->getFlagSunScale());
	conf.setDouble (SCS_VIEWING, SCK_CONSTELLATION_ART_INTENSITY, asterisms->getArtIntensity());
	conf.setDouble (SCS_VIEWING, SCK_CONSTELLATION_ART_FADE_DURATION, asterisms->getArtFadeDuration());
	conf.setDouble(SCS_VIEWING, SCK_LIGHT_POLLUTION_LIMITING_MAGNITUDE, getLightPollutionLimitingMagnitude());
	// Landscape section
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_LANDSCAPE, landscape->getFlagShow());
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_ATMOSPHERE, bodyDecor->getAtmosphereState());
	conf.setBoolean(SCS_LANDSCAPE, SCK_FLAG_FOG, landscape->fogGetFlagShow());
	// Star section
	conf.setDouble (SCS_STARS , SCK_STAR_SCALE, hip_stars->getScale());
	conf.setDouble (SCS_STARS , SCK_STAR_MAG_SCALE, hip_stars->getMagScale());
	conf.setDouble(SCS_STARS , SCK_MAX_MAG_STAR_NAME, hip_stars->getMaxMagName());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_STAR_PICK, hip_stars->getFlagIsolateSelected());
	conf.setBoolean(SCS_STARS , SCK_FLAG_STAR_TWINKLE, hip_stars->getFlagTwinkle());
	conf.setDouble(SCS_STARS , SCK_STAR_TWINKLE_AMOUNT, hip_stars->getTwinkleAmount());
	conf.setDouble(SCS_STARS , SCK_STAR_LIMITING_MAG, hip_stars->getMagConverterMaxScaled60DegMag());
	// Color section
	conf.setStr    (SCS_COLOR, SCK_AZIMUTHAL_COLOR, Utility::vec3fToStr(skyGridMgr->getColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL)));
	conf.setStr    (SCS_COLOR, SCK_EQUATORIAL_COLOR, Utility::vec3fToStr(skyGridMgr->getColor(SKYGRID_TYPE::GRID_EQUATORIAL)));
	conf.setStr    (SCS_COLOR, SCK_ECLIPTIC_COLOR, Utility::vec3fToStr(skyGridMgr->getColor(SKYGRID_TYPE::GRID_ECLIPTIC)));
	conf.setStr    (SCS_COLOR, SCK_EQUATOR_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_EQUATOR)));
	conf.setStr    (SCS_COLOR, SCK_ECLIPTIC_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_ECLIPTIC)));
	conf.setStr    (SCS_COLOR, SCK_MERIDIAN_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_MERIDIAN)));
	conf.setStr    (SCS_COLOR, SCK_ZENITH_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_ZENITH)));
	conf.setStr    (SCS_COLOR, SCK_POLAR_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_CIRCLE_POLAR)));
	conf.setStr    (SCS_COLOR, SCK_POLAR_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_POINT_POLAR)));
	conf.setStr    (SCS_COLOR, SCK_ECLIPTIC_CENTER_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE)));
	conf.setStr    (SCS_COLOR, SCK_GALACTIC_POLE_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_GALACTIC_POLE)));
	conf.setStr    (SCS_COLOR, SCK_GALACTIC_CENTER_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER)));
	conf.setStr    (SCS_COLOR, SCK_VERNAL_POINTS_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_VERNAL)));
	conf.setStr    (SCS_COLOR, SCK_ANALEMMA_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_ANALEMMA)));
	conf.setStr    (SCS_COLOR, SCK_ANALEMMA_LINE_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_ANALEMMALINE)));
	conf.setStr    (SCS_COLOR, SCK_ARIES_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_ARIES)));
	conf.setStr    (SCS_COLOR, SCK_ZODIAC_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_ZODIAC)));
	conf.setStr    (SCS_COLOR, SCK_PERSONAL_COLOR,     Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_PERSONAL)));
	conf.setStr    (SCS_COLOR, SCK_PERSONEQ_COLOR,     Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_PERSONEQ)));
	conf.setStr    (SCS_COLOR, SCK_NAUTICAL_ALT,       Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_NAUTICAL)));
	conf.setStr    (SCS_COLOR, SCK_NAUTICAL_RA,        Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_NAUTICEQ)));
	conf.setStr    (SCS_COLOR, SCK_OBJECT_COORDINATES, Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_OBJCOORDS)));
	conf.setStr    (SCS_COLOR, SCK_MOUSE_COORDINATES,  Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_MOUSECOORDS)));
	conf.setStr    (SCS_COLOR, SCK_ANGULAR_DISTANCE,   Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_ANGDIST)));
	conf.setStr    (SCS_COLOR, SCK_LOXODROMY,          Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_LOXODROMY)));
	conf.setStr    (SCS_COLOR, SCK_ORTHODROMY,         Utility::vec3fToStr(skyDisplayMgr->getColor(SKYDISPLAY_NAME::SKY_ORTHODROMY)));
	conf.setStr    (SCS_COLOR, SCK_GREENWICH_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_GREENWICH)));
	conf.setStr    (SCS_COLOR, SCK_VERTICAL_LINE, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_VERTICAL)));
	conf.setStr    (SCS_COLOR, SCK_CONST_LINES_COLOR, Utility::vec3fToStr(asterisms->getLineColor()));
	conf.setStr    (SCS_COLOR, SCK_CONST_NAMES_COLOR, Utility::vec3fToStr(asterisms->getLabelColor()));
	conf.setStr    (SCS_COLOR, SCK_CONST_ART_COLOR, Utility::vec3fToStr(asterisms->getArtColor()));
	conf.setStr    (SCS_COLOR, SCK_CONST_BOUNDARY_COLOR, Utility::vec3fToStr(asterisms->getBoundaryColor()));
	conf.setStr	   (SCS_COLOR, SCK_NEBULA_LABEL_COLOR, Utility::vec3fToStr(nebulas->getLabelColor()));
	conf.setStr	   (SCS_COLOR, SCK_NEBULA_CIRCLE_COLOR, Utility::vec3fToStr(nebulas->getCircleColor()));
	conf.setStr	   (SCS_COLOR, SCK_PRECESSION_CIRCLE_COLOR, Utility::vec3fToStr(skyLineMgr->getColor(SKYLINE_TYPE::LINE_PRECESSION)));
	conf.setStr    (SCS_COLOR, SCK_CARDINAL_COLOR, Utility::vec3fToStr(cardinals_points->getColor()));
	// Navigation section
	conf.setBoolean(SCS_NAVIGATION, SCK_FLAG_MANUAL_ZOOM, getFlagManualAutoZoom());
	conf.setDouble (SCS_NAVIGATION, SCK_AUTO_MOVE_DURATION, getAutoMoveDuration());
	conf.setDouble (SCS_NAVIGATION, SCK_ZOOM_SPEED, vzm.zoom_speed);
	conf.setDouble (SCS_NAVIGATION, SCK_HEADING, navigation->getHeading());
	// Astro section
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS, ssystemFactory->getFlag(BODY_FLAG::F_TRAIL));
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_BRIGHT_NEBULAE, nebulas->getFlagBright());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_STARS, hip_stars->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME, hip_stars->getFlagNames());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_STAR_PICK, hip_stars->getFlagIsolateSelected());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA, nebulas->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA_NAMES, nebulas->getNebulaNames());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA_HINTS, nebulas->getFlagHints());
	conf.setDouble(SCS_ASTRO, SCK_MAX_MAG_NEBULA_NAME, nebulas->getMaxMagHints());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS, ssystemFactory->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_HINTS, ssystemFactory->getFlag(BODY_FLAG::F_HINTS));
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_ORBITS, ssystemFactory->getFlagPlanetsOrbits());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_LIGHT_TRAVEL_TIME, ssystemFactory->getFlagLightTravelTime());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_MILKY_WAY, milky_way->getFlagShow());
	conf.setDouble(SCS_ASTRO, SCK_MILKY_WAY_INTENSITY, milky_way->getIntensity());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_OBJECT_TRAILS, ssystemFactory->getFlag(BODY_FLAG::F_TRAIL));
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_BRIGHT_NEBULAE, nebulas->getFlagBright());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_STARS, hip_stars->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_STAR_NAME, hip_stars->getFlagNames());
	conf.setBoolean(SCS_VIEWING, SCK_FLAG_STAR_PICK, hip_stars->getFlagIsolateSelected());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA, nebulas->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA_NAMES, nebulas->getNebulaNames());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_NEBULA_HINTS, nebulas->getFlagHints());
	conf.setDouble(SCS_ASTRO, SCK_MAX_MAG_NEBULA_NAME, nebulas->getMaxMagHints());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS, ssystemFactory->getFlagShow());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_HINTS, ssystemFactory->getFlag(BODY_FLAG::F_HINTS));
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_PLANETS_ORBITS, ssystemFactory->getFlagPlanetsOrbits());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_LIGHT_TRAVEL_TIME, ssystemFactory->getFlagLightTravelTime());
	conf.setBoolean(SCS_ASTRO, SCK_FLAG_MILKY_WAY, milky_way->getFlagShow());
	conf.setDouble(SCS_ASTRO, SCK_MILKY_WAY_INTENSITY, milky_way->getIntensity());
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
	if (selected_object.getType()==OBJECT_NEBULA) return nebulas->getLabelColor();
	if (selected_object.getType()==OBJECT_BODY) return ssystemFactory->getDefaultBodyColor("label");
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
	projection->unprojectEarthEqu(x,projection->getViewportHeight()-y,v);
	return v;
}

void Core::turnRight(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAz = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else vzm.deltaAz = 0;
}

void Core::turnLeft(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAz = -1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);

	} else vzm.deltaAz = 0;
}

void Core::turnUp(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAlt = 1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else vzm.deltaAlt = 0;
}

void Core::turnDown(int s)
{
	if (s && FlagEnableMoveKeys) {
		vzm.deltaAlt = -1;
		setFlagTracking(false);
		setFlagLockSkyPosition(false);
	} else vzm.deltaAlt = 0;
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
		projection->unprojectLocal(x2,projection->getViewportHeight()-y2, tempvec2);
		projection->unprojectLocal(x1,projection->getViewportHeight()-y1, tempvec1);
	} else {
		projection->unprojectEarthEqu(x2,projection->getViewportHeight()-y2, tempvec2);
		projection->unprojectEarthEqu(x1,projection->getViewportHeight()-y1, tempvec1);
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
		vzm.deltaAz = -depl/30;
		if (vzm.deltaAz<-0.2) vzm.deltaAz = -0.2;
	} else {
		if (vzm.deltaAz>0) {
			vzm.deltaAz = (depl/30);
			if (vzm.deltaAz>0.2) vzm.deltaAz = 0.2;
		}
	}
	if (vzm.deltaAlt<0) {
		vzm.deltaAlt = -depl/30;
		if (vzm.deltaAlt<-0.2) vzm.deltaAlt = -0.2;
	} else {
		if (vzm.deltaAlt>0) {
			vzm.deltaAlt = depl/30;
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
		observatory->setAltitude(observatory->getAltitude()*vzm.deltaHeight);
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
	ssystemFactory->startTrails( ssystemFactory->getFlag(BODY_FLAG::F_TRAIL));
	Event* event= new ObserverEvent(planet);
	EventRecorder::getInstance()->queue(event);
	bool result = false;
	if (planet=="selected")
		result =  ssystemFactory->switchToAnchor(selected_object.getEnglishName());
	else
		result =  ssystemFactory->switchToAnchor(planet);
	if (result) {
		setLandscapeToBody();
		// il faut obtenir ici la planete sur laquelle on se trouve pour accéder au champ modelAtmosphere qui se trouve dans AtmosphereParams du Body en question
		atmosphere->setModel(observatory->getHomeBody()->getAtmosphereParams()->modelAtmosphere);
	}
}

void Core::setLightPollutionLimitingMagnitude(float mag) {
	lightPollutionLimitingMagnitude = mag;
	float ln = log(mag);
	float lum = 30.0842967491175 -19.9408790405749*ln +2.12969160094949*ln*ln - .2206;
	atmosphere->setLightPollutionLuminance(lum);
}


// For use by TUI
std::string Core::getPlanetHashString()
{
	return ssystemFactory->getPlanetHashString();
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

//! Select passed object
//! @return true if the object was selected (false if the same was already selected)
bool Core::selectObject(const Object &obj)
{
	// Unselect if it is the same object
	if (obj && selected_object==obj) {
		unSelect();
		return true;
	}

	if (obj.getType()==OBJECT_CONSTELLATION) {
		return selectObject(obj.getBrightestStarInConstellation().get());
	} else {
		old_selected_object = selected_object;
		selected_object = obj;

		// If an object has been found
		if (selected_object) {
			// If an object was selected keep the earth following
			if (getFlagTracking()) navigation->setFlagLockEquPos(1);
			setFlagTracking(false);

			if (selected_object.getType()==OBJECT_STAR) {
				asterisms->setSelected(selected_object);
				hip_stars->setSelected(selected_object);

				// Build a constellation with the currently selected stars
				if (starLines->getFlagSelected()) {
					auto selected_stars = hip_stars->getSelected();
					std::string starLinesCommand = "customConst " + std::to_string(selected_stars.size()-1);
					for (std::size_t i = 0; i + 1 < selected_stars.size(); i++) {
						starLinesCommand += " " + std::to_string(selected_stars[i]);
						starLinesCommand += " " + std::to_string(selected_stars[i+1]);
					}
					starLines->loadStringData(starLinesCommand);
				}

				// potentially record this action
				if (!recordActionCallback.empty()) recordActionCallback("select " + selected_object.getEnglishName());
			} else {
				asterisms->setSelected(Object());
			}

			if (selected_object.getType()==OBJECT_BODY) {
				ssystemFactory->setSelected(selected_object);
				// potentially record this action
				if (!recordActionCallback.empty()) recordActionCallback("select planet " + selected_object.getEnglishName());
			}

			if (selected_object.getType()==OBJECT_NEBULA) {
				// potentially record this action
				if (!recordActionCallback.empty()) recordActionCallback("select nebula \"" + selected_object.getEnglishName() + "\"");
			}

			return true;
		} else {
			unSelect();
			return false;
		}
	}
	assert(0);
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
	std::vector<std::string> matchingPlanets = ssystemFactory->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingPlanets.begin(); iter != matchingPlanets.end(); ++iter) 
		withType ? result.push_back(*iter+"(P)") : result.push_back(*iter);
	maxNbItem-=matchingPlanets.size();

	// Get matching constellations
	std::vector<std::string> matchingConstellations = asterisms->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingConstellations.begin(); iter != matchingConstellations.end(); ++iter)
		withType ? result.push_back(*iter+"(C)") : result.push_back(*iter);
	maxNbItem-=matchingConstellations.size();

	// Get matching nebulae
	std::vector<std::string> matchingNebulae = nebulas->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingNebulae.begin(); iter != matchingNebulae.end(); ++iter)
		withType ? result.push_back(*iter+"(N)") : result.push_back(*iter);
	maxNbItem-=matchingNebulae.size();

	// Get matching stars
	std::vector<std::string> matchingStars = hip_stars->listMatchingObjectsI18n(objPrefix, maxNbItem);
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
	return hip_stars->getStarSizeLimit();
}

void Core::setStarSizeLimit(float f)
{
	float planet_limit = getPlanetsSizeLimit();
	hip_stars->setStarSizeLimit(f);
	setPlanetsSizeLimit(planet_limit);
}

//! Set base planets display scaling factor
//! This is additive to star size limit above
//! since makes no sense to be less
//! ONLY SET THROUGH THIS METHOD
void Core::setPlanetsSizeLimit(float f)
{
	ssystemFactory->setSizeLimit(f + starGetSizeLimit());
	hip_stars->setObjectSizeLimit(f);
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
	return nebulas->loadDeepskyObject(name, tmp_type, constellation, ra,de, magnitude, angular_size, "-", distance, filename, true,
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

	nebulas->removeNebula(name, true);
	// Try to find original version, if any
	if( updateSelection ) selected_object = nebulas->search(name);
}

void Core::removeSupplementalNebulae()
{
	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;
	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.getType()==OBJECT_NEBULA /*&& selected_object.isDeleteable()*/ ) {
		unSelect();
	}
	nebulas->removeSupplementalNebulae();
}

void Core::setJDayRelative(int year, int month)
{
	double jd = timeMgr->getJDay();
	ln_date current_date;
	SpaceDate::JulianToDate(jd,&current_date);
	timeMgr->setJDay(SpaceDate::JulianDayFromDateTime(current_date.years+year,current_date.months+month,current_date.days,current_date.hours,current_date.minutes,current_date.seconds));
}