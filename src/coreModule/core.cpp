/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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
#include "starModule/hip_star_mgr.hpp"
#include "tools/log.hpp"
#include "tools/fmath.hpp"
#include "coreModule/ubo_cam.hpp"

#include "coreModule/core_executor.hpp"
#include "navModule/anchor_manager.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_body.hpp"
#include "appModule/space_date.hpp"
#include "bodyModule/body_decor.hpp"
#include "coreModule/starNavigator.hpp"
#include "coreModule/tully.hpp"
#include "coreModule/oort.hpp"
#include "coreModule/dso3d.hpp"
#include "coreModule/landscape.hpp"
#include "mediaModule/media.hpp"
#include "coreModule/starLines.hpp"
#include "bodyModule/body_trace.hpp"

Core::Core(AppSettings* _settings, int width, int height, Media* _media, const mBoost::callback<void, std::string>& recordCallback) :
	skyTranslator(PACKAGE, _settings->getLocaleDir(), ""),
	projection(nullptr), selected_object(nullptr), hip_stars(nullptr),
	nebulas(nullptr), illuminates(nullptr), ssystem(NULL), milky_way(nullptr)
	// deltaAlt(0.), deltaAz(0.), deltaFov(0.),deltaHeight(0.),
	// move_speed(0.00025)
{
	vzm={0.,0.,0.,0.,0.,0.00025};
	recordActionCallback = recordCallback;
	settings = _settings;
	media = _media;
	projection = new Projector( width,height, 60 );
	glFrontFace(GL_CCW);

	// Set textures directory and suffix
	s_texture::setTexDir(settings->getTextureDir() );
	//set Shaders directory and suffix
	shaderProgram::setShaderDir(settings->getShaderDir() );
	shaderProgram::setLogFile(settings->getLogDir()+"shader.log");
	shaderProgram::initLogFile();

	ubo_cam = new UBOCam("cam_block");
	tone_converter = new ToneReproductor();
	atmosphere = new Atmosphere();
	ssystem = new SolarSystem();
	timeMgr = new TimeMgr();
	observatory = new Observer(*ssystem);
	navigation = new Navigator();
	nebulas = new NebulaMgr();
	illuminates= new IlluminateMgr();
	milky_way = new MilkyWay();
	starNav = new StarNavigator();
	starLines = new StarLines();
	ojmMgr = new OjmMgr();
	anchorManager = new AnchorManager(observatory,navigation, ssystem, timeMgr, ssystem->getOrbitCreator());
	bodyDecor = new BodyDecor(milky_way, atmosphere);
		

	skyGridMgr = new SkyGridMgr();
	skyGridMgr->Create("GRID_EQUATORIAL");
	skyGridMgr->Create("GRID_ECLIPTIC");
	skyGridMgr->Create("GRID_GALACTIC");
	skyGridMgr->Create("GRID_ALTAZIMUTAL");

	skyLineMgr = new SkyLineMgr();
	skyLineMgr->Create("LINE_CIRCLE_POLAR");
	skyLineMgr->Create("LINE_POINT_POLAR");
	skyLineMgr->Create("LINE_ECLIPTIC_POLE");
	skyLineMgr->Create("LINE_GALACTIC_POLE");
	skyLineMgr->Create("LINE_ANALEMMA");
	skyLineMgr->Create("LINE_ANALEMMALINE");
	skyLineMgr->Create("LINE_CIRCUMPOLAR");

	skyLineMgr->Create("LINE_GALACTIC_CENTER");
	skyLineMgr->Create("LINE_VERNAL");
	skyLineMgr->Create("LINE_GREENWICH");
	skyLineMgr->Create("LINE_ARIES");
	skyLineMgr->Create("LINE_EQUATOR");
	skyLineMgr->Create("LINE_GALACTIC_EQUATOR");

	skyLineMgr->Create("LINE_MERIDIAN");
	skyLineMgr->Create("LINE_TROPIC");
	skyLineMgr->Create("LINE_ECLIPTIC");
	skyLineMgr->Create("LINE_PRECESSION");
	skyLineMgr->Create("LINE_VERTICAL");
	skyLineMgr->Create("LINE_ZODIAC");
	skyLineMgr->Create("LINE_ZENITH");

	personal = new SkyPerson(SkyPerson::AL);
	personeq = new SkyPerson(SkyPerson::EQ);
	nautical = new SkyPerson(SkyPerson::AL);
	nauticeq = new SkyPerson(SkyPerson::EQ);
	objCoord = new SkyPerson(SkyPerson::AL);
	mouseCoord = new SkyPerson(SkyPerson::AL);
	angDist = new SkyPerson(SkyPerson::AL);
	loxodromy = new SkyPerson(SkyPerson::EQ);
	orthodromy = new SkyPerson(SkyPerson::AL);
	
	cardinals_points = new Cardinals();
	meteors = new MeteorMgr(10, 60);
	landscape = new Landscape();
	inactiveLandscape = new Landscape();
	skyloc = new SkyLocalizer(settings->getSkyCultureDir());
	hip_stars = new HipStarMgr(width,height);
	asterisms = new ConstellationMgr(hip_stars);
	text_usr = new TextMgr();
	mCity = new mCity_Mgr();
	oort =  new Oort();
	dso3d = new Dso3d();
	tully = new Tully();
	mouseX = 0;
	mouseY = 0;

	bodytrace= new BodyTrace();
	object_pointer_visibility = 1;

	tcp = nullptr;
	enable_tcp = false;

	executorInSolarSystem = new CoreExecutorInSolarSystem(this, observatory);
	executorInGalaxy = new CoreExecutorInGalaxy(this,observatory);
	executorInUniverse = new CoreExecutorInUniverse(this,observatory);

	executorInSolarSystem->defineUpMode(executorInGalaxy);
	executorInGalaxy->defineUpMode(executorInUniverse);
	executorInGalaxy->defineDownMode(executorInSolarSystem);
	executorInUniverse->defineDownMode(executorInGalaxy);

	currentExecutor = executorInSolarSystem;
}

void Core::setMouse(int x, int y) {
	mouseX = x;
	mouseY = y;
}

void Core::tcpGetStatus(std::string value) const
{
	std::string toSend="";
	if (value == "constellation") {
		toSend = asterisms->getSelectedShortName();
		cLog::get()->write("Valeur de toSend : " + toSend);
	} else
		toSend="Demande inconnue";

	if (toSend=="")
		toSend="EOF";

	tcpSend(toSend);
}

void Core::tcpSend(std::string msg) const
{
	if (enable_tcp) {
		tcp->setOutput(msg);
		cLog::get()->write("Send command : " + msg);
	} else
		cLog::get()->write("No send msg because no tcp enable");
}

void Core::tcpGetSelectedObjectInfo() const
{
	std::string toSend=getSelectedObjectInfo();
	if (toSend=="")
		toSend="EOL";

	tcpSend(toSend);
}


void Core::tcpConfigure(ServerSocket * _tcp)
{
	if (_tcp!=nullptr) {
		tcp= _tcp;
		enable_tcp = true;
		cLog::get()->write("Core tcp enable");
	} else
		cLog::get()->write("Core no tcp enable");
}


void Core::tcpGetListMatchingObjects(const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> tmp;
	std::string msgToSend;
	tmp = listMatchingObjectsI18n(objPrefix, maxNbItem);
	for( std::vector<std::string>::const_iterator itr = tmp.begin(); itr != tmp.end(); ++itr ) {
		msgToSend = msgToSend + (*itr)+";";
	}

	if (msgToSend=="")
		msgToSend="NOF"; // no object found

	tcpSend(msgToSend);
}

void Core::tcpGetPlanetsStatus() const
{
	std::string msgToSend;
	msgToSend = ssystem->getPlanetsPosition();

	if (msgToSend=="")
		msgToSend="NPF"; // no planet found ! Dramatic

	tcpSend(msgToSend);
}

void Core::tcpGetPosition()
{
	char tmp[1024];
	memset(tmp, '\0', 1024);
	sprintf(tmp,"%2.2f;%3.2f;%10.2f;%10.6f;%10.6f;", observatoryGetLatitude(), observatoryGetLongitude(), observatoryGetAltitude(), getJDay(), getHeading());
	cLog::get()->write(tmp);
	tcp->setOutput(tmp);
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
	delete illuminates;
	delete skyGridMgr;
	delete skyLineMgr;
	delete personal;
	delete personeq;
	delete nautical;
	delete nauticeq;
	delete objCoord;
	delete mouseCoord;
	delete angDist;
	delete loxodromy;
	delete orthodromy;
	delete landscape;
	delete inactiveLandscape;
	delete cardinals_points;
	landscape = nullptr;
	delete observatory;
	observatory = nullptr;
	delete geodesic_grid;
	delete milky_way;
	delete timeMgr;
	delete meteors;
	meteors = nullptr;
	delete atmosphere;
	delete tone_converter;
	s_font::deleteShader();
	delete ssystem;
	delete skyloc;
	skyloc = nullptr;
	Object::deleteTextures(); // Unload the pointer textures
	Object::deleteShaders();
	delete text_usr;
	delete mCity;
	delete bodytrace;
	delete ubo_cam;
	delete oort;
	delete dso3d;
	delete tully;
	delete ojmMgr;

	delete starNav;
	delete starLines;
	delete executorInGalaxy;
	delete executorInSolarSystem;
	delete executorInUniverse;
	
	delete anchorManager;
}


