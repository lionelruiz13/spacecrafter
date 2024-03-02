/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
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

// Class which handles the Text User Interface

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "coreModule/landscape.hpp"
#include "appModule/app.hpp"
#include "appModule/space_date.hpp"
#include "mainModule/sdl_facade.hpp"
#include "scriptModule/script_interface.hpp"
#include "coreModule/core.hpp"
#include "coreModule/sky_localizer.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "uiModule/ui.hpp"
#include "coreModule/coreLink.hpp"
#include "appModule/fontFactory.hpp"
#include "coreModule/time_mgr.hpp"
#include "spacecrafter.hpp"

// Draw simple gravity text ui.
void UI::drawGravityUi(MODULE module)
{
	// Normal transparency mode
	//StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//StateGL::enable(GL_BLEND);

	if (FlagShowTuiDateTime) {
		double jd = coreLink->getJDay();
		double lonmodulo = coreLink->observatoryGetLongitudeForDisplay();
		std::ostringstream os;

		os << spaceDate->getPrintableDateLocal(jd) << " " << spaceDate->getPrintableTimeLocal(jd);

		if ((FlagShowPlanetname) && (coreLink->getObserverHomePlanetEnglishName()!="Earth")) {
			if (coreLink->getObserverHomePlanetEnglishName()!= "")
			os << " " << _(coreLink->getObserverHomePlanetEnglishName());
		}
		if (FlagShowFov) os << " fov " << std::setprecision(3) << coreLink->getFov();
		if (FlagShowFps) os << "  FPS " << app->getFpsClock();
		if (FlagShowLatLon) {
			os << " Lat: " << Utility::printAngleDMS(coreLink->observatoryGetLatitude()*3.1415926/180)
			   << " Lon: " << Utility::printAngleDMS(lonmodulo*3.1415926/180);
			if (coreLink->observatoryGetAltitude()>1000) {
				switch (module) {
					case IN_SANDBOX:
					case SOLAR_SYSTEM:
						os << " Alt: " << coreLink->observatoryGetAltitude()/1000 << " km";
						break;
					case STELLAR_SYSTEM:
						os << " Alt: " << coreLink->observatoryGetAltitude() << " km";
						break;
					case IN_UNIVERSE:
					  os << " Alt: " << 3.44e-5 * coreLink->observatoryGetAltitude() << " l.y.";
					  break;
					case IN_GALAXY:
						os << " Alt: " << 2.26e-11 * coreLink->observatoryGetAltitude() << " l.y.";
						break;
				}
			}
		}

		if (core->getFlagNav() || core->getFlagAstronomical()) {
			std::string info = spaceDate->getPrintableTimeNav(coreLink->getJDay(), coreLink->observatoryGetLatitude(), coreLink->observatoryGetLongitudeForDisplay());
			std::string s_1, s_2, s_3;
			s_1= info.substr(0, info.find("@"));
			s_2= info.substr(info.find("@")+1);
			core->printHorizontal(tuiFont, 10,120, s_1 ,text_ui, TEXT_ALIGN::LEFT,false);//, 1, 1);
			core->printHorizontal(tuiFont, 4 ,120, s_2 ,text_ui, TEXT_ALIGN::LEFT, false);//, 1, 1);
		} else {
			int PosDateTimeL = PosDateTime;
			//we cache the texts because we display them several times
			switch(FlagNumberPrint) {
				case 3 :
					PosDateTimeL=PosDateTimeL%120;
					core->printHorizontal(tuiFont, 5, PosDateTimeL, os.str(),text_ui, TEXT_ALIGN::LEFT, false);//,  1,1);
					core->printHorizontal(tuiFont, 5, (PosDateTimeL+120), os.str(), text_ui, TEXT_ALIGN::LEFT, false);//, 1,1);
					core->printHorizontal(tuiFont, 5, (PosDateTimeL+240), os.str(), text_ui, TEXT_ALIGN::LEFT, false);//, 1,1);
					break;
				case 2 :
					PosDateTimeL=PosDateTimeL%180;
					core->printHorizontal(tuiFont, 5, PosDateTimeL, os.str(), text_ui, TEXT_ALIGN::LEFT, false);//, 1,1);
					core->printHorizontal(tuiFont, 5, (PosDateTimeL+180), os.str(), text_ui, TEXT_ALIGN::LEFT, false);//, 1,1);
					break;
				default:
					PosDateTimeL=PosDateTimeL%360;
					core->printHorizontal(tuiFont, 5, PosDateTimeL, os.str(), text_ui, TEXT_ALIGN::LEFT, false);//,  1, 1);
			}
		}
	}

	if (core->getFlagHasSelected() && FlagShowTuiShortObjInfo) {
		std::string info = core->getSelectedObjectShortInfo();
		Vec3f tmpColor = Vec3f(core->getSelectedObjectInfoColor());

		if (core->getFlagNav()) {
			std::string info2 = core->getSelectedObjectShortInfoNav();
			std::string s_1, s_2;
			s_1= info2.substr(0, info2.find("@"));
			s_2= info2.substr(info2.find("@")+1);
			core->printHorizontal(tuiFont, 4 , PosObjectInfo, s_2 , tmpColor, TEXT_ALIGN::LEFT, false);//, 1,1);
			core->printHorizontal(tuiFont, 10 ,PosObjectInfo, s_1 , tmpColor, TEXT_ALIGN::LEFT, false);//, 1,1);
			core->printHorizontal(tuiFont, 16, PosObjectInfo , info, tmpColor, TEXT_ALIGN::LEFT, false);//, 1,1);
			// tuiFont->clearCache(s_1);
			// tuiFont->clearCache(s_2);
		} else {
			int PosObjectInfoL = PosObjectInfo;
			//we cache the texts because we display them several times
			switch(FlagNumberPrint) {
				case 3 :
					PosObjectInfo=PosObjectInfo%120;
					core->printHorizontal(tuiFont, 5, PosObjectInfo , info, tmpColor, TEXT_ALIGN::LEFT, false);//,  1,1);
					core->printHorizontal(tuiFont, 5, (PosObjectInfoL+120) , info, tmpColor, TEXT_ALIGN::LEFT, false);//,  1,1);
					core->printHorizontal(tuiFont, 5, (PosObjectInfoL+240) , info, tmpColor, TEXT_ALIGN::LEFT, false);//,  1,1);
					break;
				case 2 :
					PosObjectInfo=PosObjectInfo%180;
					core->printHorizontal(tuiFont, 5, PosObjectInfo , info, tmpColor, TEXT_ALIGN::LEFT, false);//,  1,1);
					core->printHorizontal(tuiFont, 5, (PosObjectInfoL+180) , info, tmpColor, TEXT_ALIGN::LEFT, false);//,  1,1);
					break;
				default:
					PosObjectInfo=PosObjectInfo%360;
					core->printHorizontal(tuiFont, 5, PosObjectInfo , info, tmpColor, TEXT_ALIGN::LEFT, false);//,  1,1);
			}
		}
		// tuiFont->clearCache(info);
	}
}


