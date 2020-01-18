/*
Classe minimale pour tableau circulaire
Utilité : Cette classe permet de pousser des éléments en début et en fin de tableau avec une complexité en O(1)
Usage : à inclure dans le programme C++
Remarque : cette classe est minimale (incomplète) et peut provoquer des erreurs dans le cas d'une utilisation spécifique
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour immersiveadventure.net
Mise à jour le 27/05/2017
*/

#ifndef SPECIAL_ARRAY_H
#define SPECIAL_ARRAY_H

#include <stdlib.h>

template<typename T>
class SpecialArray {

private:

	T* array; //Pointeur du tableau qui va stocker les éléments
	unsigned int asize; //Taille du tableau
	unsigned int begin; //Début du tableau

	void backward() {
		if(begin == 0) begin = asize; //Si on est au début on reboucle en à la fin
		begin--; //On recule
	}

public:

	SpecialArray() = delete; //TODO
	SpecialArray(const SpecialArray<T> &sa) = delete; //TODO

	SpecialArray(const unsigned int size) : asize(size), begin(0) {
		array = new T[size];    //Constructeur
	}
	~SpecialArray() {
		delete array;    //Destructeur TODO delete[]
	}

	const unsigned int size() const {
		return asize;
	}; //Getter de la taille du tableau


	T& push(int move) { //Ajout par devant
		if(move < 0) return pushFront();
		return pushBack();
	}

	T& pushFront() { //Ajout par devant
		backward();
		return array[begin]; //On assigne la valeur
	}

	T& pushBack() { //Ajout par derrière
		if(++begin == asize) {
			begin = 0;
			return array[asize-1];
		} else return array[begin-1];
	}

	void pushFront(const T &value) { //Ajout par devant
		backward();
		array[begin] = value; //On assigne la valeur
	}

	void pushBack(const T &value) { //Ajout par derrière
		array[begin] = value; //On assigne la valeur
		if(++begin == asize) begin = 0; //On avance et si on est à la fin on reboucle au début
	}

	T& operator[](const unsigned int index) { //Accès aléatoire à un élément //TODO const
		if(index < 0 || index >= asize) throw; //TODO //En dehors du tableau
		if(begin + index >= asize) return array[begin + index - asize]; //Avant begin
		return array[index + begin]; //Après begin
	}

};

#endif