//! Load core data and initialize with default values
void Core::init(const InitParser& conf)
{
	flagNav= conf.getBoolean("main", "flag_navigation");
	setFlagNav(flagNav);
	inimBackup();
	FlagAtmosphericRefraction = conf.getBoolean("viewing:flag_atmospheric_refraction");
	FontFileNameGeneral = settings->getUserFontDir()+conf.getStr("font", "font_general_name");
	FontFileNamePlanet = settings->getUserFontDir()+conf.getStr("font", "font_planet_name");
	FontFileNameConstellation = settings->getUserFontDir()+conf.getStr("font", "font_constellation_name");
	FontFileNameText =  settings->getUserFontDir()+conf.getStr("font", "font_text_name");
	FontSizeText =  conf.getDouble("font", "font_text_size");
	FontSizeGeneral = conf.getDouble ("font","font_general_size");
	FontSizeConstellation = conf.getDouble("font","font_constellation_size");
	FontSizePlanet = conf.getDouble("font","font_planet_size");
	FontSizeCardinalPoints = conf.getDouble("font","font_cardinalpoints_size");

	// Rendering options
	setLineWidth(conf.getDouble("rendering", "line_width"));
	setFlagAntialiasLines(conf.getBoolean("rendering", "flag_antialias_lines"));

	mBackup.initial_landscapeName=conf.getStr("init_location","landscape_name");
	illuminate_size=conf.getDouble("stars", "illuminate_size");

	glDepthFunc(GL_LEQUAL);
	glDepthRange(0,1);

	// Start splash with no fonts due to font collection delays
	if (firstTime) {
		// Init the solar system first
		ssystem->iniColor( conf.getStr("color","planet_halo_color"),
							conf.getStr("color","planet_names_color"),
							conf.getStr("color","planet_orbits_color"),
							conf.getStr("color","object_trails_color"));

		ssystem->modelRingInit(conf.getInt("rendering:rings_low"),
		                         conf.getInt("rendering:rings_medium"),
		                         conf.getInt("rendering:rings_high"));

		ssystem->iniTextures();

		ssystem->load(settings->getUserDir() + "ssystem.ini");
		
		anchorManager->setRotationMultiplierCondition(conf.getDouble("navigation", "stall_radius_unit"));

		anchorManager->load(settings->getUserDir() + "anchor.ini");
		anchorManager->initFirstAnchor(conf.getStr("init_location","home_planet"));

		// Init stars
		hip_stars->iniColorTable();
		hip_stars->readColorTable();
		hip_stars->init(FontSizeGeneral, FontFileNameGeneral, conf);

		// Init nebulas
		nebulas->initFontName(FontSizeGeneral, FontFileNameGeneral);
		nebulas->loadDeepskyObject(settings->getUserDir() + "deepsky_objects.fab");

		landscape->setSlices(conf.getInt("rendering:landscape_slices"));
		landscape->setStacks(conf.getInt("rendering:landscape_stacks"));

		starNav->loadData(settings->getUserDir() + "hip2007.dat", true);
		starLines->loadHipBinCatalogue(settings->getUserDir() + "asterism.dat");
	}

	// Astro section
	hip_stars->setFlagStars(conf.getBoolean("astro:flag_stars"));
	hip_stars->setFlagNames(conf.getBoolean("astro:flag_star_name"));
	hip_stars->setScale(conf.getDouble ("stars", "star_scale"));
	hip_stars->setFlagTwinkle(conf.getBoolean("stars", "flag_star_twinkle"));
	hip_stars->setTwinkleAmount(conf.getDouble ("stars", "star_twinkle_amount"));
	hip_stars->setMaxMagName(conf.getDouble ("stars", "max_mag_star_name"));
	hip_stars->setMagScale(conf.getDouble ("stars", "star_mag_scale"));

	hip_stars->setMagConverterMaxFov(conf.getDouble("stars","mag_converter_max_fov"));
	hip_stars->setMagConverterMinFov(conf.getDouble("stars","mag_converter_min_fov"));
	hip_stars->setMagConverterMagShift(conf.getDouble("stars","mag_converter_mag_shift"));
	hip_stars->setMagConverterMaxMag(conf.getDouble("stars","mag_converter_max_mag"));
	hip_stars->setStarSizeLimit(conf.getDouble("astro","star_size_limit"));
	hip_stars->setMagConverterMaxScaled60DegMag(conf.getDouble("stars","star_limiting_mag"));

	starNav->setMagConverterMagShift(conf.getDouble("stars","mag_converter_mag_shift"));
	starNav->setMagConverterMaxMag(conf.getDouble("stars","mag_converter_max_mag"));

	starNav->setStarSizeLimit(conf.getDouble("astro","star_size_limit"));
	starNav->setScale(conf.getDouble ("stars", "star_scale"));
	starNav->setMagScale(conf.getDouble ("stars", "star_mag_scale"));


	ssystem->setFlagPlanets(conf.getBoolean("astro:flag_planets"));
	ssystem->setFlagHints(conf.getBoolean("astro:flag_planets_hints"));
	ssystem->setFlagPlanetsOrbits(conf.getBoolean("astro:flag_planets_orbits"));
	setFlagLightTravelTime(conf.getBoolean("astro", "flag_light_travel_time"));
	ssystem->setFlagTrails(conf.getBoolean("astro", "flag_object_trails"));
	startPlanetsTrails(conf.getBoolean("astro", "flag_object_trails"));
	nebulas->setFlagShow(conf.getBoolean("astro:flag_nebula"));
	nebulas->setFlagHints(conf.getBoolean("astro","flag_nebula_hints"));
	nebulas->setNebulaNames(conf.getBoolean("astro","flag_nebula_names"));
	nebulas->setMaxMagHints(conf.getDouble("astro", "max_mag_nebula_name"));

	milky_way->setFlagShow(conf.getBoolean("astro:flag_milky_way"));
	milky_way->setFlagZodiacal(conf.getBoolean("astro:flag_zodiacal_light"));
	starLines->setFlagShow(conf.getBoolean("astro:flag_star_lines"));

	nebulas->setPictoSize(conf.getInt("viewing:nebula_picto_size"));
	nebulas->setFlagBright(conf.getBoolean("astro:flag_bright_nebulae"));

	ssystem->setScale(hip_stars->getScale());
	setPlanetsSizeLimit(conf.getDouble("astro", "planet_size_marginal_limit"));

	ssystem->setFont(FontSizePlanet, FontFileNamePlanet);
	setFlagClouds(true);

	observatory->load(conf, "init_location");

	// make sure nothing selected or tracked
	deselect();
	navigation->setFlagTraking(0);
	navigation->setFlagLockEquPos(0);

	timeMgr->setTimeSpeed(JD_SECOND);  // reset to real time

	timeMgr->setJDay(SpaceDate::JulianFromSys());
	navigation->setLocalVision(Vec3f(1,1e-05,0.2));

	// Init fonts : should be moved in a specific fonction
	skyGridMgr->setFont(FontSizeGeneral, FontFileNameGeneral);
	skyLineMgr->setFont(FontSizeGeneral, FontFileNameGeneral);

	cardinals_points->setFont(FontSizeCardinalPoints, FontFileNameGeneral);
	asterisms->setFont(FontSizeConstellation, FontFileNameConstellation);
	hip_stars->setFont(FontSizeGeneral, FontFileNameGeneral);
	text_usr->setFont(FontSizeText, FontFileNameText);

	if (firstTime) {
		milky_way->defineInitialMilkywayState(settings->getTextureDir() , conf.getStr("astro:milky_way_texture"), 
				conf.getStr("astro:milky_way_iris_texture"), conf.getDouble("astro","milky_way_intensity"));
		milky_way->defineZodiacalState(settings->getTextureDir() + conf.getStr("astro:zodiacal_light_texture"), conf.getDouble("astro","zodiacal_intensity"));
		milky_way->setFaderDuration(conf.getInt("astro:milky_way_fader_duration")*1000);

		atmosphere->initGridViewport(projection);
		atmosphere->initGridPos();

		oort->populate(conf.getInt("rendering:oort_elements"));
		tully->setTexture("typegals.png");
		tully->loadCatalog(settings->getUserDir() + "tully.dat");
		dso3d->setTexture("dsocat.png");
		dso3d->loadCatalog(settings->getUserDir() + "dso3d.dat");

		ojmMgr->init();
		// 3D object integration test
		ojmMgr-> load("in_universe", "Milkyway", settings->getModel3DDir() + "Milkyway/Milkyway.ojm",settings->getModel3DDir()+"Milkyway/", Vec3f(0.0000001,0.0000001,0.0000001), 0.01f);

		// Load the pointer textures
		Object::initTextures();
		ObjectBase::createShaderStarPointeur();
		ObjectBase::createShaderPointeur();
		//Init of the text's shaders
		s_font::createShader();
	}

	setLandscape(observatory->getLandscapeName());

	tone_converter->setWorldAdaptationLuminance(3.75f + atmosphere->getIntensity()*40000.f);

	// Compute planets data and init viewing position position of sun and all the satellites (ie planets)
	ssystem->computePositions(timeMgr->getJDay(), observatory);

	// Compute transform matrices between coordinates systems
	navigation->updateTransformMatrices(observatory, timeMgr->getJDay());
	navigation->updateViewMat(projection, projection->getFov());

	setPlanetsSelected("");	// Fix a bug on macosX! Thanks Fumio!

	std::string skyLocaleName = conf.getStr("localization", "sky_locale");
	mBackup.initial_skyLocale=skyLocaleName;
	setSkyLanguage(skyLocaleName);

	int grid_level = hip_stars->getMaxGridLevel();
	geodesic_grid = new GeodesicGrid(grid_level);
	hip_stars->setGrid(geodesic_grid);

	FlagEnableZoomKeys	= conf.getBoolean("navigation:flag_enable_zoom_keys");
	FlagEnableMoveKeys  = conf.getBoolean("navigation:flag_enable_move_keys");
	setFlagManualAutoZoom( conf.getBoolean("navigation:flag_manual_zoom") );

	setAutomoveDuration( conf.getDouble ("navigation","auto_move_duration") );
	vzm.move_speed			= conf.getDouble("navigation","move_speed");
	vzm.zoom_speed			= conf.getDouble("navigation","zoom_speed");

	// Viewing Mode
	std::string tmpstr = conf.getStr("navigation:viewing_mode");
	if (tmpstr=="equator") 	navigation->setViewingMode(Navigator::VIEW_EQUATOR);
	else {
		if (tmpstr=="horizon") navigation->setViewingMode(Navigator::VIEW_HORIZON);
		else {
			std::cerr << "ERROR : Unknown viewing mode type : " << tmpstr << std::endl;
			assert(0);
		}
	}

	InitFov				= conf.getDouble ("navigation","init_fov");
	projection->setFov(InitFov);

	double heading = conf.getDouble ("navigation","heading");
	navigation->setHeading(heading);
	navigation->setDefaultHeading(heading);

	InitViewPos = Utility::strToVec3f(conf.getStr("navigation:init_view_pos").c_str());

	double viewOffset = conf.getDouble ("navigation","view_offset");

	setViewOffset(viewOffset);

	// Load constellations from the correct sky culture
	std::string tmp = conf.getStr("localization", "sky_culture");
	mBackup.initial_skyCulture=tmp;
	setSkyCultureDir(tmp);
	skyCultureDir = tmp;

	// Landscape section
	landscape->setFlagShow(conf.getBoolean("landscape", "flag_landscape"));
	landscape->setFlagShowFog(conf.getBoolean("landscape:flag_fog"));

	bodyDecor->setAtmosphereState(conf.getBoolean("landscape:flag_atmosphere"));
	atmosphere->setFlagShow(conf.getBoolean("landscape:flag_atmosphere"));
	atmosphere->setFaderDuration(conf.getDouble("viewing","atmosphere_fade_duration"));

	// Viewing section
	asterisms->setFlagLines( conf.getBoolean("viewing:flag_constellation_drawing"));
	asterisms->setFlagNames(conf.getBoolean("viewing:flag_constellation_name"));
	asterisms->setFlagBoundaries(conf.getBoolean("viewing","flag_constellation_boundaries"));
	asterisms->setFlagArt(conf.getBoolean("viewing:flag_constellation_art"));
	asterisms->setFlagIsolateSelected(conf.getBoolean("viewing", "flag_constellation_pick"));
	asterisms->setArtIntensity(conf.getDouble("viewing","constellation_art_intensity"));
	asterisms->setArtFadeDuration(conf.getDouble("viewing","constellation_art_fade_duration"));

	skyGridMgr->setFlagShow("GRID_ALTAZIMUTAL",conf.getBoolean("viewing:flag_azimutal_grid"));
	skyGridMgr->setFlagShow("GRID_EQUATORIAL",conf.getBoolean("viewing:flag_equatorial_grid"));
	skyGridMgr->setFlagShow("GRID_ECLIPTIC",conf.getBoolean("viewing:flag_ecliptic_grid"));
	skyGridMgr->setFlagShow("GRID_GALACTIC",conf.getBoolean("viewing:flag_galactic_grid"));

	skyLineMgr->setFlagShow("LINE_EQUATOR", conf.getBoolean("viewing:flag_equator_line"));
	skyLineMgr->setFlagShow("LINE_GALACTIC_EQUATOR", conf.getBoolean("viewing:flag_galactic_line"));
	skyLineMgr->setFlagShow("LINE_ECLIPTIC", conf.getBoolean("viewing:flag_ecliptic_line"));
	skyLineMgr->setFlagShow("LINE_PRECESSION", conf.getBoolean("viewing:flag_precession_circle"));
	skyLineMgr->setFlagShow("LINE_CIRCUMPOLAR", conf.getBoolean("viewing:flag_circumpolar_circle"));
	skyLineMgr->setFlagShow("LINE_TROPIC", conf.getBoolean("viewing:flag_tropic_lines"));
	skyLineMgr->setFlagShow("LINE_MERIDIAN", conf.getBoolean("viewing:flag_meridian_line"));
	skyLineMgr->setFlagShow("LINE_ZENITH", conf.getBoolean("viewing:flag_zenith_line"));
	skyLineMgr->setFlagShow("LINE_CIRCLE_POLAR", conf.getBoolean("viewing:flag_polar_circle"));
	skyLineMgr->setFlagShow("LINE_POINT_POLAR", conf.getBoolean("viewing:flag_polar_point"));
	skyLineMgr->setFlagShow("LINE_ECLIPTIC_POLE", conf.getBoolean("viewing:flag_ecliptic_center"));
	skyLineMgr->setFlagShow("LINE_GALACTIC_POLE", conf.getBoolean("viewing:flag_galactic_pole"));
	skyLineMgr->setFlagShow("LINE_GALACTIC_CENTER", conf.getBoolean("viewing:flag_galactic_center"));
	skyLineMgr->setFlagShow("LINE_VERNAL", conf.getBoolean("viewing:flag_vernal_points"));
	skyLineMgr->setFlagShow("LINE_ANALEMMALINE", conf.getBoolean("viewing:flag_analemma_line"));
	skyLineMgr->setFlagShow("LINE_ANALEMMA", conf.getBoolean("viewing:flag_analemma"));
	skyLineMgr->setFlagShow("LINE_ARIES", conf.getBoolean("viewing:flag_aries_line"));
	skyLineMgr->setFlagShow("LINE_ZODIAC", conf.getBoolean("viewing:flag_zodiac"));
	personal->setFlagShow(conf.getBoolean("viewing:flag_personal"));
	personeq->setFlagShow(conf.getBoolean("viewing:flag_personeq"));
	nautical->setFlagShow(conf.getBoolean("viewing:flag_nautical_alt"));
	nauticeq->setFlagShow(conf.getBoolean("viewing:flag_nautical_ra"));
	objCoord->setFlagShow(conf.getBoolean("viewing:flag_object_coordinates"));
	mouseCoord->setFlagShow(conf.getBoolean("viewing:flag_mouse_coordinates"));
	angDist->setFlagShow(conf.getBoolean("viewing:flag_angular_distance"));
	loxodromy->setFlagShow(conf.getBoolean("viewing:flag_loxodromy"));
	orthodromy->setFlagShow(conf.getBoolean("viewing:flag_orthodromy"));
	skyLineMgr->setFlagShow("LINE_GREENWICH", conf.getBoolean("viewing:flag_greenwich_line"));
	skyLineMgr->setFlagShow("LINE_VERTICAL", conf.getBoolean("viewing:flag_vertical_line"));
	cardinals_points->setFlagShow(conf.getBoolean("viewing:flag_cardinal_points"));

	setFlagMoonScaled(conf.getBoolean("viewing", "flag_moon_scaled"));
	setMoonScale(conf.getDouble ("viewing","moon_scale"), true); //? toujours true TODO
	setFlagSunScaled(conf.getBoolean("viewing", "flag_sun_scaled"));
	setSunScale(conf.getDouble ("viewing","sun_scale"), true); //? toujours true TODO

	oort->setFlagShow(conf.getBoolean("viewing:flag_oort"));

	setLightPollutionLimitingMagnitude(conf.getDouble("viewing","light_pollution_limiting_magnitude"));

	setMeteorsRate(conf.getInt("astro", "meteor_rate"));

	atmosphere->setFlagOptoma(conf.getBoolean("main:flag_optoma"));

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	mCity->loadCities(settings-> getDataDir() + "mcities.fab");
	initialSolarSystemBodies();

	defaultLandscape = observatoryGetLandscapeName();
	tempLandscape = observatoryGetLandscapeName();

	firstTime = 0;
}