// Create all the components of the text user interface
// should be safe to call more than once but not recommended
// since lose states - try localizeTui() instead
void UI::initTui()
{
	// If already initialized before, delete existing objects
	if (tui_root) delete tui_root;
	// if (tuiFont) delete tuiFont;

	// Load standard font based on app locale
	//tuiFont = std::make_unique<s_font>(FontSizeTuiMenu, FontNameTuiMenu);
	//fontFactory->rebuildUiFont();
	tuiFont->clearCache();
	//tuiFont = fontFactory->getUiFont();
	// if (!tuiFont) {
	// 	cLog::get()->write("ui_tuiconf: error while creating font tuiFont",LOG_TYPE::L_ERROR);
	// 	exit(-1);
	// }

	tui_root = new s_tui::Branch();

	// Submenus
	tui_menu_location = new s_tui::MenuBranch(std::string("1. ") );
	tui_menu_time = new s_tui::MenuBranch(std::string("2. ") );
	tui_menu_general = new s_tui::MenuBranch(std::string("3. ") );
	tui_menu_stars = new s_tui::MenuBranch(std::string("4. ") );
	tui_menu_colors = new s_tui::MenuBranch(std::string("5. ") );
	tui_menu_effects = new s_tui::MenuBranch(std::string("6. ") );
	tui_menu_scripts = new s_tui::MenuBranch(std::string("7. ") );
	tui_menu_administration = new s_tui::MenuBranch(std::string("8. ") );

	tui_root->addComponent(tui_menu_location);
	tui_root->addComponent(tui_menu_time);
	tui_root->addComponent(tui_menu_general);
	tui_root->addComponent(tui_menu_stars);
	tui_root->addComponent(tui_menu_colors);
	tui_root->addComponent(tui_menu_effects);
	tui_root->addComponent(tui_menu_scripts);
	tui_root->addComponent(tui_menu_administration);

	// 1. Location
	tui_location_latitude = new s_tui::DecimalItem(-90., 90., 0.,std::string("1.1 ") );
	tui_location_latitude->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbSetlocation));
	tui_location_longitude = new s_tui::DecimalItem(-720., 720., 0.,std::string("1.2 "), 1, true);
	tui_location_longitude->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbSetlocation));
	tui_location_altitude = new s_tui::IntegerItemLogstep(-500, 100000000, 0,std::string("1.3 ") );
	tui_location_altitude->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbSetlocation));

	// Home planet only changed if hit enter to accept because
	// switching planet instantaneously as select is hard on a planetarium audience
	tui_location_planet = new s_tui::MultiSet2Item<std::string>(std::string("1.4 ") );
	tui_location_planet->addItemList(core->getPlanetHashString());
	//	tui_location_planet->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbLocationChangePlanet));
	tui_location_planet->setOnTriggerCallback(mBoost::callback<void>(this, &UI::tuiCbLocationChangePlanet));

	tui_menu_location->addComponent(tui_location_latitude);
	tui_menu_location->addComponent(tui_location_longitude);
	tui_menu_location->addComponent(tui_location_altitude);
	tui_menu_location->addComponent(tui_location_planet);

	tui_location_heading = new s_tui::DecimalItem(-180, 180, 0,std::string("7.8 "), 1, true);
	tui_location_heading->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbViewportRelated));
	tui_menu_location->addComponent(tui_location_heading);


	// 2. Time
	tui_time_skytime = new s_tui::TimeItem(std::string("2.1 ") );
	tui_time_skytime->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbSkyTime));
	tui_time_settmz = new s_tui::TimeZoneitem(AppSettings::Instance()->getDataDir() + "zone.tab",std::string("2.2 ") );
	tui_time_settmz->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbSettimezone));
	tui_time_settmz->settz(spaceDate->getCustomTzName());
	tui_time_day_key = new s_tui::MultiSet2Item<std::string>(std::string("2.2 ") );
	tui_time_day_key->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbDayKey));

	tui_time_presetskytime = new s_tui::TimeItem(std::string("2.3 ") );
	tui_time_presetskytime->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCb1));
	tui_time_startuptime = new s_tui::MultiSet2Item<std::string>(std::string("2.4 ") );
	tui_time_startuptime->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCb1));
	tui_time_displayformat = new s_tui::MultiSetItem<std::string>(std::string("2.5 ") );
	tui_time_displayformat->addItem("24h");
	tui_time_displayformat->addItem("12h");
	tui_time_displayformat->addItem("system_default");
	tui_time_displayformat->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbSetTimeDisplayFormat));
	tui_time_dateformat = new s_tui::MultiSetItem<std::string>(std::string("2.6 ") );
	tui_time_dateformat->addItem("yyyymmdd");
	tui_time_dateformat->addItem("ddmmyyyy");
	tui_time_dateformat->addItem("mmddyyyy");
	tui_time_dateformat->addItem("system_default");
	tui_time_dateformat->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbSetTimeDisplayFormat));

	tui_menu_time->addComponent(tui_time_skytime);
	tui_menu_time->addComponent(tui_time_settmz);
	tui_menu_time->addComponent(tui_time_day_key);
	tui_menu_time->addComponent(tui_time_presetskytime);
	tui_menu_time->addComponent(tui_time_startuptime);
	tui_menu_time->addComponent(tui_time_displayformat);
	tui_menu_time->addComponent(tui_time_dateformat);

	// 3. General settings
	tui_general_landscape = new s_tui::MultiSetItem<std::string>(std::string("3.1 ") );
	tui_general_landscape->addItemList(std::string(Landscape::getFileContent(AppSettings::Instance()->getUserDir() + "landscapes.ini")));

	tui_general_landscape->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbTuiGeneralChangeLandscape));
	tui_menu_general->addComponent(tui_general_landscape);


	// sky culture goes here
	tui_general_sky_culture = new s_tui::MultiSet2Item<std::string>(std::string("3.1 ") );
	tui_general_sky_culture->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbTuiGeneralChangeSkyCulture));
	tui_menu_general->addComponent(tui_general_sky_culture);

	tui_general_sky_locale = new s_tui::MultiSetItem<std::string>(std::string("3.2 ") );
	tui_general_sky_locale->addItemList(std::string(Translator::getAvailableLanguagesCodes(AppSettings::Instance()->getLanguageDir())));

	tui_general_sky_locale->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbTuiGeneralChangeSkyLocale));
	tui_menu_general->addComponent(tui_general_sky_locale);


	// 4. Stars
	tui_stars_show = new s_tui::BooleanItem(false,std::string("4.1 ") );
	tui_stars_show->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbStars));
	tui_star_magscale = new s_tui::DecimalItem(0,30, 1,std::string("4.2 "), 0.1 );
	tui_star_magscale->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbStars));
	tui_star_labelmaxmag = new s_tui::DecimalItem(-1.5, 9.5, 1.5,std::string("4.3 "),0.5 );
	tui_star_labelmaxmag->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbStars));
	tui_stars_twinkle = new s_tui::DecimalItem(0., 1., 0.3,std::string("4.4 "), 0.1);
	tui_stars_twinkle->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbStars));
	tui_star_limitingmag = new s_tui::DecimalItem(0., 9.0, 6.5,std::string("4.5 "), 0.1);
	tui_star_limitingmag->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbStars));

	tui_menu_stars->addComponent(tui_stars_show);
	tui_menu_stars->addComponent(tui_star_magscale);
	tui_menu_stars->addComponent(tui_star_labelmaxmag);
	tui_menu_stars->addComponent(tui_stars_twinkle);
	tui_menu_stars->addComponent(tui_star_limitingmag);

	// 5. Colors
	tui_colors_const_line_color = new s_tui::VectorItem(std::string("5.1 "));
	tui_colors_const_line_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_const_line_color);

	tui_colors_const_label_color = new s_tui::VectorItem(std::string("5.2 "));
	tui_colors_const_label_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_const_label_color);

	tui_colors_const_art_intensity = new s_tui::DecimalItem(0,1,1,std::string("5.3 "),0.05);
	tui_colors_const_art_intensity->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_const_art_intensity);

	tui_colors_const_art_color = new s_tui::VectorItem(std::string("5.2 "));
	tui_colors_const_art_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_const_art_color);

	tui_colors_const_boundary_color = new s_tui::VectorItem(std::string("5.4 "));
	tui_colors_const_boundary_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_const_boundary_color);

	tui_colors_cardinal_color = new s_tui::VectorItem(std::string("5.5 "));
	tui_colors_cardinal_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_cardinal_color);

	tui_colors_planet_names_color = new s_tui::VectorItem(std::string("5.6 "));
	tui_colors_planet_names_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_planet_names_color);

	tui_colors_planet_orbits_color = new s_tui::VectorItem(std::string("5.7 "));
	tui_colors_planet_orbits_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_planet_orbits_color);

	tui_colors_object_trails_color = new s_tui::VectorItem(std::string("5.9 "));
	tui_colors_object_trails_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_object_trails_color);

	tui_colors_meridian_color = new s_tui::VectorItem(std::string("5.10 "));
	tui_colors_meridian_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_meridian_color);

	tui_colors_azimuthal_color = new s_tui::VectorItem(std::string("5.11 "));
	tui_colors_azimuthal_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_azimuthal_color);

	tui_colors_equatorial_color = new s_tui::VectorItem(std::string("5.12 "));
	tui_colors_equatorial_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_equatorial_color);

	tui_colors_equator_color = new s_tui::VectorItem(std::string("5.13 "));
	tui_colors_equator_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_equator_color);

	tui_colors_ecliptic_color = new s_tui::VectorItem(std::string("5.14 "));
	tui_colors_ecliptic_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_ecliptic_color);

	tui_colors_nebula_label_color = new s_tui::VectorItem(std::string("5.15 "));
	tui_colors_nebula_label_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_nebula_label_color);

	tui_colors_nebula_circle_color = new s_tui::VectorItem(std::string("5.16 "));
	tui_colors_nebula_circle_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_nebula_circle_color);

	tui_colors_precession_circle_color = new s_tui::VectorItem(std::string("5.17 "));
	tui_colors_precession_circle_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_precession_circle_color);

	tui_colors_circumpolar_circle_color = new s_tui::VectorItem(std::string("5.18 "));
	tui_colors_circumpolar_circle_color->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbChangeColor));
	tui_menu_colors->addComponent(tui_colors_circumpolar_circle_color);

	// *** Effects
	tui_effect_light_pollution = new s_tui::DecimalItem(0.5, 9.0, 6.5,std::string("6.1 "), 0.5 );
	tui_effect_light_pollution->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_light_pollution);

	tui_effect_manual_zoom = new s_tui::BooleanItem(false,std::string("6.2 ") );
	tui_effect_manual_zoom->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_manual_zoom);

	tui_effect_pointobj = new s_tui::BooleanItem(false,std::string("6.3 ") );
	tui_effect_pointobj->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_pointobj);

	tui_effect_object_scale = new s_tui::DecimalItem(0, 25, 1,std::string("6.4 "), 0.05);  // changed to .05 delta
	tui_effect_object_scale->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_object_scale);

	tui_effect_star_size_limit = new s_tui::DecimalItem(0.25, 25, 5,std::string("6.5 "), 0.25);
	tui_effect_star_size_limit->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_star_size_limit);

	tui_effect_planet_size_limit = new s_tui::DecimalItem(-10, 10, 4,std::string("6.6 "), 0.25);
	tui_effect_planet_size_limit->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_planet_size_limit);



	tui_effect_milkyway_intensity = new s_tui::DecimalItem(0, 100, 1,std::string("6.7 "), .1);  // cvs
	tui_effect_milkyway_intensity->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffectsMilkywayIntensity));
	tui_menu_effects->addComponent(tui_effect_milkyway_intensity);

	tui_effect_nebulae_label_magnitude = new s_tui::DecimalItem(0, 100, 1,std::string("6.8 "), .5);
	tui_effect_nebulae_label_magnitude->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffectsNebulaeLabelMagnitude));
	tui_menu_effects->addComponent(tui_effect_nebulae_label_magnitude);

	tui_effect_view_offset = new s_tui::DecimalItem(-0.5, 0.5, 0,std::string("6.9 "), 0.05 );
	tui_effect_view_offset->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbViewportRelated));
	tui_menu_effects->addComponent(tui_effect_view_offset);


	tui_effect_zoom_duration = new s_tui::DecimalItem(1, 10, 2,std::string("6.10 ") );
	tui_effect_zoom_duration->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_zoom_duration);

	tui_effect_cursor_timeout = new s_tui::DecimalItem(0, 60, 1,std::string("6.11 ") );
	tui_effect_cursor_timeout->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_cursor_timeout);

	tui_effect_light_travel = new s_tui::BooleanItem(false,std::string("6.12 ") );
	tui_effect_light_travel->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_light_travel);

	tui_effect_antialias = new s_tui::BooleanItem(false,std::string("6.13 ") );
	tui_effect_antialias->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_antialias);

	tui_effect_line_width = new s_tui::DecimalItem(0.125, 5, 1,std::string("6.14 "), 0.125 );
	tui_effect_line_width->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbEffects));
	tui_menu_effects->addComponent(tui_effect_line_width);

	// 7. Scripts
	tui_scripts_shows = new s_tui::ListItem<std::string>(std::string("7.1 ") );
	tui_scripts_shows->addItemList(std::string(TUI_SCRIPT_MSG) +std::string("\n") +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "shows/")));
	tui_scripts_shows->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbScriptsShows));
	tui_menu_scripts->addComponent(tui_scripts_shows);

	tui_scripts_basis = new s_tui::ListItem<std::string>(std::string("7.2 ") );
	tui_scripts_basis->addItemList(std::string(TUI_SCRIPT_MSG) +std::string("\n") +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "basis/")));
	tui_scripts_basis->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbScriptsBasis));
	tui_menu_scripts->addComponent(tui_scripts_basis);

	tui_scripts_planets = new s_tui::ListItem<std::string>(std::string("7.3 ") );
	tui_scripts_planets->addItemList(std::string(TUI_SCRIPT_MSG) +std::string("\n") +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "planets/")));
	tui_scripts_planets->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbScriptsPlanets));
	tui_menu_scripts->addComponent(tui_scripts_planets);

	tui_scripts_deepsky = new s_tui::ListItem<std::string>(std::string("7.4 ") );
	tui_scripts_deepsky->addItemList(std::string(TUI_SCRIPT_MSG) +std::string("\n") +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "deepsky/")));
	tui_scripts_deepsky->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbScriptsDeepsky));
	tui_menu_scripts->addComponent(tui_scripts_deepsky);

	tui_scripts_navigation = new s_tui::ListItem<std::string>(std::string("7.5 ") );
	tui_scripts_navigation->addItemList(std::string(TUI_SCRIPT_MSG) +std::string("\n") +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "navigation/")));
	tui_scripts_navigation->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbScriptsNavigation));
	tui_menu_scripts->addComponent(tui_scripts_navigation);

	tui_scripts_custom = new s_tui::ListItem<std::string>(std::string("7.6 ") );
	tui_scripts_custom->addItemList(std::string(TUI_SCRIPT_MSG) +std::string("\n") +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "custom/")));
	tui_scripts_custom->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbScriptsCustom));
	tui_menu_scripts->addComponent(tui_scripts_custom);

	// 8. Administration
	tui_admin_loaddefault = new s_tui::ActionConfirmItem(std::string("8.1 ") );
	tui_admin_loaddefault->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbAdminLoadDefault));
	tui_admin_savedefault = new s_tui::ActionConfirmItem(std::string("8.2 ") );
	tui_admin_savedefault->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbAdminSaveDefault));
	tui_admin_shutdown = new s_tui::ActionConfirmItem(std::string("8.3 ") );
	tui_admin_shutdown->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbAdminShutdown));
	tui_menu_administration->addComponent(tui_admin_loaddefault);
	tui_menu_administration->addComponent(tui_admin_savedefault);
	tui_menu_administration->addComponent(tui_admin_shutdown);

	// get system info
	std::string systemInfo=std::string(APP_NAME)+" "+std::string(USER_EDITION);
	tui_admin_info = new s_tui::Display(std::string("Label: "),std::string(systemInfo));
	tui_menu_administration->addComponent(tui_admin_info);
	tui_admin_resolution = new s_tui::Display(std::string("Label: "),m_sdl->getStrResolution());
	tui_menu_administration->addComponent(tui_admin_resolution);
	// get user info
	std::string userInfo=std::string(USER_NAME)+" "+std::string(USER_EDITION);
	tui_admin_user = new s_tui::Display(std::string("Label: "),std::string(userInfo));
	tui_menu_administration->addComponent(tui_admin_user);

	tui_admin_setlocale = new s_tui::MultiSetItem<std::string>("8.5 ");
	tui_admin_setlocale->addItemList(std::string(Translator::getAvailableLanguagesCodes(AppSettings::Instance()->getLanguageDir())));
	tui_admin_setlocale->setOnChangeCallback(mBoost::callback<void>(this, &UI::tuiCbAdminSetLocale));
	tui_menu_administration->addComponent(tui_admin_setlocale);

	// Now add in translated labels
	localizeTui();
}


