/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Fabien Chereau
 * Copyright (C) 2007 Digitalis Education Solutions, Inc.
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

#ifndef _FADER_H_
#define _FADER_H_

#include <cstdio>
#include <cfloat>

/**
 * @file fader.hpp
 * @brief contains all the fader classes that manage the transition between two states
 * 
 * @class Fader
 * 
 * @brief Class canvas which manages a (usually smooth) transition between two states (typically ON/OFF) in function of a counter
 * It used for various purpose like smooth transitions between two states
 *
 * virtual base class :  not usable directly
 */
class Fader {
public:
	//! Create and initialise
	Fader(bool _state, float _min_value=0.f, float _max_value=1.f) : state(_state), min_value(_min_value), max_value(_max_value) {
		;
	}
	virtual ~Fader() {
		;
	}
	//! Increments the internal counter of delta_time ticks
	virtual void update(int delta_ticks) = 0;
	//! Gets current switch state
	virtual float getInterstate() const = 0;
	virtual float getInterstatePercentage() const = 0;
	//! Switchors can be used just as bools
	virtual Fader& operator=(bool s) = 0;
	bool operator==(bool s) const {
		return state==s;
	}
	operator bool() const {
		return state;
	}
	virtual void setDuration(int _duration) {
		;
	}

	virtual void reset(bool _state) = 0;

	virtual float getDuration() = 0;
	virtual void setMinValue(float _min) {
		min_value = _min;
	}
	virtual void setMaxValue(float _max) {
		max_value = _max;
	}
	float getMinValue() {
		return min_value;
	}
	float getMaxValue() {
		return max_value;
	}
protected:
	bool state;
	float min_value, max_value;
};

/**
 *  @class BooleanFader
 * 
 * @brief boolean transition between two states
 * 
 */
class BooleanFader : public Fader {
public:
	//! Create and initialise
	BooleanFader(bool _state=false, float _min_value=0.f, float _max_value=1.f) : Fader(_state, _min_value, _max_value) {
		;
	}
	~BooleanFader() {
		;
	}
	//! Increments the internal counter of delta_time ticks
	void update(int delta_ticks) {
		;
	}

	virtual void reset(bool _state) {
		state = _state;
	}

	//! Gets current switch state
	float getInterstate() const {
		return state ? max_value : min_value;
	}
	float getInterstatePercentage() const {
		return state ? 100.f : 0.f;
	}
	//! Switchors can be used just as bools
	Fader& operator=(bool s) {
		state=s;
		return *this;
	}
	virtual float getDuration() {
		return 0.f;
	}
protected:
};

/**
 *  @class LinearFader
 * 
 * @brief Linear transition between two states
 * 
 * Please note that state is updated instantaneously, so if you need to draw something fading in
 * and out, you need to check the interstate value (!=0) to know to draw when on AND during transitions
 */
class LinearFader : public Fader {
public:
	//! Create and initialise to default
	LinearFader(int _duration=2000, float _min_value=0.f, float _max_value=1.f, bool _state=false)
		: Fader(_state, _min_value, _max_value) {
		is_transiting = false;
		duration = _duration;
		interstate = state ? max_value : min_value;
	}

	~LinearFader() {
		;
	}

	//! Increments the internal counter of delta_time ticks
	void update(int delta_ticks) {
		if (!is_transiting) return; // We are not in transition
		counter+=delta_ticks;
		if (counter>=duration) {
			// Transition is over
			is_transiting = false;
			interstate = target_value;
			// state = (target_value==max_value) ? true : false;
		} else {
			interstate = start_value + (target_value - start_value) * counter/duration;
		}

		//		printf("Counter %d  interstate %f\n", counter, interstate);
	}

	virtual void reset(bool _state) {
		is_transiting = false;
		state = _state;
		interstate = state ? max_value : min_value;
		counter = duration;
	}

	//! Get current switch state
	float getInterstate() const {
		return interstate;
	}

	float getInterstatePercentage() const {
		return 100.f * (interstate-min_value)/(max_value-min_value);
	}

	//! Faders can be used just as bools
	Fader& operator=(bool s) {

		if (is_transiting) {
			// if same end state, no changes
			if (s == state) return *this;

			// otherwise need to reverse course
			state = s;
			counter = duration - counter;
			float temp = start_value;
			start_value = target_value;
			target_value = temp;

		} else {

			if (state == s) return *this; // no change

			// set up and begin transit
			state = s;
			start_value = s ? min_value : max_value;
			target_value = s ? max_value : min_value;
			counter=0;
			is_transiting = true;
		}
		return *this;
	}

	void setDuration(int _duration) {
		if (_duration <= 0) duration=0;
		else
			duration = _duration;
	}
	virtual float getDuration() {
		return duration;
	}
	void setMaxValue(float _max) {
		if (interstate >=  max_value) interstate =_max;
		max_value = _max;
	}

protected:
	bool is_transiting;
	int duration;
	float start_value, target_value;
	int counter;
	float interstate;
};


/**
 *  @class ParabolicFader
 * 
 * @brief Parabolic transition between two states
 * 
 * Please note that state is updated instantaneously, so if you need to draw something fading in
 * and out, you need to check the interstate value (!=0) to know to draw when on AND during transitions
 */
class ParabolicFader : public Fader {
public:
	//! Create and initialise to default
	ParabolicFader(int _duration=1000, float _min_value=0.f, float _max_value=1.f, bool _state=false)
		: Fader(_state, _min_value, _max_value) {
		is_transiting = false;
		duration = _duration;
		interstate = state ? max_value : min_value;
	}

	~ParabolicFader() {
		;
	}

	//! Increments the internal counter of delta_time ticks
	void update(int delta_ticks) {
		if (!is_transiting) return; // We are not in transition
		counter+=delta_ticks;
		if (counter>=duration) {
			// Transition is over
			is_transiting = false;
			interstate = target_value;
			// state = (target_value==max_value) ? true : false;
		} else {
			interstate = start_value + (target_value - start_value) * counter/duration;
			interstate *= interstate;
		}

		// printf("Counter %d  interstate %f\n", counter, interstate);
	}

	virtual void reset(bool _state) {
		is_transiting = false;
		state = _state;
		interstate = state ? max_value : min_value;
		counter = duration;
	}

	//! Get current switch state
	float getInterstate() const {
		return interstate;
	}
	float getInterstatePercentage() const {
		return 100.f * (interstate-min_value)/(max_value-min_value);
	}

	//! Faders can be used just as bools
	Fader& operator=(bool s) {

		if (is_transiting) {
			// if same end state, no changes
			if (s == state) return *this;

			// otherwise need to reverse course
			state = s;
			counter = duration - counter;
			float temp = start_value;
			start_value = target_value;
			target_value = temp;

		} else {

			if (state == s) return *this; // no change

			// set up and begin transit
			state = s;
			start_value = s ? min_value : max_value;
			target_value = s ? max_value : min_value;
			counter=0;
			is_transiting = true;
		}
		return *this;
	}

	void setDuration(int _duration) {
		duration = _duration;
	}
	virtual float getDuration() {
		return duration;
	}
	bool isTransiting() {
		return is_transiting;
	}
	bool getState() {
		return state;
	}
protected:
	bool is_transiting;
	int duration;
	float start_value, target_value;
	int counter;
	float interstate;
};

#endif //_FADER_H_