void Core::update(int delta_time)
{
	currentExecutor->update(delta_time);
}

void Core::updateMode()
{
	if (currentExecutor->testValidAltitude(observatory->getAltitude())) {
		currentExecutor->onExit();
		currentExecutor = currentExecutor->getNextMode();
		std::cout << "Changement de mode pour " << currentExecutor->getName() << std::endl;
		currentExecutor->onEnter();
	}
}

void Core::bodyTraceHide(std::string value) const
{
	if (value=="all")
		bodytrace->hide(-1);
	else
		bodytrace->hide(Utility::strToInt(value));
}

bool Core::illuminateLoad(std::string filename, double ra, double de, double angular_size, std::string name, double r, double g, double b, float rotation)
{
	bool created = false;
	// if no size, so take default value from flag illuminate_star
	if (angular_size==0.0)
		angular_size=illuminate_size;

	if(illuminates) {
		created = illuminates->loadIlluminate(filename, ra, de, angular_size, name, r,g,b, rotation);
	}
	return created;
}


std::string Core::illuminateRemove(const std::string& name)
{
	std::string error = illuminates->removeIlluminate(name);
	return error;
}

std::string Core::illuminateRemoveAll()
{
	return illuminates->removeAllIlluminate();
}


//! Update all the objects in function of the time
void Core::updateInSolarSystem(int delta_time)
{
	if( firstTime ) // Do not update prior to Init. Causes intermittent problems at startup
		return;

	// Update the position of observation and time etc...
	observatory->update(delta_time);
	timeMgr->update(delta_time);
	navigation->update(delta_time);

	// Position of sun and all the satellites (ie planets)
	ssystem->computePositions(timeMgr->getJDay(), observatory);

	anchorManager->update();

	// Transform matrices between coordinates systems
	navigation->updateTransformMatrices(observatory, timeMgr->getJDay());
	// Direction of vision
	navigation->updateVisionVector(delta_time, selected_object);
	// Field of view
	projection->updateAutoZoom(delta_time, FlagManualZoom);
	// update faders and Planet trails (call after nav is updated)
	ssystem->update(delta_time, navigation, timeMgr);
			
	// Move the view direction and/or fov
	updateMove(delta_time);
	// Update info about selected object
	selected_object.update();
	// Update faders
	skyGridMgr->update(delta_time);
	skyLineMgr->update(delta_time);
	personal->update(delta_time);
	personeq->update(delta_time);
	nautical->update(delta_time);
	nauticeq->update(delta_time);
	objCoord->update(delta_time);
	mouseCoord->update(delta_time);
	angDist->update(delta_time);
	loxodromy->update(delta_time);
	orthodromy->update(delta_time);
	asterisms->update(delta_time);
	atmosphere->update(delta_time);
	landscape->update(delta_time);
	inactiveLandscape->update(delta_time);
	hip_stars->update(delta_time);
	nebulas->update(delta_time);
	cardinals_points->update(delta_time);
	milky_way->update(delta_time);
	text_usr->update(delta_time);
	bodytrace->update(delta_time);

	starLines->update(delta_time);

	oort->update(delta_time);

	// Compute the sun position in local coordinate
	Vec3d temp(0.,0.,0.);
	Vec3d sunPos = navigation->helioToLocal(temp);


	// Compute the moon position in local coordinate
	Vec3d moon = ssystem->getMoon()->get_heliocentric_ecliptic_pos();
	Vec3d moonPos = navigation->helioToLocal(moon);

	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color
	projection->setModelViewMatrices( navigation->getEarthEquToEyeMat(),
	                                    navigation->getEarthEquToEyeMatFixed(),
	                                    navigation->getHelioToEyeMat(),
	                                    navigation->getLocalToEyeMat(),
	                                    navigation->getJ2000ToEyeMat(),
	                                    navigation->geTdomeMat(),
	                                    navigation->getDomeFixedMat());

	std::future<void> a = std::async(std::launch::async, &Core::ssystemComputePreDraw, this);
	std::future<void> b = std::async(std::launch::async, &Core::atmosphereComputeColor, this, sunPos, moonPos);
	std::future<void> c = std::async(std::launch::async, &Core::hipStarMgrPreDraw, this);

	/*
	 * Récupération des résultats de async
	 */
	a.get();
	b.get();
	c.get();
	tone_converter->setWorldAdaptationLuminance(atmosphere->getWorldAdaptationLuminance());

	sunPos.normalize();
	moonPos.normalize();

	double alt, az;
	ssystem->bodyTraceGetAltAz(navigation, &alt, &az);
	bodytrace->addData(navigation, alt, az);

	// compute global sky brightness TODO : make this more "scientifically"
	// TODO: also add moonlight illumination
	if (sunPos[2] < -0.1/1.5 ) sky_brightness = 0.01;
	else sky_brightness = (0.01 + 1.5*(sunPos[2]+0.1/1.5));
	// TODO make this more generic for non-atmosphere planets
	if (atmosphere->getFadeIntensity() == 1) {
		// If the atmosphere is on, a solar eclipse might darken the sky otherwise we just use the sun position calculation above
		sky_brightness *= (atmosphere->getIntensity()+0.1);
	}
	// TODO: should calculate dimming with solar eclipse even without atmosphere on
	landscape->setSkyBrightness(sky_brightness+0.05);
	inactiveLandscape->setSkyBrightness(sky_brightness+0.05);
	// - if above troposphere equivalent on Earth in altitude
	
	if (!observatory->isOnBody()) { // && (observatory->getHomeBody()->getEnglishName() == "Earth")
		if ((observatory->getLandscapeName()!=tempLandscape) && (defaultLandscape!=tempLandscape) && !observatory->getSpacecraft()) tempLandscape=observatory->getLandscapeName(); //setLandscape(defaultLandscape);
		if ((observatory->getLandscapeName()!=defaultLandscape) && !observatory->getSpacecraft()) setLandscape(defaultLandscape); //setInitialLandscapeName(); //setLandscape(defaultLandscape);
		bodyDecor->anchorAssign(observatory->getSpacecraft());
		//std::cout << "O " << observatory->getLandscapeName() << " T " << tempLandscape << std::endl;
	} else { 
		if (observatory->getHomeBody()->getEnglishName() != "Sun") if ((observatory->getLandscapeName()==defaultLandscape) && (observatory->getHomeBody()->getEnglishName() != "Earth") && (observatory->getHomeBody()->getParent()->getEnglishName() == "Sun") && !observatory->getSpacecraft()) {
			setLandscape(observatory->getHomeBody()->getEnglishName());
			atmosphere->setFlagShow(true);
			atmosphereSetFlag(true);
		}	
		if (observatory->getHomeBody()->getEnglishName() == "Sun") setLandscape("sun");
		if (observatory->getHomeBody()->getEnglishName() != "Sun") if ((observatory->getLandscapeName()==defaultLandscape) && (observatory->getHomeBody()->getParent()->getEnglishName() != "Sun") && !observatory->getSpacecraft()) setLandscape("moon");
		if ((observatory->getLandscapeName()==defaultLandscape) && (defaultLandscape!=tempLandscape) && (observatory->getHomeBody()->getEnglishName() == "Earth") && !observatory->getSpacecraft()) setLandscape(tempLandscape);
		bodyDecor->bodyAssign(observatory->getAltitude(), observatory->getHomeBody()->getAtmosphereParams(), observatory->getSpacecraft());
		//std::cout << "O " << observatory->getHomeBody()->getEnglishName() << " L " << observatory->getLandscapeName() << " T " << tempLandscape << std::endl;
    }
	uboCamUpdate();
}