// Update fonts, labels and lists for a new app locale
void UI::localizeTui()
{
	cLog::get()->write("Localizing TUI for locale: " + app->getAppLanguage(),LOG_TYPE::L_INFO);
	// if (tuiFont) delete tuiFont;
	//tuiFont = std::make_unique<s_font>(FontSizeTuiMenu, FontNameTuiMenu);
	//fontFactory->rebuildUiFont();
	tuiFont->clearCache();
	//tuiFont = fontFactory->getUiFont();
	// tuiFont = new s_font(FontSizeMenuTui, FontNameMenu);
	// if (!tuiFont) {
	// 	cLog::get()->write("Error while creating font name tuiFont",LOG_TYPE::L_ERROR);
	// 	exit(-1);
	// }

	if (!tui_root) return; // not initialized yet

	tui_menu_location->setLabel(std::string("1. ") + _("Set Location "));
	tui_menu_time->setLabel(std::string("2. ") + _("Set Time "));
	tui_menu_general->setLabel(std::string("3. ") + _("General "));
	tui_menu_stars->setLabel(std::string("4. ") + _("Stars "));
	tui_menu_colors->setLabel(std::string("5. ") + _("Colors "));
	tui_menu_effects->setLabel(std::string("6. ") + _("Effects "));
	tui_menu_scripts->setLabel(std::string("7. ") + _("Scripts "));
	tui_menu_administration->setLabel(std::string("8. ") + _("Administration "));

	// 1. Location
	tui_location_latitude->setLabel(std::string("1.1 ") + _("Latitude: "));
	tui_location_longitude->setLabel(std::string("1.2 ") + _("Longitude: "));
	tui_location_altitude->setLabel(std::string("1.3 ") + _("Altitude (m): "));
	tui_location_planet->setLabel(std::string("1.4 ") + _("Solar System Body: "));
	tui_location_planet->replaceItemList(core->getPlanetHashString(),0);
	tui_location_heading->setLabel(std::string("1.5 ") + _("Heading: "));

	// 2. Time
	tui_time_skytime->setLabel(std::string("2.1 ") + _("Sky Time: "));
	tui_time_settmz->setLabel(std::string("2.2 ") + _("Set Time Zone: "));

	tui_time_day_key->setLabel(std::string("2.3 ") + _("Day keys: "));
	tui_time_day_key->replaceItemList(_("Calendar") +std::string("\ncalendar\n")
	                                  + _("Sidereal") +std::string("\nsidereal\n"), 0);
	tui_time_presetskytime->setLabel(std::string("2.4 ") + _("Preset Sky Time: "));
	tui_time_startuptime->setLabel(std::string("2.5 ") + _("Sky Time At Start-up: "));
	tui_time_startuptime->replaceItemList(_("Actual Time") +std::string("\nActual\n")
	                                      + _("Preset Time") +std::string("\nPreset\n"), 0);
	tui_time_displayformat->setLabel(std::string("2.6 ") + _("Time Display Format: "));
	tui_time_dateformat->setLabel(std::string("2.7 ") + _("Date Display Format: "));

	// 3. General settings
	tui_general_landscape->setLabel(std::string("3.1 ") + _("Landscape: "));

	// sky culture goes here
	tui_general_sky_culture->setLabel(std::string("3.2 ") + _("Sky Culture: "));
	tui_general_sky_culture->replaceItemList(core->getSkyCultureHash(), 0);  // human readable names

	tui_general_sky_locale->setLabel(std::string("3.3 ") + _("Sky Language: "));

	// 4. Stars
	tui_stars_show->setLabel(std::string("4.1 ") + _("Show: "), _("Yes"),_("No"));
	tui_star_magscale->setLabel(std::string("4.2 ") + _("Star Value Multiplier: "));
	tui_star_labelmaxmag->setLabel(std::string("4.3 ") + _("Maximum Magnitude to Label: "));
	tui_stars_twinkle->setLabel(std::string("4.4 ") + _("Twinkling: "));
	tui_star_limitingmag->setLabel(std::string("4.5 ") + _("Limiting Magnitude: "));

	// 5. Colors
	tui_colors_const_line_color->setLabel(std::string("5.1 ") + _("Constellation Lines") + ": ");
	tui_colors_const_label_color->setLabel(std::string("5.2 ") + _("Constellation Labels")+": ");
	tui_colors_const_art_intensity->setLabel(std::string("5.3 ") + _("Constellation Art Intensity") + ": ");
	tui_colors_const_art_color->setLabel(std::string("5.4 ") + _("Constellation Art")+": ");
	tui_colors_const_boundary_color->setLabel(std::string("5.5 ") + _("Constellation Boundaries") + ": ");
	tui_colors_cardinal_color->setLabel(std::string("5.6 ") + _("Cardinal Points")+ ": ");
	tui_colors_planet_names_color->setLabel(std::string("5.7 ") + _("Body Labels") + ": ");
	tui_colors_planet_orbits_color->setLabel(std::string("5.8 ") + _("Body Orbits") + ": ");

	tui_colors_object_trails_color->setLabel(std::string("5.10 ") + _("Body Trails") + ": ");  // TODO: Should be Body Trails
	tui_colors_meridian_color->setLabel(std::string("5.11 ") + _("Meridian Line") + ": ");
	tui_colors_azimuthal_color->setLabel(std::string("5.12 ") + _("Azimuthal Grid") + ": ");
	tui_colors_equatorial_color->setLabel(std::string("5.13 ") + _("Equatorial Grid") + ": ");
	tui_colors_equator_color->setLabel(std::string("5.14 ") + _("Equator Line") + ": ");
	tui_colors_ecliptic_color->setLabel(std::string("5.15 ") + _("Ecliptic Line") + ": ");
	tui_colors_nebula_label_color->setLabel(std::string("5.16 ") + _("Nebula Labels") + ": ");
	tui_colors_nebula_circle_color->setLabel(std::string("5.17 ") + _("Nebula Circles") + ": ");
	tui_colors_precession_circle_color->setLabel(std::string("5.18 ") + _("Precession Circle") + ": ");
	tui_colors_circumpolar_circle_color->setLabel(std::string("5.19 ") + _("Circumpolar Circle") + ": ");


	// 6. Effects

	tui_effect_light_pollution->setLabel(std::string("6.1 ") + _("Light Pollution Limiting Magnitude: "));
	tui_effect_manual_zoom->setLabel(std::string("6.2 ") + _("Manual zoom: "), _("Yes"),_("No"));
	tui_effect_pointobj->setLabel(std::string("6.3 ") + _("Object Sizing Rule: "), _("Point"),_("Magnitude"));
	tui_effect_object_scale->setLabel(std::string("6.4 ") + _("Magnitude Scaling Multiplier: "));
	// TODO need to renumber or move
	tui_effect_star_size_limit->setLabel(std::string("6.5 ") + _("Star Size Limit: "));
	tui_effect_planet_size_limit->setLabel(std::string("6.6 ") + _("Planet Size Marginal Limit: "));
	tui_effect_milkyway_intensity->setLabel(std::string("6.7 ") + _("Milky Way intensity: "));
	tui_effect_nebulae_label_magnitude->setLabel(std::string("6.8 ") + _("Maximum Nebula Magnitude to Label: "));
	tui_effect_view_offset->setLabel(std::string("6.9 ") + _("Zoom Offset: "));
	tui_effect_zoom_duration->setLabel(std::string("6.10 ") + _("Zoom Duration: "));
	tui_effect_cursor_timeout->setLabel(std::string("6.12 ") + _("Cursor Timeout: "));
	tui_effect_light_travel->setLabel(std::string("6.13 ") + _("Correct for light travel time: "), _("Yes"),_("No"));
	tui_effect_antialias->setLabel(std::string("6.14 ") + _("Antialias Lines: "), _("Yes"),_("No"));
	tui_effect_line_width->setLabel(std::string("6.15 ") + _("Line Width: "));


	// 7. Scripts
	tui_scripts_shows->setLabel(std::string("7.1 ") + "Shows: ");
	tui_scripts_shows->replaceItemList(_(TUI_SCRIPT_MSG) +std::string("\n")
	                                   +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "shows/")), 0);
	tui_scripts_basis->setLabel(std::string("7.2 ") +  "Basis: ");
	tui_scripts_basis->replaceItemList(_(TUI_SCRIPT_MSG) +std::string("\n")
	                                   +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "basis/")), 0);
	tui_scripts_planets->setLabel(std::string("7.3 ") + "Planets: ");
	tui_scripts_planets->replaceItemList(_(TUI_SCRIPT_MSG) +std::string("\n")
	                                     +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "planets/")), 0);
	tui_scripts_deepsky->setLabel(std::string("7.4 ") + "Deepsky: ");
	tui_scripts_deepsky->replaceItemList(_(TUI_SCRIPT_MSG) +std::string("\n")
	                                     +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "deepsky/")), 0);
	tui_scripts_navigation->setLabel(std::string("7.5 ") + "Navigation: ");
	tui_scripts_navigation->replaceItemList(_(TUI_SCRIPT_MSG) +std::string("\n")
	                                        +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "navigation/")), 0);
	tui_scripts_custom->setLabel(std::string("7.6 ") + "Custom: ");
	tui_scripts_custom->replaceItemList(_(TUI_SCRIPT_MSG) +std::string("\n")
	                                    +std::string(scriptInterface->getScriptList(AppSettings::Instance()->getScriptDir() + "custom/")), 0);

	// 8. Administration
	tui_admin_loaddefault->setLabel(std::string("8.1 ") + _("Load Default Configuration: "));
	tui_admin_loaddefault->translateActions();
	tui_admin_savedefault->setLabel(std::string("8.2 ") + _("Save Current Configuration as Default: "));
	tui_admin_savedefault->translateActions();
	tui_admin_shutdown->setLabel(std::string("8.3 ") + _("Shut Down: "));
	tui_admin_shutdown->translateActions();
	tui_admin_info->setLabel(std::string("8.4 ") + _("Info: "));
	tui_admin_user->setLabel(std::string("8.5 ") + _("User: "));
	tui_admin_setlocale->setLabel(std::string("8.6 ") + _("Set UI Locale: "));
	tui_admin_resolution->setLabel(std::string("8.7 ") + _("Resolution: "));

}


