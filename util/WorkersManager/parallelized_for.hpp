/*
Classe de gestion basique de paralélisation de plus haut niveau que LoopParallelizer afin de gérer une boucle for
Utilité : Cette classe permet de paralélliser des calculs et peut se résumer à une boucle for(int i = début, i < fin, ++i) fonction(i, objet);
Usage : à inclure dans le programme C++
Remarque : version basique **non** thread safe
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour immersiveadventure.net
Mise à jour le 14/06/2017
*/

#ifndef PARALELIZED_FOR
#define PARALELIZED_FOR

#include "parallelized_loop.hpp"
#include <atomic>

template<typename T> //Type de l'objet passé en paramètre de la fonction
class ForParallelizer : protected LoopParallelizer<int> {

private:
	std::atomic<int> iterator; //Itérateur
	int end; //Valeur d'arrêt (exlu)
	void (*func)(int, T); //Fonction à éxécuter à chaque itération
	T obj; //Objet passé en paramètre de la fonction

	bool iteration(int it) { //Fonction éxécutée à chaque itération
		if (it < end) { //Si on a pas encore atteint la fin
			func(it, obj); //On éxécute la fonction avec l'itérateur et l'objet passés en paramètre)
			return true; //On informe le manager qu'on a effectué un traitement
		}
		else return false; //On informe le manager qu'on a plus rien à traiter
	}
	
	int next() { return iterator++; } //Renvoie l'itérateur suivant

public:

	void compute(const int Begin, const int End, void (*Function)(int, T), T Obj) { //Fonction qui lance un boucle for de Begin à End en éxécutant la fonction Function à chaque itération avec l'objet Obj en paramètre
		std::atomic_init(&iterator, Begin);
		end = End;
		func = Function;
		obj = Obj;
		LoopParallelizer::compute();
	}

	ForParallelizer() {}
	ForParallelizer(const int ThreadCount): LoopParallelizer(ThreadCount) {}
	~ForParallelizer() {}

};

#endif
