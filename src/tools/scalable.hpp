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
 * Cette classe template met à disposition des nombres T mobiles, qui évoluent d'eux même en fonction d'une unité de mesure. 
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
 *  La valeur de a passera de 9. à 5. pendant duration unité de temps, ici par paquets de 20.
 *  
 * 
 */

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
        if (!isTransiting)
            return;
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

	//! opérateur d'assignement
	Scalable& operator=(T s){
        desiredValue = s;
        isTransiting = true;
        counter = 0;
        updateCoeff = (desiredValue - currentValue) / float(duration);
        return *this;
    }
	
	//! Opératuer de comparaison
    bool operator==(T s) const {
		return currentValue==s;
	}
	
	//! Opérateur de retour 
	operator T() const {
		return currentValue;
	}

	//! Définition de la durée du transit 
	void setDuration(int _duration) {
		duration = _duration;
        counter = 0;
	}

	//! Indication de la durée du transit
	float getDuration() const {
        return duration;
    }

	//! Définition initiale et initialisation du scalable
    void set(T f){
        currentValue = f;
        desiredValue = f;
        isTransiting = false;
    }

	//! renvoie la valeur actuelle du scalable dans le type T
    T value() const {
        return currentValue;
    }

	T final() const {
		return desiredValue;
	}

	bool isScaling(){
		return isTransiting;
	}

	//! compatibilité affichage ostream 
    friend std::ostream& operator << (std::ostream & sortie, const Scalable &s) {
		return sortie << s.currentValue;
	}


protected:
    int duration;	//!< nombre total d'unité de mesure du changement
    int counter;	//!< nombre d'unité de mesure déjà écoulée 
	T currentValue;	//!< valeur du scalable
	T desiredValue; //!< valeur finale après changement
    T updateCoeff = 0;	//!< valeur représentant 1 unité de changement
    bool isTransiting = false;	//!< booléan indiquant si on effectue un changement 
};

#endif // _SCALABLE_HPP__