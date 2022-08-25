/*
 * Copyright (C) 2018 of the LSS Team & Association Sirius
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

#ifndef _SCALABLE_HPP__
#define _SCALABLE_HPP__

/**
 * @file    scalable.hpp
 * @author  Olivier Nivoix
 * @version 1.0
 *
 * @section REQUIREMENTS
 * no requirement
 *
 * @section DESCRIPTION
 *
 * This template class provides mobile numbers T, which evolve by themselves according to a unit of measurement.
 * Define T with set() function.
 * Update T with update() function.
 *
 * @section EXAMPLE
 *
 *  Scalable<float> a;
 *  a.set(9.f);
 *
 * 	a = 5.f;
 *
 *  for (int i=0; i<50; i++) {
        a.update(20);
    }
 *
 *  The value of a will go from 9. to 5. during a unit of time, here by packets of 20.
 *
 *
 */

#include <ostream>

template <typename T>
class Scalable {
public:
	//! Create and initialise
	Scalable(){
		currentValue = 0;
        desiredValue = 0;
        updateCoeff = 0.;
        duration = 1000;
        counter = 0;
	}

	~Scalable() {
	}

    //! Increments the internal counter of delta_time ticks
	void update(int delta_ticks) {
        if (!isTransiting) {
			changedInUpdate = false;
            return;
		}
		changedInUpdate = true;
        counter+=delta_ticks;
		if (counter>=duration) {
			// Transition is over
			isTransiting = false;
			currentValue = desiredValue;
            counter = 0.;
		} else {
			currentValue = currentValue + updateCoeff * delta_ticks;
		}
    }

	//! assignment operator
	Scalable& operator=(T s){
        desiredValue = s;
        isTransiting = true;
        counter = 0;
        updateCoeff = (desiredValue - currentValue) / float(duration);
        return *this;
    }

	//! Comparison operator
    bool operator==(T s) const {
		return currentValue==s;
	}

	//! Return operator
	operator T() const {
		return currentValue;
	}

	//! Definition of transit time
	void setDuration(int _duration) {
		duration = _duration;
        counter = 0;
	}

	//! Indication of the duration of the transit
	float getDuration() const {
        return duration;
    }

	//! Initial definition and initialization of scalable
    void set(T f){
        currentValue = f;
        desiredValue = f;
        isTransiting = false;
    }

	//! returns the current value of the scalable in the type T
    T value() const {
        return currentValue;
    }

	T final() const {
		return desiredValue;
	}

	bool isScaling(){
		return changedInUpdate;
	}

	//! ostream display compatibility
    friend std::ostream& operator << (std::ostream & sortie, const Scalable &s) {
		return sortie << s.currentValue;
	}


protected:
    int duration;	//!< total number of measurement units of the change
    int counter;	//!< number of measurement units already elapsed
	T currentValue;	//!< value of the scalable
	T desiredValue; //!< final value after change
    T updateCoeff = 0;	//!< value representing 1 unit of change
    bool isTransiting = false;	//!< boolean indicating if a change is made
	bool changedInUpdate = false; //!< Tell if the value has changed with last update
};

#endif // _SCALABLE_HPP__
