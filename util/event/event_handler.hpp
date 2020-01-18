#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP


#include "event.hpp"
#include <iostream>

class EventHandler{
public:
	EventHandler() {
	}

	~EventHandler() {
	}

    void handle(Event e) {
		//~ std::cout << "EventHandler handle" << std::endl;
		switch(e.getEventType()) {
			case Event::Not_Set : 
				std::cout << "---> test not set" << std::endl;
				break;

			case Event::Altitude_Changed:
				std::cout << "---> test altitude changed " << e.getDouble() << std::endl;
				
				break;

			case Event::Atmosphere:
				std::cout << "---> test atmosphere " << e.getBool() << std::endl;
				
				break;

			case Event::E_Altimetre:
				if (e.getBool())
					std::cout << "---> test altimetre on" << std::endl;
				else
					std::cout << "---> test altimetre off" << std::endl;
				
				break;
			
			
			
			default: break;
		}	
	}


protected :

};


#endif