// Display the tui
void UI::drawTui()
{
	// Normal transparency mode
	//StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//StateGL::enable(GL_BLEND);

	if (tui_root) {
		core->printHorizontal(tuiFont, 5, PosMenuM, tui_root->getString() , text_tui_root, TEXT_ALIGN::LEFT, false);//, 1, 1);
		//tuiFont->clearCache(tui_root->getString());
	}
}

int UI::handleKeysTui(SDL_Scancode key, s_tui::S_TUI_VALUE state)
{
	return tui_root->onKey(key, state);
}

// Update all the core parameters with values taken from the tui widgets
void UI::tuiCb1()
{
	// 2. Date & Time
	app->setPresetSkyTime(tui_time_presetskytime->getJDay());
	app->setStartupTimeMode(tui_time_startuptime->getCurrent());

}

// Update all the tui widgets with values taken from the core parameters
void UI::tuiUpdateWidgets()
{
	if (!FlagShowTuiMenu) return;

	// 1. Location
	tui_location_latitude->setValue(coreLink->observatoryGetLatitude());
	tui_location_longitude->setValue(coreLink->observatoryGetLongitude());
	tui_location_altitude->setValue(coreLink->observatoryGetAltitude());
	tui_location_heading->setValue(coreLink->getHeading());


	// 2. Date & Time
	tui_time_skytime->setJDay(coreLink->getJDay() + spaceDate->getGMTShift(coreLink->getJDay())*JD_HOUR);
	tui_time_settmz->settz(spaceDate->getCustomTzName());
	tui_time_presetskytime->setJDay(app->getPresetSkyTime());
	tui_time_startuptime->setCurrent(std::string(app->getStartupTimeMode()));
	tui_time_displayformat->setCurrent(std::string(spaceDate->getTimeFormatStr()));
	tui_time_dateformat->setCurrent(std::string(spaceDate->getDateFormatStr()));

	//	cout << "Updating tui value to " << app->DayKeyMode << endl;
	tui_time_day_key->setCurrent(std::string(app->getDayKeyMode()));

	// 3. general
	tui_general_landscape->setValue(std::string(coreLink->landscapeGetName()));
	tui_general_sky_culture->setValue(std::string(core->getSkyCultureDir()));
	tui_general_sky_locale->setValue(std::string(core->getSkyLanguage()));

	// 4. Stars
	tui_stars_show->setValue(coreLink->starGetFlag());
	tui_star_labelmaxmag->setValue(coreLink->starGetMaxMagName());
	tui_stars_twinkle->setValue(coreLink->starGetTwinkleAmount());
	tui_star_magscale->setValue(coreLink->starGetMagScale());
	tui_star_limitingmag->setValue(coreLink->starGetLimitingMag());

	// 5. Colors
	tui_colors_const_line_color->setVector(coreLink->constellationGetColorLine());
	tui_colors_const_label_color->setVector(coreLink->constellationGetColorNames());
	tui_colors_cardinal_color->setVector(coreLink->cardinalsPointsGetColor());
	tui_colors_const_art_intensity->setValue(coreLink->constellationGetArtIntensity());
	tui_colors_const_art_color->setVector(coreLink->constellationGetColorArt());
	tui_colors_const_boundary_color->setVector(coreLink->constellationGetColorBoundaries());
	tui_colors_planet_names_color->setVector(coreLink->planetGetDefaultColor("label"));
	tui_colors_planet_orbits_color->setVector(coreLink->planetGetDefaultColor("orbit"));

	tui_colors_object_trails_color->setVector(coreLink->planetGetDefaultColor("trail"));
	tui_colors_meridian_color->setVector(coreLink->skyLineMgrGetColor(SKYLINE_TYPE::LINE_MERIDIAN));
	tui_colors_azimuthal_color->setVector(coreLink->skyGridMgrGetColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL));
	tui_colors_equatorial_color->setVector(coreLink->skyGridMgrGetColor(SKYGRID_TYPE::GRID_EQUATORIAL));
	tui_colors_equator_color->setVector(coreLink->skyLineMgrGetColor(SKYLINE_TYPE::LINE_EQUATOR));
	tui_colors_ecliptic_color->setVector(coreLink->skyLineMgrGetColor(SKYLINE_TYPE::LINE_ECLIPTIC));
	tui_colors_nebula_label_color->setVector(coreLink->nebulaGetColorLabels());
	tui_colors_nebula_circle_color->setVector(coreLink->nebulaGetColorCircle());
	tui_colors_precession_circle_color->setVector(coreLink->skyLineMgrGetColor(SKYLINE_TYPE::LINE_PRECESSION));
	tui_colors_circumpolar_circle_color->setVector(coreLink->skyLineMgrGetColor(SKYLINE_TYPE::LINE_CIRCUMPOLAR));

	// *** Effects
	tui_effect_zoom_duration->setValue(core->getAutoMoveDuration());
	tui_effect_manual_zoom->setValue(core->getFlagManualAutoZoom());
	tui_effect_object_scale->setValue(coreLink->starGetScale());
	tui_effect_star_size_limit->setValue(core->starGetSizeLimit());
	tui_effect_planet_size_limit->setValue(core->getPlanetsSizeLimit());
	tui_effect_milkyway_intensity->setValue(coreLink->milkyWayGetIntensity());
	tui_effect_cursor_timeout->setValue(MouseCursorTimeout);
	tui_effect_light_pollution->setValue(core->getLightPollutionLimitingMagnitude());
	tui_effect_nebulae_label_magnitude->setValue(coreLink->nebulaGetMaxMagHints());
	tui_effect_light_travel->setValue(coreLink->getFlagLightTravelTime());
	tui_effect_view_offset->setValue(coreLink->getViewOffset());
	tui_effect_antialias->setValue(app->getFlagAntialiasLines());
	tui_effect_line_width->setValue(app->getLineWidth());

	// 7. Scripts
	// each fresh time enter needs to reset to select message
	if (scriptInterface->getSelectedScript()=="") {
		tui_scripts_basis->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_deepsky->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_navigation->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_planets->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_shows->setCurrent(_(TUI_SCRIPT_MSG));
		tui_scripts_custom->setCurrent(_(TUI_SCRIPT_MSG));
	}

	// 8. admin
	tui_admin_setlocale->setValue(std::string(app->getAppLanguage()) );
}

