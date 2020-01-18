/*
 * Copyright (C) 2018 Immersive Adventure
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

#ifndef _CORE_EXECUTOR_HPP_
#define _CORE_EXECUTOR_HPP_

class Observer;
class Core;

//
// classe virtuelle qui représente le squelette d'un mode du logiciel.
//
class CoreExecutor {
public:
	//fonction qui est exécutée lorsque le logiciel rentre dans ce mode
	virtual void onEnter() = 0;
	//fonction qui est exécutée lorsque le logiciel sort de ce mode
	virtual void onExit() = 0;
	// focntion tube qui sert à lancer les updates des objets de ce mode
	virtual void update(int delta_time)=0;
	// focntion tube qui sert à lancer les draw des objets de ce mode
	virtual void draw(int delta_time)=0;

	virtual ~CoreExecutor();

	// indique si l'altitude est bien comprise dans le mode ie si on doit rester dans ce mode ou pas
	// renvoie true si on doit changer de mode, false sinon
	virtual bool testValidAltitude(double altitude) = 0;

	// renvoie le nom du mode qui devrait succéder à this si testValidAltitude indique un changement de mode
	CoreExecutor* getNextMode();

	//indique le nom du mode
	const std::string getName() {return name;};

	// indique quel mode succèderait à this si testValidAltitude renvoie true
	void defineUpMode(CoreExecutor* _upMode){
		upMode = _upMode;
	}
	// indique quel mode succèderait à this si testValidAltitude renvoie true
	void defineDownMode(CoreExecutor* _downMode){
		downMode = _downMode;
	}

protected:
	// constructeur de base de la classe
	CoreExecutor(Core *_core, Observer* _observer);
	Core* core;
	Observer* observer;
	CoreExecutor* nextMode=nullptr;
	CoreExecutor* upMode=nullptr;
	CoreExecutor* downMode=nullptr;
	std::string name;
	double minAltToGoDown = 0.0;	// altitude min avant changement de mode vers upMode
	double maxAltToGoUp = 0.0;		// altitude max avant changement de mode vers downMode
};

//
// classe spécialisée concernant le système solaire
//

class CoreExecutorInSolarSystem : public CoreExecutor {
public:
	virtual void onEnter() override;
	virtual void onExit() override;
	virtual void update(int delta_time) override;
	virtual void draw(int delta_time) override;
	virtual bool testValidAltitude(double altitude) override;
	CoreExecutorInSolarSystem(Core *_core, Observer* _observer);
	~CoreExecutorInSolarSystem();
private:
};


//
// classe spécialisée concernant la gestion des étoiles de la galaxie proche du soleil
//

class CoreExecutorInGalaxy : public CoreExecutor {
public:
	virtual void onEnter() override;
	virtual void onExit() override;
	virtual void update(int delta_time) override;
	virtual void draw(int delta_time) override;
	virtual bool testValidAltitude(double altitude) override;
	CoreExecutorInGalaxy(Core *_core, Observer* _observer);
	~CoreExecutorInGalaxy();
private:
};


//
// classe spécialisée concernant les galaxies proches de la MilkyWay
//

class CoreExecutorInUniverse : public CoreExecutor {
public:
	virtual void onEnter() override;
	virtual void onExit() override;
	virtual void update(int delta_time) override;
	virtual void draw(int delta_time) override;
	virtual bool testValidAltitude(double altitude) override;
	CoreExecutorInUniverse(Core *_core, Observer* _observer);
	~CoreExecutorInUniverse();
private:
};

#endif //_CORE_EXECUTOR_HPP_