void Core::ssystemComputePreDraw()
{
	ssystem->computePreDraw(projection, navigation);
}


void Core::atmosphereComputeColor(Vec3d sunPos, Vec3d moonPos )
{
	atmosphere->computeColor(timeMgr->getJDay(), sunPos, moonPos,
	                          ssystem->getMoon()->get_phase(ssystem->getEarth()->get_heliocentric_ecliptic_pos()),
	                          tone_converter, projection, observatory->getHomePlanetEnglishName(), observatory->getLatitude(), observatory->getAltitude(),
	                          15.f, 40.f);	// Temperature = 15c, relative humidity = 40%
}

void Core::hipStarMgrPreDraw()
{
	hip_stars->preDraw(geodesic_grid, tone_converter, projection, navigation, timeMgr,observatory->getAltitude(), atmosphere->getFlagShow() && atmosphericRefractionGetFlag());
}

void Core::uboCamUpdate()
{
	ubo_cam->setViewport(projection->getViewport());
	ubo_cam->setClippingFov(projection->getClippingFov());
	ubo_cam->setViewportCenter(projection->getViewportFloatCenter());
	ubo_cam->setMVP2D(projection->getMatProjectionOrtho2D());
	ubo_cam->update();
}

//! Update all the objects in function of the time
void Core::updateInGalaxy(int delta_time)
{
	// Update the position of observation and time etc...
	observatory->update(delta_time);
	timeMgr->update(delta_time);
	navigation->update(delta_time);

	// Position of sun and all the satellites (ie planets)
	ssystem->computePositions(timeMgr->getJDay(), observatory);
	// Transform matrices between coordinates systems
	navigation->updateTransformMatrices(observatory, timeMgr->getJDay());
	// Direction of vision
	navigation->updateVisionVector(delta_time, selected_object);
	// Field of view
	projection->updateAutoZoom(delta_time, FlagManualZoom);
	// update faders and Planet trails (call after nav is updated)
	ssystem->update(delta_time, navigation, timeMgr);
	// Move the view direction and/or fov
	updateMove(delta_time);
	// Update faders
	personal->update(delta_time);
	personeq->update(delta_time);
	nautical->update(delta_time);
	nauticeq->update(delta_time);
	starLines->update(delta_time);
	milky_way->update(delta_time);
	text_usr->update(delta_time);
	dso3d->update(delta_time);
	landscape->update(delta_time);
	inactiveLandscape->update(delta_time);
	
	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color
	projection->setModelViewMatrices( navigation->getEarthEquToEyeMat(),
	                                    navigation->getEarthEquToEyeMatFixed(),
	                                    navigation->getHelioToEyeMat(),
	                                    navigation->getLocalToEyeMat(),
	                                    navigation->getJ2000ToEyeMat(),
	                                    navigation->geTdomeMat(),
	                                    navigation->getDomeFixedMat());
	uboCamUpdate();
}


