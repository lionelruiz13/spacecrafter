#include <string>
#include <map>
#include <iostream>

enum class FLAG_VALUE : char {
    NONE, ON, OFF, TOGGLE
};

enum class FLAG_NAMES: char {FN_ANTIALIAS_LINES, FN_CONSTELLATION_DRAWING, FN_CONSTELLATION_NAMES, FN_CONSTELLATION_ART, FN_CONSTELLATION_BOUNDARIES, FN_CONSTELLATION_PICK,
                             FN_STAR_TWINKLE, FN_NAVIGATION, FN_SHOW_TUI_DATETIME, FN_SHOW_TUI_SHORT_OBJ_INFO, FN_MANUAL_ZOOM, FN_LIGHT_TRAVEL_TIME, FN_DSO_PICTOGRAMS,
                             FN_FOG, FN_ATMOSPHERE, FN_AZIMUTHAL_GRID, FN_EQUATORIAL_GRID, FN_ECLIPTIC_GRID, FN_GALACTIC_GRID, FN_EQUATOR_LINE, FN_GALACTIC_LINE,
                             FN_ECLIPTIC_LINE, FN_PRECESSION_CIRCLE, FN_CIRCUMPOLAR_CIRCLE, FN_TROPIC_LINES, FN_MERIDIAN_LINE, FN_ZENITH_LINE, FN_POLAR_CIRCLE,
                             FN_ECLIPTIC_CENTER, FN_GALACTIC_POLE, FN_GALACTIC_CENTER, FN_VERNAL_POINTS, FN_ANALEMMA_LINE, FN_ANALEMMA, FN_ARIES_LINE,
                             FN_ZODIAC, FN_PERSONAL, FN_PERSONEQ, FN_GREENWICH_LINE, FN_VERTICAL_LINE, FN_CARDINAL_POINTS, FN_CLOUDS, FN_MOON_SCALED, FN_SUN_SCALED,
                             FN_LANDSCAPE, FN_STARS, FN_STAR_NAMES, FN_PLANETS, FN_PLANET_NAMES, FN_PLANET_ORBITS, FN_ORBITS, FN_PLANETS_ORBITS, FN_PLANETS_AXIS,
                             FN_SATELLITES_ORBITS, FN_NEBULAE, FN_NEBULA_NAMES, FN_NEBULA_HINTS, FN_MILKY_WAY, FN_BRIGHT_NEBULAE, FN_OBJECT_TRAILS, FN_TRACK_OBJECT,
                             FN_SCRIPT_GUI_DEBUG, FN_LOCK_SKY_POSITION, FN_BODY_TRACE, FN_SHOW_LATLON, FN_COLOR_INVERSE, FN_OORT, FN_STARS_TRACE, FN_STAR_LINES,
                             FN_SKY_DRAW, FN_ZODIAC_LIGHT , FN_TULLY, FN_SATELLITES, FN_NONE
                            };

// virtual class
class ScCommand {
public:
    ScCommand(std::string _name, bool _stackable=false){name=_name; stackable = _stackable;}
    virtual ~ScCommand(){}
    std::string getName() { return name;}
    bool isStackable(){return stackable;}
    bool isValide() { return valide;}
	virtual void print(){ std::cout << "ScCommand "<< name << std::endl; }
    static FLAG_VALUE flagValueConvert(std::string _valueToConvert);
protected:
    bool valide = false;
private:
    virtual void testValidity() = 0;
    std::string name;
    bool stackable = false;
};



FLAG_VALUE ScCommand::flagValueConvert(std::string _valueToConvert)
{
    if (_valueToConvert=="toggle")
        return FLAG_VALUE::TOGGLE;
    if (_valueToConvert=="on" ||_valueToConvert=="true" || _valueToConvert=="1")
        return FLAG_VALUE::ON;
    if (_valueToConvert=="off" ||_valueToConvert=="false" || _valueToConvert=="0")
        return FLAG_VALUE::OFF;
    return FLAG_VALUE::NONE;   
}



// classe dérivée représentant les flags
class FlagCommand : public ScCommand {
public:
    FlagCommand(std::string _flagName, std::string _flagValue);
    FlagCommand(FLAG_NAMES _flagName, FLAG_VALUE _flagValue);
    ~FlagCommand(){};
    static void insertFlagsName();
private:
    virtual void testValidity() override;
    FLAG_NAMES flagNameConvert(std::string _nameToConvert);
    FLAG_NAMES flagName = FLAG_NAMES::FN_NONE;
    FLAG_VALUE flagValue = FLAG_VALUE::NONE;
    static std::map<const std::string, FLAG_NAMES> scFlags;
};