// Launch script to set time zone in the system locales
// TODO : this works only if the system manages the TZ environment
// variables of the form "Europe/Paris". This is not the case on windows
// so everything migth have to be re-done internaly :(
void UI::tuiCbSettimezone()
{
	// Don't call the script anymore coz it's pointless
	spaceDate->setCustomTzName(tui_time_settmz->gettz());
}

// Set time format mode
void UI::tuiCbSetTimeDisplayFormat()
{
	spaceDate->setTimeFormatStr(std::string(tui_time_displayformat->getCurrent()));
	spaceDate->setDateFormatStr(std::string(tui_time_dateformat->getCurrent()));
}

// 7. Administration actions functions

// Load default configuration
void UI::tuiCbAdminLoadDefault()
{
	app->init();
	tuiUpdateIndependentWidgets();
}

// Save to default configuration
void UI::tuiCbAdminSaveDefault()
{
	app->saveCurrentConfig(AppSettings::Instance()->getConfigFile());
}


// Launch script for shutdown, then exit
void UI::tuiCbAdminShutdown()
{
	app->flag(APP_FLAG::ALIVE, false);
}

// Set a new landscape skin
void UI::tuiCbTuiGeneralChangeLandscape()
{
	this->executeCommand(std::string("set landscape_name " + std::string(tui_general_landscape->getCurrent())));
}