//! Update all the objects in function of the time
void Core::updateInUniverse(int delta_time)
{
	// Update the position of observation and time etc...
	observatory->update(delta_time);
	navigation->update(delta_time);
	// Transform matrices between coordinates systems
	navigation->updateTransformMatrices(observatory, timeMgr->getJDay());
	// Direction of vision
	navigation->updateVisionVector(delta_time, selected_object);
	// Field of view
	projection->updateAutoZoom(delta_time, FlagManualZoom);
	// Move the view direction and/or fov
	updateMove(delta_time);
	// Update faders
	personal->update(delta_time);
	personeq->update(delta_time);
	nautical->update(delta_time);
	nauticeq->update(delta_time);
	tully->update(delta_time);
	// milky3d->update(delta_time);
	text_usr->update(delta_time);

	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color
	projection->setModelViewMatrices( navigation->getEarthEquToEyeMat(),
	                                    navigation->getEarthEquToEyeMatFixed(),
	                                    navigation->getHelioToEyeMat(),
	                                    navigation->getLocalToEyeMat(),
	                                    navigation->getJ2000ToEyeMat(),
	                                    navigation->geTdomeMat(),
	                                    navigation->getDomeFixedMat());
	uboCamUpdate();
}


//! Execute commun first drawing functions
void Core::preDraw(float clipping_min, float clipping_max)
{
	// Init openGL viewing with fov, screen size and clip planes
	projection->setClippingPlanes(clipping_min ,clipping_max);
	// Init viewport to current projector values
	projection->applyViewport();
	// User supplied line width value
	glLineWidth(m_lineWidth);
	StateGL::BlendFunc(GL_ONE, GL_ONE);
}

//! Execute commun last drawing functions
void Core::postDraw()
{
	media->imageDraw(navigation, projection);
	text_usr->draw(projection);
}


void Core::draw(int delta_time)
{
	currentExecutor->draw(delta_time);
}

void Core::switchMode(const std::string &mode)
{
	if (mode.empty())
		return;
	
	currentExecutor->onExit();
	if (mode =="InGalaxy") {
		currentExecutor = 	executorInGalaxy;
	} else
	if (mode =="InUniverse") {
		currentExecutor = 	executorInUniverse;
	} else
	if (mode =="InSolarSystem") {
		currentExecutor = 	executorInSolarSystem;
	}
	currentExecutor->onEnter();
}


//! Execute all the drawing functions
void Core::drawInSolarSystem(int delta_time)
{
	//for VR360 drawing
	media->drawVR360(projection, navigation);

	milky_way->draw(tone_converter, projection, navigation, timeMgr->getJulian());
	nebulas->draw(projection, navigation, tone_converter, atmosphere->getFlagShow() ? sky_brightness : 0);
	oort->draw(observatory->getAltitude(), projection, navigation);
	illuminates->draw(projection, navigation);
	asterisms->draw(projection, navigation);

	starLines->draw(projection);

	hip_stars->draw(geodesic_grid, tone_converter, projection, timeMgr,observatory->getAltitude());
	skyGridMgr->draw(projection);
	skyLineMgr->draw(projection, navigation, timeMgr, observatory);
	bodytrace->draw(projection, navigation);

	personal->draw(projection, navigation);
	personeq->draw(projection, navigation);
	nautical->nautical_draw(projection, navigation, selected_object.getEarthEquPos(navigation));
	nauticeq->nautical_draw(projection, navigation, selected_object.getEarthEquPos(navigation));

	objCoord->objCoord_draw(projection, navigation, selected_object.getEarthEquPos(navigation));
	mouseCoord->objCoord_draw(projection, navigation, getCursorPosEqu(mouseX, mouseY));
	angDist->angDist_draw(projection, navigation, selected_object.getEarthEquPos(navigation), old_selected_object.getEarthEquPos(navigation));
	loxodromy->loxodromy_draw(projection, navigation, selected_object.getEarthEquPos(navigation), old_selected_object.getEarthEquPos(navigation));
	orthodromy->orthodromy_draw(projection, navigation, selected_object.getEarthEquPos(navigation), old_selected_object.getEarthEquPos(navigation));

	ssystem->draw(projection,navigation, observatory, tone_converter, bodyDecor->canDrawBody() /*aboveHomePlanet*/ );

	// Draw the pointer on the currently selected object
	// TODO: this would be improved if pointer was drawn at same time as object for correct depth in scene
	if (selected_object && object_pointer_visibility) selected_object.drawPointer(delta_time, projection, navigation);

	// Update meteors
	meteors->update(projection, navigation, timeMgr, tone_converter, delta_time);

	// retiré la condition && atmosphere->getFlagShow() de sorte à pouvoir en avoir par atmosphère ténue
	// if (!aboveHomePlanet && (sky_brightness<0.1) && (observatory->getHomeBody()->getEnglishName() == "Earth" || observatory->getHomeBody()->getEnglishName() == "Mars")) { 
	if (bodyDecor->canDrawMeteor() && (sky_brightness<0.1))
		meteors->draw(projection, navigation);

	// if (bodyDecor->canDrawAtmosphere())
		atmosphere->draw(projection, observatory->getHomePlanetEnglishName());

	// Draw the landscape
	if (bodyDecor->canDrawLandscape()) { //!aboveHomePlanet) // TODO decide if useful or too confusing to leave alone
		landscape->draw(tone_converter, projection, navigation);
		inactiveLandscape->draw(tone_converter, projection, navigation);
	}

	cardinals_points->draw(projection, observatory->getLatitude());
}

//! Execute all the drawing functions
void Core::drawInGalaxy(int delta_time)
{
	starNav->computePosition(navigation->getObserverHelioPos());

	//for VR360 drawing
	media->drawVR360(projection, navigation);

	milky_way->draw(tone_converter, projection, navigation, timeMgr->getJulian());
	glClear(GL_DEPTH_BUFFER_BIT);

	//tracé des lignes sans activation du tampon de profondeur.
	personal->draw(projection, navigation);
	personeq->draw(projection, navigation);
	starLines->draw(navigation);
	
	// transparence.
	dso3d->draw(observatory->getAltitude(), projection, navigation);
	ojmMgr->draw(projection, navigation, OjmMgr::STATE_POSITION::IN_GALAXY);
	starNav->draw(navigation, projection);

	if (bodyDecor->canDrawLandscape()) { //!aboveHomePlanet) // TODO decide if useful or too confusing to leave alone
		landscape->draw(tone_converter, projection, navigation);
		inactiveLandscape->draw(tone_converter, projection, navigation);
	}
}

//! Execute all the drawing functions
void Core::drawInUniverse(int delta_time)
{
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_DEPTH_BUFFER_BIT);

	//for VR360 drawing
	media->drawVR360(projection, navigation);
	ojmMgr->draw(projection, navigation, OjmMgr::STATE_POSITION::IN_UNIVERSE);
	tully->draw(observatory->getAltitude(), projection, navigation);

	personal->draw(projection, navigation);
	personeq->draw(projection, navigation);
}

bool Core::setLandscape(const std::string& new_landscape_name)
{
	if (new_landscape_name.empty()) return 0;
	std::string l_min = landscape->getName();
	transform(l_min.begin(), l_min.end(), l_min.begin(), ::tolower);
	if (new_landscape_name == l_min) return 0;

	Landscape* newLandscape = Landscape::createFromFile(settings->getUserDir() + "landscapes.ini", new_landscape_name);
	if (!newLandscape) return 0;

	if (landscape) {
		bool previousLandscapeFlag = landscapeGetFlag();

		//Switch between the inactive and active background
		Landscape* tempLandscape = landscape;
		landscape = newLandscape;
		inactiveLandscape = tempLandscape;

		//Fade off the old landscape
		inactiveLandscape->setFlagShow(false);
		//Fade in the new landscape (only if the landscape was activated)
		if (previousLandscapeFlag) {
			landscapeSetFlag(false);
			landscapeSetFlag(true);
		}
	}
	observatory->setLandscapeName(new_landscape_name);
	observatory->setSpacecraft(false);
	return 1;
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
		newLandscape->setFlagShowFog(landscape->getFlagShowFog());
		delete landscape;
		landscape = newLandscape;
	}
	observatory->setLandscapeName(param["name"]);
	// probably not particularly useful, as not in landscape.ini file
	observatory->setSpacecraft(param["spacecraft"]=="on");
	return 1;
}

//! Load a solar system body based on a hash of parameters mirroring the ssystem.ini file
std::string Core::addSolarSystemBody(stringHash_t& param)
{
	return ssystem->addBody(param);
}