std::map<const std::string, FLAG_NAMES> FlagCommand::scFlags;

FLAG_NAMES FlagCommand::flagNameConvert(std::string _nameToConvert)
{
    auto it = scFlags.find(_nameToConvert);
    if (it != scFlags.end())
        return it->second;
    
    return FLAG_NAMES::FN_NONE;
}


FlagCommand::FlagCommand(std::string _flagName, std::string _flagValue) 
						: ScCommand("flag", true)
{
    flagName = flagNameConvert(_flagName);
    flagValue = flagValueConvert(_flagValue);
    testValidity();
}


FlagCommand::FlagCommand(FLAG_NAMES _flagName, FLAG_VALUE _flagValue) : ScCommand("flag", true)
{
    flagName = _flagName;
    flagValue = _flagValue;
    testValidity();
}


void FlagCommand::testValidity() 
{
    valide = ((flagName != FLAG_NAMES::FN_NONE)  &&(flagValue != FLAG_VALUE::NONE));
}

void FlagCommand::insertFlagsName()
{
	scFlags["antialias_lines"]= FLAG_NAMES::FN_ANTIALIAS_LINES;
	scFlags["constellation_drawing"]= FLAG_NAMES::FN_CONSTELLATION_DRAWING;
	scFlags["constellation_names"]= FLAG_NAMES::FN_CONSTELLATION_NAMES;
	scFlags["constellation_art"]= FLAG_NAMES::FN_CONSTELLATION_ART;
	scFlags["constellation_boundaries"]= FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES;
	scFlags["constellation_pick"]= FLAG_NAMES::FN_CONSTELLATION_PICK;

	scFlags["star_twinkle"]= FLAG_NAMES::FN_STAR_TWINKLE;
	scFlags["navigation"]= FLAG_NAMES::FN_NAVIGATION;
	scFlags["show_tui_datetime"]= FLAG_NAMES::FN_SHOW_TUI_DATETIME;
	scFlags["show_tui_short_obj_info"]= FLAG_NAMES::FN_SHOW_TUI_SHORT_OBJ_INFO;
	scFlags["manual_zoom"]= FLAG_NAMES::FN_MANUAL_ZOOM;

	scFlags["light_travel_time"]= FLAG_NAMES::FN_LIGHT_TRAVEL_TIME;
	scFlags["fog"]= FLAG_NAMES::FN_FOG;
	scFlags["atmosphere"]= FLAG_NAMES::FN_ATMOSPHERE;
	scFlags["azimuthal_grid"]= FLAG_NAMES::FN_AZIMUTHAL_GRID;
	scFlags["equatorial_grid"]= FLAG_NAMES::FN_EQUATORIAL_GRID;

	scFlags["ecliptic_grid"]= FLAG_NAMES::FN_ECLIPTIC_GRID;
	scFlags["galactic_grid"]= FLAG_NAMES::FN_GALACTIC_GRID;
	scFlags["equator_line"]= FLAG_NAMES::FN_EQUATOR_LINE;
	scFlags["galactic_line"]= FLAG_NAMES::FN_GALACTIC_LINE;
	scFlags["ecliptic_line"]= FLAG_NAMES::FN_ECLIPTIC_LINE;

	scFlags["precession_circle"]= FLAG_NAMES::FN_PRECESSION_CIRCLE;
	scFlags["circumpolar_circle"]= FLAG_NAMES::FN_CIRCUMPOLAR_CIRCLE;
	scFlags["tropic_lines"]= FLAG_NAMES::FN_TROPIC_LINES;
	scFlags["meridian_line"]= FLAG_NAMES::FN_MERIDIAN_LINE;
	scFlags["zenith_line"]= FLAG_NAMES::FN_ZENITH_LINE;

	scFlags["polar_circle"]= FLAG_NAMES::FN_POLAR_CIRCLE;
	scFlags["ecliptic_center"]= FLAG_NAMES::FN_ECLIPTIC_CENTER;
	scFlags["galactic_pole"]= FLAG_NAMES::FN_GALACTIC_POLE;
	scFlags["galactic_center"]= FLAG_NAMES::FN_GALACTIC_CENTER;
	scFlags["vernal_points"]= FLAG_NAMES::FN_VERNAL_POINTS;

	scFlags["analemma_line"]= FLAG_NAMES::FN_ANALEMMA_LINE;
	scFlags["analemma"]= FLAG_NAMES::FN_ANALEMMA;
	scFlags["aries_line"]= FLAG_NAMES::FN_ARIES_LINE;
	scFlags["zodiac"]= FLAG_NAMES::FN_ZODIAC;
	scFlags["personal"]= FLAG_NAMES::FN_PERSONAL;

	scFlags["personeq"]= FLAG_NAMES::FN_PERSONEQ;
	scFlags["greenwich_line"]= FLAG_NAMES::FN_GREENWICH_LINE;
	scFlags["vertical_line"]= FLAG_NAMES::FN_VERTICAL_LINE;
	scFlags["cardinal_points"]= FLAG_NAMES::FN_CARDINAL_POINTS;
	scFlags["clouds"]= FLAG_NAMES::FN_CLOUDS;

	scFlags["moon_scaled"]= FLAG_NAMES::FN_MOON_SCALED;
	scFlags["sun_scaled"]= FLAG_NAMES::FN_SUN_SCALED;
	scFlags["landscape"]= FLAG_NAMES::FN_LANDSCAPE;
	scFlags["stars"]= FLAG_NAMES::FN_STARS;
	scFlags["star_names"]= FLAG_NAMES::FN_STAR_NAMES;

	scFlags["planets"]= FLAG_NAMES::FN_PLANETS;
	scFlags["planet_names"]= FLAG_NAMES::FN_PLANET_NAMES;
	scFlags["planet_orbits"]= FLAG_NAMES::FN_PLANET_ORBITS;
	scFlags["orbits"]= FLAG_NAMES::FN_ORBITS;
	scFlags["planets_orbits"]= FLAG_NAMES::FN_PLANETS_ORBITS;

	scFlags["planets_axis"]= FLAG_NAMES::FN_PLANETS_AXIS;
	scFlags["satellites_orbits"]= FLAG_NAMES::FN_SATELLITES_ORBITS;
	scFlags["nebulae"]= FLAG_NAMES::FN_NEBULAE;
	scFlags["nebula_names"]= FLAG_NAMES::FN_NEBULA_NAMES;
	scFlags["nebula_hints"]= FLAG_NAMES::FN_NEBULA_HINTS;

	scFlags["milky_way"]= FLAG_NAMES::FN_MILKY_WAY;
	scFlags["bright_nebulae"]= FLAG_NAMES::FN_BRIGHT_NEBULAE;
	scFlags["object_trails"]= FLAG_NAMES::FN_OBJECT_TRAILS;
	scFlags["track_object"]= FLAG_NAMES::FN_TRACK_OBJECT;
	scFlags["script_gui_debug"]= FLAG_NAMES::FN_SCRIPT_GUI_DEBUG;

	scFlags["lock_sky_position"]= FLAG_NAMES::FN_LOCK_SKY_POSITION;
	scFlags["body_trace"]= FLAG_NAMES::FN_BODY_TRACE;
	scFlags["show_latlon"]= FLAG_NAMES::FN_SHOW_LATLON;
	scFlags["color_inverse"]= FLAG_NAMES::FN_COLOR_INVERSE;
	scFlags["oort"]= FLAG_NAMES::FN_OORT;

	scFlags["stars_trace"]= FLAG_NAMES::FN_STARS_TRACE;
	scFlags["star_lines"]= FLAG_NAMES::FN_STAR_LINES;
	scFlags["sky_draw"]= FLAG_NAMES::FN_SKY_DRAW;
	scFlags["dso_pictograms"]= FLAG_NAMES::FN_DSO_PICTOGRAMS;
	scFlags["zodiacal_light"]= FLAG_NAMES::FN_ZODIAC_LIGHT;

	scFlags["tully"]= FLAG_NAMES::FN_TULLY;
	scFlags["satellites"] = FLAG_NAMES::FN_SATELLITES;
}



int main(int argc, char *argv[])
{
	FlagCommand * test1 = new FlagCommand("FN_TULLY","off");
	test1->print();
	std::cout << "valide " << test1->isValide() << std::endl;
	std::cout << "stackable " << test1->isStackable() << std::endl;
	delete test1;

	test1 = new FlagCommand(FLAG_NAMES::FN_CLOUDS,FLAG_VALUE::TOGGLE);
	test1->print();
	std::cout << "valide " << test1->isValide() << std::endl;
	std::cout << "stackable " << test1->isStackable() << std::endl;
	delete test1;

    return 0;
}