// Set a new sky culture
void UI::tuiCbTuiGeneralChangeSkyCulture()
{
	this->executeCommand( std::string("set sky_culture ") +std::string(tui_general_sky_culture->getCurrent()));
}

// Set a new sky locale
void UI::tuiCbTuiGeneralChangeSkyLocale()
{
	this->executeCommand( std::string("set sky_locale " +std::string(tui_general_sky_locale->getCurrent())));
}


// mBoost::callback for changing scripts from basis
void UI::tuiCbScriptsBasis()
{
	if (tui_scripts_basis->getCurrent()!=_(TUI_SCRIPT_MSG)) {
		scriptInterface->setSelectedScript(tui_scripts_basis->getCurrent());
		scriptInterface->setSelectedScriptDirectory(AppSettings::Instance()->getScriptDir() + "basis/");
	} else {
		scriptInterface->setSelectedScript("");
	}
}

// mBoost::callback for changing scripts from deepsky
void UI::tuiCbScriptsDeepsky()
{
	if (tui_scripts_deepsky->getCurrent()!=_(TUI_SCRIPT_MSG)) {
		scriptInterface->setSelectedScript(tui_scripts_deepsky->getCurrent());
		scriptInterface->setSelectedScriptDirectory(AppSettings::Instance()->getScriptDir() + "deepsky/");
	} else {
		scriptInterface->setSelectedScript("");
	}
}