std::string Core::removeSolarSystemBody(std::string name)
{
	// Make sure this object is not already selected so won't crash
	if (selected_object.getType()==OBJECT_BODY && selected_object.getEnglishName() == name) {
		unSelect();
	}

	// Make sure not standing on this object!
	const Body *p = observatory->getHomeBody();
	if (p!= nullptr && p->getEnglishName() == name) {
		return (std::string("Can not delete current home planet ") + name);
	}

	if(ssystem->removeBody(name)){
		return "";
	}
	else{
		return "could not delete given body see logs for more informations";
	}
}

std::string Core::removeSupplementalSolarSystemBodies()
{
	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;
	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.getType()==OBJECT_BODY /*&& selected_object.isDeleteable() */) {
		unSelect();
	}
	if(ssystem->removeSupplementalBodies(observatory->getHomePlanetEnglishName())){
		return "";
	}
	else{
		return "could not delete given body see logs for more informations";
	}
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
	rval = ssystem->searchByNamesI18(name);
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
		setPlanetsSelected("");

	} else if (type=="star") {
		selected_object = hip_stars->search(id).get();
		asterisms->setSelected(selected_object);
		setPlanetsSelected("");

	} else if (type=="planet") {
		setPlanetsSelected(id);
		selected_object = ssystem->getSelected();
		asterisms->setSelected(Object());

	} else if (type=="nebula") {
		selected_object = nebulas->search(id);
		setPlanetsSelected("");
		asterisms->setSelected(Object());

	} else if (type=="constellation") {

		// Select only constellation, nothing else
		asterisms->setSelected(id);

		selected_object = nullptr;
		setPlanetsSelected("");

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
		setPlanetsSelected("");
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
	setPlanetsSelected("");
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
	float fov_around = projection->getFov()/myMin(projection->getViewportWidth(), projection->getViewportHeight()) * 30.f;

	float xpos, ypos;
	projection->projectEarthEqu(v, winpos);
	xpos = winpos[0];
	ypos = winpos[1];

	// Collect the planets inside the range
	if (ssystem->getFlagPlanets()) {
		temp = ssystem->searchAround(v, fov_around, navigation, observatory, projection, &is_default_object, bodyDecor->canDrawBody()); //aboveHomePlanet);
		candidates.insert(candidates.begin(), temp.begin(), temp.end());

		if (is_default_object && temp.begin() != temp.end()) {
			//cout << "was default object\n";
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
	if (hip_stars->getFlagStars()) {
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
			if ( ssystem->getFlagHints() ) {
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
		navigation->moveTo(selected_object.getEarthEquPos(navigation),
		                    move_duration, false, 1);
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
	//	cout << "Set sky culture to: " << cultureDir << "(skyCultureDir: " << skyCultureDir << endl;
	if (skyCultureDir == cultureDir) return 1;
	// make sure culture definition exists before attempting or will die
	// Do not comment this out! Rob
	if (skyloc->directoryToSkyCultureEnglish(cultureDir) == "") {
		std::cerr << "Invalid sky culture directory: " << cultureDir << std::endl;
		return 0;
	}
	skyCultureDir = cultureDir;
	if (!asterisms) return 0;

	asterisms->loadLinesAndArt(settings->getSkyCultureDir() + skyCultureDir);
	asterisms->loadNames(settings->getSkyCultureDir() + skyCultureDir + "/constellation_names.eng.fab");
	// Re-translated constellation names
	asterisms->translateNames(skyTranslator);

	// as constellations have changed, clear out any selection and retest for match!
	if (selected_object && selected_object.getType()==OBJECT_STAR) {
		asterisms->setSelected(selected_object);
	} else {
		asterisms->setSelected(Object());
	}

	// Load culture star names in english
	hip_stars->loadCommonNames(settings->getSkyCultureDir() + skyCultureDir + "/star_names.fab");
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
	if ( !hip_stars || !cardinals_points || !asterisms || ! skyLineMgr->isExist("LINE_ECLIPTIC")) return; // objects not initialized yet

	std::string oldLocale = getSkyLanguage();

	// Update the translator with new locale name
	skyTranslator = Translator(PACKAGE, settings->getLocaleDir(), newSkyLocaleName);
	cLog::get()->write("Sky locale is " + skyTranslator.getLocaleName(), LOG_TYPE::L_INFO);
	printf("SkyLocale : %s\n", newSkyLocaleName.c_str());

	// Translate all labels with the new language
	cardinals_points->translateLabels(skyTranslator);
	skyLineMgr->translateLabels(skyTranslator); //ecliptic_line
	asterisms->translateNames(skyTranslator);
	ssystem->translateNames(skyTranslator);
	nebulas->translateNames(skyTranslator);
	hip_stars->updateI18n(skyTranslator);
}


//! Please keep saveCurrentSettings up to date with any new color settings added here
void Core::setColorScheme(const std::string& skinFile, const std::string& section)
{
	InitParser conf;
	conf.load(skinFile);

	// simple default color, rather than black which doesn't show up
	// Load colors from config file
	skyGridMgr->setColor("GRID_ALTAZIMUTAL", Utility::strToVec3f(conf.getStr(section,"azimuthal_color")));
	skyGridMgr->setColor("GRID_EQUATORIAL", Utility::strToVec3f(conf.getStr(section,"equatorial_color")));
	skyGridMgr->setColor("GRID_ECLIPTIC", Utility::strToVec3f(conf.getStr(section,"ecliptic_color")));
	skyGridMgr->setColor("GRID_GALACTIC", Utility::strToVec3f(conf.getStr(section,"galactic_color")));
	skyLineMgr->setColor("LINE_ECLIPTIC", Utility::strToVec3f(conf.getStr(section,"ecliptic_color")));
	skyLineMgr->setColor("LINE_ECLIPTIC_POLE",Utility::strToVec3f(conf.getStr(section,"ecliptic_center_color")));
	skyLineMgr->setColor("LINE_GALACTIC_CENTER",Utility::strToVec3f(conf.getStr(section,"galactic_center_color")));
	skyLineMgr->setColor("LINE_GALACTIC_POLE",Utility::strToVec3f(conf.getStr(section,"galactic_pole_color")));

	nebulas->setLabelColor(Utility::strToVec3f(conf.getStr(section,"nebula_label_color")));
	nebulas->setCircleColor(Utility::strToVec3f(conf.getStr(section,"nebula_circle_color")));

	skyLineMgr->setColor("LINE_PRECESSION", Utility::strToVec3f(conf.getStr(section,"precession_circle_color")));
	skyLineMgr->setColor("LINE_CIRCUMPOLAR", Utility::strToVec3f(conf.getStr(section,"circumpolar_circle_color")));
	skyLineMgr->setColor("LINE_GALACTIC_EQUATOR", Utility::strToVec3f(conf.getStr(section,"galactic_color")));
	skyLineMgr->setColor("LINE_VERNAL", Utility::strToVec3f(conf.getStr(section,"vernal_points_color")));
	skyLineMgr->setColor("LINE_EQUATOR", Utility::strToVec3f(conf.getStr(section,"equator_color")));
	skyLineMgr->setColor("LINE_TROPIC", Utility::strToVec3f(conf.getStr(section,"equator_color")));

	ssystem->setDefaultBodyColor(conf.getStr(section,"planet_names_color"), conf.getStr(section,"planet_names_color"), 
								conf.getStr(section,"planet_orbits_color"), conf.getStr(section,"object_trails_color"));

	// default color override
	asterisms->setLineColor(Utility::strToVec3f(conf.getStr(section,"const_lines_color")));
	starLines-> setColor(Utility::strToVec3f(conf.getStr(section,"const_lines3D_color")));
	asterisms->setBoundaryColor(Utility::strToVec3f(conf.getStr(section,"const_boundary_color")));
	asterisms->setLabelColor(Utility::strToVec3f(conf.getStr(section,"const_names_color")));
	asterisms->setArtColor(Utility::strToVec3f(conf.getStr(section,"const_art_color")));

	skyLineMgr->setColor("LINE_ANALEMMALINE", Utility::strToVec3f(conf.getStr(section,"analemma_line_color")));
	skyLineMgr->setColor("LINE_ANALEMMA", Utility::strToVec3f(conf.getStr(section,"analemma_color")));
	skyLineMgr->setColor("LINE_ARIES",Utility::strToVec3f(conf.getStr(section,"aries_color")));
	cardinals_points->setColor(Utility::strToVec3f(conf.getStr(section,"cardinal_color")));
	skyLineMgr->setColor("LINE_ECLIPTIC_POLE",Utility::strToVec3f(conf.getStr(section,"ecliptic_center_color")));
	skyLineMgr->setColor("LINE_GALACTIC_POLE",Utility::strToVec3f(conf.getStr(section,"galactic_pole_color")));
	skyLineMgr->setColor("LINE_GALACTIC_CENTER",Utility::strToVec3f(conf.getStr(section,"galactic_center_color")));
	skyLineMgr->setColor("LINE_GREENWICH",Utility::strToVec3f(conf.getStr(section,"greenwich_color")));
	skyLineMgr->setColor("LINE_MERIDIAN",Utility::strToVec3f(conf.getStr(section,"meridian_color")));
	personal->setColor(Utility::strToVec3f(conf.getStr(section,"personal_color")));
	personeq->setColor(Utility::strToVec3f(conf.getStr(section,"personeq_color")));
	nautical->setColor(Utility::strToVec3f(conf.getStr(section,"nautical_alt_color")));
	nauticeq->setColor(Utility::strToVec3f(conf.getStr(section,"nautical_ra_color")));
	objCoord->setColor(Utility::strToVec3f(conf.getStr(section,"object_coordinates_color")));
	mouseCoord->setColor(Utility::strToVec3f(conf.getStr(section,"mouse_coordinates_color")));
	angDist->setColor(Utility::strToVec3f(conf.getStr(section,"angular_distance_color")));
	loxodromy->setColor(Utility::strToVec3f(conf.getStr(section, "loxodromy_color")));
	orthodromy->setColor(Utility::strToVec3f(conf.getStr(section, "orthodromy_color")));
	skyLineMgr->setColor("LINE_CIRCLE_POLAR", Utility::strToVec3f(conf.getStr(section,"polar_color")));
	skyLineMgr->setColor("LINE_POINT_POLAR", Utility::strToVec3f(conf.getStr(section,"polar_color")));
	text_usr->setColor(Utility::strToVec3f(conf.getStr(section,"text_usr_color")));
	skyLineMgr->setColor("LINE_VERNAL",Utility::strToVec3f(conf.getStr(section,"vernal_points_color")));
	skyLineMgr->setColor("LINE_VERTICAL",Utility::strToVec3f(conf.getStr(section,"vertical_color")));
	skyLineMgr->setColor("LINE_ZENITH",Utility::strToVec3f(conf.getStr(section,"zenith_color")));
	skyLineMgr->setColor("LINE_ZODIAC",Utility::strToVec3f(conf.getStr(section,"zodiac_color")));

	oort->setColor(Utility::strToVec3f(conf.getStr(section,"oort_color")));
}

//! For use by TUI - saves all current settings
//! @todo Put in stel_core?
void Core::saveCurrentConfig(InitParser &conf)
{
	// localization section
	conf.setStr("localization:sky_culture", getSkyCultureDir());
	conf.setStr("localization:sky_locale", getSkyLanguage());
	// Rendering section
	conf.setBoolean("rendering:flag_antialias_lines", getFlagAntialiasLines());
	conf.setDouble("rendering:line_width", getLineWidth());
	// viewing section
	conf.setBoolean("viewing:flag_constellation_drawing", constellationGetFlagLines());
	conf.setBoolean("viewing:flag_constellation_name", constellationGetFlagNames());
	conf.setBoolean("viewing:flag_constellation_art", constellationGetFlagArt());
	conf.setBoolean("viewing:flag_constellation_boundaries", constellationGetFlagBoundaries());
	conf.setBoolean("viewing:flag_constellation_pick", constellationGetFlagIsolateSelected());
	conf.setDouble("viewing:moon_scale", getMoonScale());
	conf.setDouble("viewing:sun_scale", getSunScale());
	conf.setBoolean("viewing:flag_equatorial_grid", skyGridMgrGetFlagShow("GRID_EQUATORIAL"));
	conf.setBoolean("viewing:flag_ecliptic_grid", skyGridMgrGetFlagShow("GRID_ECLIPTIC"));
	conf.setBoolean("viewing:flag_galactic_grid", skyGridMgrGetFlagShow("GRID_GALACTIC"));
	conf.setBoolean("viewing:flag_azimutal_grid", skyGridMgrGetFlagShow("GRID_ALTAZIMUTAL"));
	conf.setBoolean("viewing:flag_equator_line", skyGridMgrGetFlagShow("LINE_EQUATOR"));
	conf.setBoolean("viewing:flag_ecliptic_line", skyLineMgrGetFlagShow("LINE_ECLIPTIC"));
	conf.setBoolean("viewing:flag_cardinal_points", cardinalsPointsGetFlag());
	conf.setBoolean("viewing:flag_zenith_line", skyLineMgrGetFlagShow("LINE_ZENITH"));
	conf.setBoolean("viewing:flag_polar_circle", skyLineMgrGetFlagShow("LINE_CIRCLE_POLAR"));
	conf.setBoolean("viewing:flag_polar_point", skyLineMgrGetFlagShow("LINE_POINT_POLAR"));
	conf.setBoolean("viewing:flag_ecliptic_center", skyLineMgrGetFlagShow("LINE_ECLIPTIC_POLE"));
	conf.setBoolean("viewing:flag_galactic_pole", skyLineMgrGetFlagShow("LINE_GALACTIC_POLE"));
	conf.setBoolean("viewing:flag_galactic_center", skyLineMgrGetFlagShow("LINE_GALACTIC_CENTER"));
	conf.setBoolean("viewing:flag_vernal_points", skyLineMgrGetFlagShow("LINE_VERNAL"));
	conf.setBoolean("viewing:flag_analemma", skyLineMgrGetFlagShow("LINE_ANALEMMA"));
	conf.setBoolean("viewing:flag_analemma_line", skyLineMgrGetFlagShow("LINE_ANALEMMALINE"));
	conf.setBoolean("viewing:flag_aries_line", skyLineMgrGetFlagShow("LINE_ARIES"));
	conf.setBoolean("viewing:flag_zodiac", skyLineMgrGetFlagShow("LINE_ZODIAC"));
	conf.setBoolean("viewing:flag_greenwich_line", skyLineMgrGetFlagShow("LINE_GREENWICH"));
	conf.setBoolean("viewing:flag_vertical_line", skyLineMgrGetFlagShow("LINE_VERTICAL"));
	conf.setBoolean("viewing:flag_meridian_line", skyLineMgrGetFlagShow("LINE_MERIDIAN"));
	conf.setBoolean("viewing:flag_precession_circle", skyLineMgrGetFlagShow("LINE_PRECESSION"));
	conf.setBoolean("viewing:flag_circumpolar_circle", skyLineMgrGetFlagShow("LINE_CIRCUMPOLAR"));
	conf.setBoolean("viewing:flag_tropic_lines", skyLineMgrGetFlagShow("LINE_TROPIC"));
	conf.setBoolean("viewing:flag_moon_scaled", getFlagMoonScaled());
	conf.setBoolean("viewing:flag_sun_scaled", getFlagSunScaled());
	conf.setDouble ("viewing:constellation_art_intensity", constellationGetArtIntensity());
	conf.setDouble ("viewing:constellation_art_fade_duration", constellationGetArtFadeDuration());
	conf.setDouble("viewing:light_pollution_limiting_magnitude", getLightPollutionLimitingMagnitude());
	// Landscape section
	conf.setBoolean("landscape:flag_landscape", landscapeGetFlag());
	conf.setBoolean("landscape:flag_atmosphere", atmosphereGetFlag());
	conf.setBoolean("landscape:flag_fog", fogGetFlag());
	// Star section
	conf.setDouble ("stars:star_scale", starGetScale());
	conf.setDouble ("stars:star_mag_scale", starGetMagScale());
	conf.setDouble("stars:max_mag_star_name", starGetMaxMagName());
	conf.setBoolean("stars:flag_star_twinkle", starGetFlagTwinkle());
	conf.setDouble("stars:star_twinkle_amount", starGetTwinkleAmount());
	conf.setDouble("stars:star_limiting_mag", starGetLimitingMag());
	// Color section
	conf.setStr    ("color:azimuthal_color", Utility::vec3fToStr(skyGridMgrGetColor("GRID_ALTAZIMUTAL")));
	conf.setStr    ("color:equatorial_color", Utility::vec3fToStr(skyGridMgrGetColor("GRID_EQUATORIAL")));
	conf.setStr    ("color:ecliptic_color", Utility::vec3fToStr(skyGridMgrGetColor("GRID_ECLIPTIC")));
	conf.setStr    ("color:equator_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_EQUATOR")));
	conf.setStr    ("color:ecliptic_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_ECLIPTIC")));
	conf.setStr    ("color:meridian_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_MERIDIAN")));
	conf.setStr    ("color:zenith_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_ZENITH")));
	conf.setStr    ("color:polar_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_CIRCLE_POLAR")));
	conf.setStr    ("color:polar_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_POINT_POLAR")));
	conf.setStr    ("color:ecliptic_center_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_ECLIPTIC_POLE")));
	conf.setStr    ("color:galactic_pole_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_GALACTIC_POLE")));
	conf.setStr    ("color:galactic_center_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_GALACTIC_CENTER")));
	conf.setStr    ("color:vernal_points_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_VERNAL")));
	conf.setStr    ("color:analemma_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_ANALEMMA")));
	conf.setStr    ("color:analemma_line_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_ANALEMMALINE")));
	conf.setStr    ("color:aries_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_ARIES")));
	conf.setStr    ("color:zodiac_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_ZODIAC")));
	conf.setStr    ("color:personal_color", Utility::vec3fToStr(personalGetColor()));
	conf.setStr    ("color:personeq_color", Utility::vec3fToStr(personeqGetColor()));
	conf.setStr    ("color:nautical_alt", Utility::vec3fToStr(nauticalGetColor()));
	conf.setStr    ("color:nautical_ra", Utility::vec3fToStr(nauticeqGetColor()));
	conf.setStr    ("color:object_coordinates", Utility::vec3fToStr(objCoordGetColor()));
	conf.setStr    ("color:mouse_coordinates", Utility::vec3fToStr(mouseCoordGetColor()));
	conf.setStr    ("color:angular_distance", Utility::vec3fToStr(angDistGetColor()));
	conf.setStr    ("color:loxodromy", Utility::vec3fToStr(loxodromyGetColor()));
	conf.setStr    ("color:orthodromy", Utility::vec3fToStr(orthodromyGetColor()));
	conf.setStr    ("color:greenwich_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_GREENWICH")));
	conf.setStr    ("color:vertical_line", Utility::vec3fToStr(skyLineMgrGetColor("LINE_VERTICAL")));
	conf.setStr    ("color:const_lines_color", Utility::vec3fToStr(constellationGetColorLine()));
	conf.setStr    ("color:const_names_color", Utility::vec3fToStr(constellationGetColorNames()));
	conf.setStr    ("color:const_art_color", Utility::vec3fToStr(constellationGetColorArt()));
	conf.setStr    ("color:const_boundary_color", Utility::vec3fToStr(constellationGetColorBoundaries()));
	conf.setStr	   ("color:nebula_label_color", Utility::vec3fToStr(nebulaGetColorLabels()));
	conf.setStr	   ("color:nebula_circle_color", Utility::vec3fToStr(nebulaGetColorCircle()));
	conf.setStr	   ("color:precession_circle_color", Utility::vec3fToStr(skyLineMgrGetColor("LINE_PRECESSION")));
	conf.setStr    ("color:cardinal_color", Utility::vec3fToStr(cardinalsPointsGetColor()));
	// Navigation section
	conf.setBoolean("navigation:flag_manual_zoom", getFlagManualAutoZoom());
	conf.setDouble ("navigation:auto_move_duration", getAutoMoveDuration());
	conf.setDouble ("navigation:zoom_speed", getZoomSpeed());
	conf.setDouble ("navigation:heading", getHeading());
	// Astro section
	conf.setBoolean("astro:flag_object_trails", planetsGetFlagTrails());
	conf.setBoolean("astro:flag_bright_nebulae", nebulaGetFlagBright());
	conf.setBoolean("astro:flag_stars", starGetFlag());
	conf.setBoolean("astro:flag_star_name", starGetFlagName());
	conf.setBoolean("astro:flag_nebula", nebulaGetFlag());
	conf.setBoolean("astro:flag_nebula_names", nebulaGetFlagNames());
	conf.setBoolean("astro:flag_nebula_hints", nebulaGetFlagHints());
	conf.setDouble("astro:max_mag_nebula_name", nebulaGetMaxMagHints());
	conf.setBoolean("astro:flag_planets", planetsGetFlag());
	conf.setBoolean("astro:flag_planets_hints", planetsGetFlagHints());
	conf.setBoolean("astro:flag_planets_orbits", planetsGetFlagOrbits());
	conf.setBoolean("astro:flag_light_travel_time", getFlagLightTravelTime());
	conf.setBoolean("astro:flag_milky_way", milkyWayGetFlag());
	conf.setDouble("astro:milky_way_intensity", milkyWayGetIntensity());
	conf.setDouble("astro:star_size_limit", starGetSizeLimit());
	conf.setDouble("astro:planet_size_marginal_limit", getPlanetsSizeLimit());
}


