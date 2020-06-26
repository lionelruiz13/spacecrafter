/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 Elitit-40
 * Copyright (C) 2020 Elitit-40
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */


#ifndef SCREEN_FADER_EVENT_HPP
#define SCREEN_FADER_EVENT_HPP

#include <string>
#include <sstream>
#include "eventModule/event.hpp"

class ScreenFaderEvent : public Event {
public:
    enum ScreenFaderTypeEvent : char {UP, DOWN, FIX, CHANGE};

    ScreenFaderEvent(ScreenFaderTypeEvent _strategy, float _value) : Event(E_SCREEN_FADER) {
        strategy = _strategy;
        value = _value;
    }
    ScreenFaderEvent(ScreenFaderTypeEvent _strategy, float _value, float _duration) : Event(E_SCREEN_FADER) {
        strategy = _strategy;
        value = _value;
	duration = _duration;
    }
    ~ScreenFaderEvent(){};

    ScreenFaderTypeEvent getStrategy() {
        return strategy;
    }

    float getValue() const {
        return value;
    }

    float getTime() const {
        return duration;
    }

    virtual std::string toString() const{
        std::ostringstream os;
        os << Event::toString() << " Strategy : " << strategy << " value " << value << std::endl;
        return os.str();
    }
    
private:
    ScreenFaderTypeEvent strategy;
    float value;
    float duration;
};


class ScreenFaderInterludeEvent : public Event {
public:
    enum ScreenFaderInterludeTypeEvent : char {UP, DOWN};

    ScreenFaderInterludeEvent(ScreenFaderInterludeTypeEvent _strategy, float _min, float _max, float _value) : Event(E_SCREEN_FADER_INTERLUDE) {
        strategy = _strategy;
        value = _value;
        min = _min;
        max = _max;
    }
    ~ScreenFaderInterludeEvent(){};

    ScreenFaderInterludeTypeEvent getStrategy() {
        return strategy;
    }

    float getValue() const {
        return value;
    }
    float getMin() const {
        return min;
    }
    float getMax() const {
        return max;
    }

    virtual std::string toString() const{
        std::ostringstream os;
        os << Event::toString() << std::endl << " Strategy : " << strategy << " min " << min << " max " << max << " value " << value << std::endl;
        return os.str();
    }


private:
    ScreenFaderInterludeTypeEvent strategy;
    float min, max, value;
};


#endif //SCREEN_FADER_EVENT_HPP