// mBoost::callback for changing scripts from navigation
void UI::tuiCbScriptsNavigation()
{
	if (tui_scripts_navigation->getCurrent()!=_(TUI_SCRIPT_MSG)) {
		scriptInterface->setSelectedScript(tui_scripts_navigation->getCurrent());
		scriptInterface->setSelectedScriptDirectory(AppSettings::Instance()->getScriptDir() + "navigation/");
	} else {
		scriptInterface->setSelectedScript("");
	}
}

// mBoost::callback for changing scripts from planets
void UI::tuiCbScriptsPlanets()
{
	if (tui_scripts_planets->getCurrent()!=_(TUI_SCRIPT_MSG)) {
		scriptInterface->setSelectedScript(tui_scripts_planets->getCurrent());
		scriptInterface->setSelectedScriptDirectory(AppSettings::Instance()->getScriptDir() + "planets/");
	} else {
		scriptInterface->setSelectedScript("");
	}
}

// mBoost::callback for changing scripts from shows
void UI::tuiCbScriptsShows()
{
	if (tui_scripts_shows->getCurrent()!=_(TUI_SCRIPT_MSG)) {
		scriptInterface->setSelectedScript(tui_scripts_shows->getCurrent());
		scriptInterface->setSelectedScriptDirectory(AppSettings::Instance()->getScriptDir() + "shows/");
	} else {
		scriptInterface->setSelectedScript("");
	}
}

