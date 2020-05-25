/*
 * event_handler.cpp
 * 
 * Copyright 2018 AssociationSirius
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include "eventModule/event_handler.hpp"
#include "eventModule/event.hpp"
#include "eventModule/event_handler_canvas.hpp"
#include "eventModule/event_recorder.hpp"

/*
 * EventHandler ------------------------------------------------------------------------
*/

EventHandler::EventHandler(EventRecorder* _eventManager)
{
	eventManager = _eventManager;
}

void EventHandler::handleEvents() {
	while (eventManager->haveEvents()) {
		// assume e != nullptr
		const Event* e = eventManager->getEvent();
		this->handle(e);
		delete e;
	}
}

void EventHandler::handle(const Event* e) {
	Event::Event_Type et = e->getEventType();
	auto it=handlerMap.find(et);

	if (it==handlerMap.end()) {
		std::cerr << "error taking handler " << e->toString() << std::endl;
		return;
	}
	
	it->second->handle(e);
}

void EventHandler::add(EventHandlerCanvas *sE, Event::Event_Type et){
	handlerMap.insert(std::make_pair(et,sE));
}

void EventHandler::remove(Event::Event_Type et){
	auto it=handlerMap.find(et);
	if (it->second != nullptr)
		delete it->second;
  	handlerMap.erase(it);	
}