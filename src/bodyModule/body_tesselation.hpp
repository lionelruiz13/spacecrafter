#ifndef _BODY_TESSELATION_HPP_
#define _BODY_TESSELATION_HPP_

#include "tools/scalable.hpp"

class BodyTesselation 
{
    public:

	void createTesselationParams() {
		min_tes_level.set(1);
		max_tes_level.set(1);
		planet_altimetry_factor.set(1);
		moon_altimetry_factor.set(1);
		earth_altimetry_factor.set(1);
		min_tes_level_ini=1;
		max_tes_level_ini=1;
		planet_altimetry_factor_ini=1;
		moon_altimetry_factor_ini=1;
		earth_altimetry_factor_ini=1;
	}

	void resetTesselationParams() {
		min_tes_level = 1;
		max_tes_level = 1;
		planet_altimetry_factor = 1;
		moon_altimetry_factor = 1;
		earth_altimetry_factor = 1;		
	}

	void updateTesselation (int delta_time) {
		min_tes_level.update(delta_time);
		max_tes_level.update(delta_time);
		planet_altimetry_factor.update(delta_time);
		moon_altimetry_factor.update(delta_time);
		earth_altimetry_factor.update(delta_time);
	}

    void setMinTes(int v, bool ini = false) {
		if (ini) {
			min_tes_level.set(v);
			min_tes_level_ini = v;
		} else
			min_tes_level = v;
	}

	void setMaxTes(int v, bool ini = false) {
		if (ini) {
			max_tes_level_ini = v;
			max_tes_level.set(v);
		} else
		max_tes_level = v;
	}

	void setPlanetTes(int v, bool ini = false) {
		if (ini) {
			planet_altimetry_factor_ini = v;
			planet_altimetry_factor.set(v);
		} else
		planet_altimetry_factor = v;
	}

	void setMoonTes(int v, bool ini = false) {
		if (ini) {
			moon_altimetry_factor_ini = v;
			moon_altimetry_factor.set(v);
		} else
			moon_altimetry_factor = v;
	}

	void setEarthTes(int v, bool ini = false) {
		if (ini) {
			earth_altimetry_factor_ini = v;
			earth_altimetry_factor.set(v);
		} else
			earth_altimetry_factor = v;
	}

    int getMinTesLevel() {
        return min_tes_level.value();
    }

    int getMaxTesLevel() {
        return max_tes_level.value();
    }

    int getEarthAltimetryFactor() {
        return earth_altimetry_factor.value();
    }

    int getMoonAltimetryFactor() {
        return moon_altimetry_factor.value();
    }

    int getPlanetAltimetryFactor() {
        return planet_altimetry_factor.value();
    }

private:
	Scalable<int> min_tes_level;
	Scalable<int> max_tes_level;
	Scalable<int> planet_altimetry_factor;
	Scalable<int> moon_altimetry_factor;
	Scalable<int> earth_altimetry_factor;
	int min_tes_level_ini;
	int max_tes_level_ini;
	int planet_altimetry_factor_ini;
	int moon_altimetry_factor_ini;
	int earth_altimetry_factor_ini;
};

#endif // _BODY_TESSELATION_HPP_