void Core::setFontScheme() //TODO deja fait ailleurs ?
{
	skyLineMgr->setFont(FontSizeGeneral, FontFileNameGeneral);
}

//! Get a color used to display info about the currently selected object
Vec3f Core::getSelectedObjectInfoColor(void) const
{
	if (!selected_object) {
		std::cerr << "WARNING: Core::getSelectedObjectInfoColor was called while no object is currently selected!!" << std::endl;
		return Vec3f(1, 1, 1);
	}
	if (selected_object.getType()==OBJECT_NEBULA) return nebulas->getLabelColor();
	if (selected_object.getType()==OBJECT_BODY) return ssystem->getDefaultBodyColor("label");
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
	navigation->updateMove(projection, az2-az1, alt1-alt2, projection->getFov());
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
		getObservatory()->setAltitude(getObservatory()->getAltitude()*vzm.deltaHeight);
	}

	if (vzm.deltaFov != 0 ) {
		projection->changeFov(vzm.deltaFov);
		std::ostringstream oss;
		oss << "zoom delta_fov " << vzm.deltaFov;
		if (!recordActionCallback.empty()) recordActionCallback(oss.str());
	}

	if (vzm.deltaAz != 0 || vzm.deltaAlt != 0) {
		navigation->updateMove(projection, vzm.deltaAz, vzm.deltaAlt, projection->getFov());
		std::ostringstream oss;
		oss << "look delta_az " << vzm.deltaAz << " delta_alt " << vzm.deltaAlt;
		if (!recordActionCallback.empty()) recordActionCallback(oss.str());
	} else {
		// must perform call anyway, but don't record!
		navigation->updateMove(projection, vzm.deltaAz, vzm.deltaAlt, projection->getFov());
	}
}


