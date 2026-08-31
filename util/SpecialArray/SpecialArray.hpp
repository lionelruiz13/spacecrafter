/*
Classe minimale pour tableau circulaire
Utilite : Cette classe permet de pousser des elements en debut et en fin de tableau avec une complexite en O(1)
Usage : a inclure dans le programme C++
Remarque : cette classe est minimale (incomplete) et peut provoquer des erreurs dans le cas d'une utilisation specifique
Auteur : Aurelien Schwab <aurelien.schwab+dev@gmail.com> pour immersiveadventure.net
Mise a jour le 27/05/2017
*/

#ifndef SPECIAL_ARRAY_H
#define SPECIAL_ARRAY_H

#include <stdlib.h>

template<typename T>
class SpecialArray {

private:

	T* array; //Pointeur du tableau qui va stocker les elements
	unsigned int asize; //Taille du tableau
	unsigned int begin; //Debut du tableau
	
	void backward() {
		if(begin == 0) begin = asize; //Si on est au debut on reboucle en a la fin
		begin--; //On recule
	}

public:

	SpecialArray() = delete; //TODO
	SpecialArray(const SpecialArray<T> &sa) = delete; //TODO

	SpecialArray(const unsigned int size) : asize(size), begin(0) { array = new T[size]; } //Constructeur
	~SpecialArray() { delete array; array = nullptr; } //Destructeur TODO delete[]

	const unsigned int size() const { return asize; }; //Getter de la taille du tableau
	
	T& pushFront() { //Ajout par devant
		backward();
		return array[begin]; //On assigne la valeur
	}
	
	T& push_back() { //Ajout par derriere
		if(++begin == asize) {
			begin = 0;
			return array[asize-1];
		}
		else return array[begin-1];
	}

	void pushFront(const T &value) { //Ajout par devant
		backward();
		array[begin] = value; //On assigne la valeur
	}

	void push_back(const T &value) { //Ajout par derriere
		array[begin] = value; //On assigne la valeur
		if(++begin == asize) begin = 0; //On avance et si on est a la fin on reboucle au debut
	}

	T& operator[](const unsigned int index) { //Acces aleatoire a un element //TODO const
		if(index < 0 || index >= asize) throw; //TODO //En dehors du tableau
		if(begin + index >= asize) return array[begin + index - asize]; //Avant begin
		return array[index + begin]; //Apres begin
	}

};

#endif

