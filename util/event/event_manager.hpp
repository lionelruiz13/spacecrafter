#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP


#include "event.hpp"
#include "event_handler.hpp"
#include <list>

class EventManager{
public:
	EventManager(EventManager const &) = delete;
	EventManager& operator = (EventManager const &) = delete;
	
	static void End() {
		if (instance != nullptr) {
			delete instance;
		}
		instance = nullptr;
	}
	
    static EventManager * getInstance(){
        return instance == nullptr ? new EventManager() : instance;
    }

	static void Init(){
		if (instance != nullptr)
			delete instance;
		instance = new EventManager();
	}

    void queue(Event &event){
		std::cout << "Add event" << std::endl;
        envents.push_back(event);
    }

    void handleEvents(){
		std::cout << "handle event" << std::endl;
        std::cout << "Taille de envents " << envents.size() << std::endl;
        while( ! envents.empty() ){
			//~ std::cout << "chain handle event" << std::endl;
            chain->handle(envents.front());
            envents.pop_front();
        }
    }

private:
    EventManager(){
		std::cout << "Creation de l'EventManager" << std::endl;
        chain = new EventHandler();
    }

	~EventManager() {
		std::cout << "Delete EventManager" << std::endl;
        if (chain)
			delete chain;
	}

    //singleton pour ne pas changer toutes les classes du logiciel
    static EventManager * instance;
    // la liste des évènements
    std::list<Event> envents;
    //La chaine qui s'occuper de traiter les évenemnts
    EventHandler * chain;
};

#endif
