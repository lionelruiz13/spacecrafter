#include "event_manager.hpp"
#include "observer.hpp"
#include "Altimetre.hpp"



int main()
{
	EventManager::Init();
	EventManager* eventManager = EventManager::getInstance();
		
	Observer test1;
	test1.setAltitude(30.);
	test1.setAltitude(30.);
	test1.setAltitude(30.);
	test1.setAltitude(30.);
	test1.setAltitude(30.);
	test1.setAltitude(30.);
	test1.setAltitude(30.);
	test1.setAltitude(30.);
		
	Observer test2;
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	test2.setAltitude(10.);
	
	Altimetre test3;
	test3.set(true);
	test3.set(false);
	test3.set(true);	
	test3.set(true);
	test3.set(false);
	test3.set(true);	
	
	eventManager->handleEvents();
	
	EventManager::End();
	return 0;
}
