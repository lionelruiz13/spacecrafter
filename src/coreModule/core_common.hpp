#ifndef CORE_COMMON_HPP
#define CORE_COMMON_HPP

enum class SKYLINE_TYPE : char {
		LINE_CIRCLE_POLAR,
		LINE_POINT_POLAR,
		LINE_ECLIPTIC_POLE,
		LINE_GALACTIC_POLE,
		LINE_ANALEMMA,
		LINE_ANALEMMALINE,
		LINE_CIRCUMPOLAR,
		LINE_GALACTIC_CENTER,
		LINE_VERNAL,
		LINE_GREENWICH,
		LINE_ARIES,
		LINE_EQUATOR,
		LINE_GALACTIC_EQUATOR,
		LINE_MERIDIAN,
		LINE_TROPIC,
		LINE_ECLIPTIC,
		LINE_PRECESSION,
		LINE_VERTICAL,
		LINE_ZODIAC,
		LINE_ZENITH,
		LINE_UNKNOWN
	};

//structure that stores the grids at a time t
struct SkyLineSave {
	bool circle_polar;
	bool point_polar;
	bool ecliptic_pole;
	bool galactic_pole;
	bool analemma;
	bool analemmaline;
	bool circumpolar;
	bool galactic_center;
	bool vernal;
	bool greenwich;
	bool aries;
	bool equator;
	bool galactic_equator;
	bool meridian;
	bool tropic;
	bool ecliptic;
	bool precession;
	bool vertical;
	bool zodiac;
	bool zenith;
};

enum class SKYGRID_TYPE : char {
	GRID_EQUATORIAL,
	GRID_ECLIPTIC,
	GRID_GALACTIC,
	GRID_ALTAZIMUTAL,
	GRID_UNKNOWN
};

//structure that stores the grids at a time t
struct SkyGridSave {
	bool equatorial;
	bool ecliptic;
	bool galactic;
	bool altazimutal;
};

enum class SKYDISPLAY_TYPE : char{
	SKY_PERSON,
	SKY_NAUTIC,
	SKY_ORTHODROMY,
	SKY_LOXODROMY,
	SKY_COORDS,
	SKY_UNKNOWN
};

enum class SKYDISPLAY_NAME : char{
	SKY_PERSONAL,
	SKY_PERSONEQ,
	SKY_NAUTICAL,
	SKY_NAUTICEQ,
	SKY_ORTHODROMY,
	SKY_LOXODROMY,
	SKY_OBJCOORDS,
	SKY_MOUSECOORDS,
	SKY_ANGDIST
};

//structure that stores SkyDisplay at a time t
struct SkyDisplaySave {
	bool personal;
	bool personeq;
	bool nautical;
	bool nauticeq;
	bool orthodromy;
	bool loxodromy;
	bool objcoords;
	bool mousecoords;
	bool angdist;
};

#endif