// mBoost::callback for changing scripts from custom
void UI::tuiCbScriptsCustom()
{
	if (tui_scripts_custom->getCurrent()!=_(TUI_SCRIPT_MSG)) {
		scriptInterface->setSelectedScript(tui_scripts_custom->getCurrent());
		scriptInterface->setSelectedScriptDirectory(AppSettings::Instance()->getScriptDir() + "custom/");
	} else {
		scriptInterface->setSelectedScript("");
	}
}


// change UI locale
void UI::tuiCbAdminSetLocale()
{
	app->setAppLanguage(std::string(tui_admin_setlocale->getCurrent()));
}


// change heading or view offset
void UI::tuiCbViewportRelated()
{
	coreLink->setHeading(tui_location_heading->getValue(),
	                 int(tui_effect_zoom_duration->getValue()*1000));  // TEMP temporarily using zoom duration
	core->setViewOffset(tui_effect_view_offset->getValue());
}


void UI::tuiCbEffectsMilkywayIntensity()
{
	std::ostringstream oss;
	oss << "set milky_way_intensity " << tui_effect_milkyway_intensity->getValue();
	this->executeCommand(oss.str());
}


void UI::tuiCbSetlocation()
{
	// change to human readable coordinates with current values, then change
	coreLink->observatorySetLongitude(coreLink->observatoryGetLongitude());

	coreLink->observerMoveTo(tui_location_latitude->getValue(),
	                            tui_location_longitude->getValue(),
	                            tui_location_altitude->getValue(),
	                            int(tui_effect_zoom_duration->getValue()*1000),  // TEMP temporarily using zoom duration
	                            1); // use relative calculated duration
}


void UI::tuiCbStars()
{
	// 4. Stars
	std::ostringstream oss;

	oss << "flag stars " << tui_stars_show->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "set max_mag_star_name " << tui_star_labelmaxmag->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "set star_twinkle_amount " << tui_stars_twinkle->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "set star_mag_scale " << tui_star_magscale->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "set star_limiting_mag " << tui_star_limitingmag->getValue();
	this->executeCommand(oss.str());

}


void UI::tuiCbEffects()
{
	// *** Effects
	std::ostringstream oss;

	oss << "flag point_star " << tui_effect_pointobj->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "set auto_move_duration " << tui_effect_zoom_duration->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "flag manual_zoom " << tui_effect_manual_zoom->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "set star_scale " << tui_effect_object_scale->getValue();
	this->executeCommand(oss.str());

	core->setStarSizeLimit( tui_effect_star_size_limit->getValue() );

	core->setPlanetsSizeLimit( tui_effect_planet_size_limit->getValue() );

	MouseCursorTimeout = tui_effect_cursor_timeout->getValue();  // never recorded

	oss.str("");
	oss << "set light_pollution_limiting_magnitude " << tui_effect_light_pollution->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "flag light_travel_time " << tui_effect_light_travel->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "set line_width " << tui_effect_line_width->getValue();
	this->executeCommand(oss.str());

	oss.str("");
	oss << "flag antialias_lines " << tui_effect_antialias->getValue();
	this->executeCommand(oss.str());
}


// set sky time
void UI::tuiCbSkyTime()
{
	std::ostringstream oss;
	oss << "date local " << std::string(tui_time_skytime->getDateString());
	this->executeCommand(oss.str());
}


// set nebula label limit
void UI::tuiCbEffectsNebulaeLabelMagnitude()
{
	std::ostringstream oss;
	oss << "set max_mag_nebula_name " << tui_effect_nebulae_label_magnitude->getValue();
	this->executeCommand(oss.str());
}


void UI::tuiCbChangeColor()
{
	coreLink->constellationSetColorLine( tui_colors_const_line_color->getVector() );
	coreLink->constellationSetColorNames( tui_colors_const_label_color->getVector() );
	coreLink->cardinalsPointsSetColor( tui_colors_cardinal_color->getVector() );
	coreLink->constellationSetArtIntensity(tui_colors_const_art_intensity->getValue() );
	coreLink->constellationSetColorArt( tui_colors_const_art_color->getVector() );
	coreLink->constellationSetColorBoundaries(tui_colors_const_boundary_color->getVector() );
	coreLink->planetSetDefaultColor("orbit", tui_colors_planet_orbits_color->getVector() );
	coreLink->planetSetDefaultColor("label", tui_colors_planet_names_color->getVector() );
	coreLink->planetSetDefaultColor("trail",tui_colors_object_trails_color->getVector() );

	coreLink->skyGridMgrSetColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL , tui_colors_azimuthal_color->getVector() );
	coreLink->skyGridMgrSetColor(SKYGRID_TYPE::GRID_EQUATORIAL  , tui_colors_equatorial_color->getVector() );
	coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_EQUATOR, tui_colors_equator_color->getVector() );
	coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ECLIPTIC, tui_colors_ecliptic_color->getVector() );
	coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_MERIDIAN, tui_colors_meridian_color->getVector() );
	coreLink->nebulaSetColorLabels(tui_colors_nebula_label_color->getVector() );
	coreLink->nebulaSetColorCircle(tui_colors_nebula_circle_color->getVector() );
	coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_PRECESSION, tui_colors_precession_circle_color->getVector() );
	coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_CIRCUMPOLAR, tui_colors_circumpolar_circle_color->getVector() );
}


void UI::tuiCbLocationChangePlanet()
{
	this->executeCommand(std::string("set home_planet \"") + std::string( tui_location_planet->getCurrent() ) + "\"");
}

void UI::tuiCbDayKey()
{
	app->setDayKeyMode(tui_time_day_key->getCurrent());
}

// Update widgets that don't always match current settings with current settings
void UI::tuiUpdateIndependentWidgets()
{
	// Since some tui options don't immediately affect actual settings
	// reset those options to the current values now
	// (can not do this in tuiUpdateWidgets)
	tui_location_planet->setValue(std::string(coreLink->getObserverHomePlanetEnglishName() ) );
}
