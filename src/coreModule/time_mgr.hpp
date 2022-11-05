/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2016 Association Sirius
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

#ifndef __TIME_MGR_HPP__
#define __TIME_MGR_HPP__

// Conversion in standar Julian time format
#define JD_SECOND 0.000011574074074074074074
#define JD_MINUTE 0.00069444444444444444444
#define JD_HOUR   0.041666666666666666666
#define JD_DAY    1.

#include "tools/utility.hpp"
#include "tools/no_copy.hpp"
#include <cassert>

class TimeMgr : public NoCopy {
public:
	TimeMgr();
	~TimeMgr();

	// Time controls
	void setJDay(double JD) {
		JDay=JD;
	}
	double getJDay(void) const {
		return JDay;
	}

	void changeTimeSpeed(double _time_speed, double duration);

	void setTimeSpeed(double ts) {
		//std::cout << "TimeSpeed is "<< time_speed << " & it became " << ts << std::endl;
		time_speed=ts;
	}

	double getTimeSpeed(void) const {
		return (timeLockCount) ? 0 : time_speed;
	}

	double getTimePause(void) const {
		return FlagTimePause;
	}
	void setTimePause(bool _value) {
		if (FlagTimePause != _value) {
			if (_value) {
				FlagTimePause = true;
				++timeLockCount;
			} else {
				FlagTimePause = false;
				--timeLockCount;
			}
		}
	}

	//! Acquire a time lock, prevent time to change while at least one lock is hold
	inline void lockTime() {
		// ++timeLockCount;
	}

	//! Release a time lock
	inline void unlockTime() {
		// assert(--timeLockCount >= 0);
	}

	// double getTimeMultiplier() const {
	// 	std::cout << "time_multiplier is "<< time_multiplier << std::endl;
	// 	return time_multiplier;
	// }
	// void setTimeMultiplier(double _value) {
	// 	std::cout << "time_multiplier is "<< time_multiplier << " & it became " << _value << std::endl;
	// 	time_multiplier = _value;
	// }
	//! Increment time
	void update(int delta_time); // ancien update_time

	double getJulian(void) const {
		return JDay;
	}

	//! return the JD time when the sun go down
	double dateSunRise(double jd, double longitude, double latitude);
	//! return the JD time when the sun set up
	double dateSunSet(double jd, double longitude, double latitude);
	//! return the JD time when the sun cross the meridian
	double dateSunMeridian(double jd, double longitude, double latitude);

private:
	// Time variable
	double time_speed;				// Positive : forward, Negative : Backward, 1 = 1sec/sec
	double JDay;        			// Curent time in Julian day
	bool FlagTimePause = false;		// say if software time is in pause
	bool FlagChangeTimeSpeed;
	int timeLockCount = 0;			// Number for lock acquired, paused if non-zero
	double start_time_speed, end_time_speed;
//	int time_multiplier;			//! used for adjusting delta_time for script speeds
	float move_to_coef, move_to_mult;
};

#endif
