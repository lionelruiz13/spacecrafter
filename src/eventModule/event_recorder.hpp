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


#ifndef EVENT_RECORDER_HPP
#define EVENT_RECORDER_HPP


#include "event.hpp"
#include <list>
#include <algorithm>

class EventRecorder{
public:
	EventRecorder(EventRecorder const &) = delete;
	EventRecorder& operator = (EventRecorder const &) = delete;
	
	static void End() {
		if (instance != nullptr) {
			delete instance;
		}
		instance = nullptr;
	}
	
    static EventRecorder * getInstance(){
        return instance == nullptr ? new EventRecorder() : instance;
    }

	static void Init(){
		if (instance != nullptr)
			delete instance;
		instance = new EventRecorder();
	}

    void queue(const Event* event){
		//std::cout << event->toString();
   	    envents.push_back(event);
    }

	bool haveEvents() {
		return !envents.empty();
	}

	const Event* getEvent() {
		const Event* event = envents.front();
		envents.pop_front();
		return event; 
	}

private:
    EventRecorder(){
    }

	~EventRecorder() {
		std::for_each(envents.begin(), envents.end(), [](const Event* e) { delete e; });
		envents.clear();
	}
    //singleton to not change all the classes of the software
    static EventRecorder * instance;
    // the event list
    std::list<const Event*> envents;
};

#endif