bool Core::setHomePlanet(const std::string &planet)
{
	// reset planet trails due to changed perspective
	ssystem->startTrails( ssystem->getFlagTrails() );
	if (planet=="selected") return anchorManager->switchToAnchor(selected_object.getEnglishName()); else return anchorManager->switchToAnchor(planet);
}


// For use by TUI
std::string Core::getPlanetHashString()
{
	return ssystem->getPlanetHashString();
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
				// potentially record this action
				if (!recordActionCallback.empty()) recordActionCallback("select " + selected_object.getEnglishName());
			} else {
				asterisms->setSelected(Object());
			}

			if (selected_object.getType()==OBJECT_BODY) {
				ssystem->setSelected(selected_object);
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
	assert(0);	// Non reachable code
}


//! Find and return the list of at most maxNbItem objects auto-completing passed object I18 name
//! @param objPrefix the first letters of the searched object
//! @param maxNbItem the maximum number of returned object names
//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
std::vector<std::string> Core::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> result;
	std::vector <std::string>::const_iterator iter;

	// Get matching planets
	std::vector<std::string> matchingPlanets = ssystem->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingPlanets.begin(); iter != matchingPlanets.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingPlanets.size();

	// Get matching constellations
	std::vector<std::string> matchingConstellations = asterisms->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingConstellations.begin(); iter != matchingConstellations.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingConstellations.size();

	// Get matching nebulae
	std::vector<std::string> matchingNebulae = nebulas->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingNebulae.begin(); iter != matchingNebulae.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingNebulae.size();

	// Get matching stars
	std::vector<std::string> matchingStars = hip_stars->listMatchingObjectsI18n(objPrefix, maxNbItem);
	for (iter = matchingStars.begin(); iter != matchingStars.end(); ++iter)
		result.push_back(*iter);
	maxNbItem-=matchingStars.size();

	std::sort(result.begin(), result.end());

	return result;
}

void Core::setFlagTracking(bool b)
{
	if (!b || !selected_object) {
		navigation->setFlagTraking(0);
	} else if ( !navigation->getFlagTraking()) {
		navigation->moveTo(selected_object.getEarthEquPos(navigation),
		                    getAutomoveDuration());
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
	ssystem->setSizeLimit(f + starGetSizeLimit());
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

std::string Core::removeNebula(const std::string& name)
{
	bool updateSelection = false;

	// Make sure this object is not already selected so won't crash
	if (selected_object.getType()==OBJECT_NEBULA && selected_object.getEnglishName()==name /*&& selected_object.isDeleteable()*/) {

		updateSelection = true;
		selected_object=nullptr;
	}

	std::string error = nebulas->removeNebula(name, true);

	// Try to find original version, if any
	if( updateSelection ) selected_object = nebulas->search(name);

	return error;

}

std::string Core::removeSupplementalNebulae()
{
	//  cout << "Deleting planets and object deleteable = " << selected_object.isDeleteable() << endl;
	// Make sure an object to delete is NOT selected so won't crash
	if (selected_object.getType()==OBJECT_NEBULA /*&& selected_object.isDeleteable()*/ ) {
		unSelect();
	}

	return nebulas->removeSupplementalNebulae();

}

void Core::setJDayRelative(int year, int month)
{
	double jd = timeMgr->getJDay();
	ln_date current_date;
	SpaceDate::JulianToDate(jd,&current_date);
	timeMgr->setJDay(SpaceDate::JulianDayFromDateTime(current_date.years+year,current_date.months+month,current_date.days,current_date.hours,current_date.minutes,current_date.seconds));
}

void Core::setmBackup()
{
	if (mBackup.jday !=0) {
		timeMgr->setJDay(mBackup.jday);
		setFov(mBackup.fov);
		moveObserver (mBackup.latitude, mBackup.longitude, mBackup.altitude, 1/*, mBackup.pos_name*/);
	}
	setHomePlanet(mBackup.home_planet_name);
}

void Core::getmBackup()
{
	mBackup.jday=timeMgr->getJDay();
	mBackup.latitude=getObservatory()->getLatitude();
	mBackup.longitude=getObservatory()->getLongitude();
	mBackup.altitude=getObservatory()->getAltitude();
	mBackup.pos_name=getObservatory()->getName();
	mBackup.fov=getFov();
	mBackup.home_planet_name=getObservatory()->getHomePlanetEnglishName();
}

void Core::inimBackup()
{
	mBackup.jday=0.0;
	mBackup.latitude=0.0;
	mBackup.longitude=0.0;
	mBackup.altitude=0.0;
	mBackup.fov=0.0;
	mBackup.pos_name="";
	mBackup.home_planet_name="";
}
