/*
Classe de gestion basique de paralélisation de boucle
Utilité : Cette classe permet de paralélliser des calculs et peut se résumer à une boucle while(itérateur)
Usage : à inclure dans le programme C++
Remarque : version basique **non** thread safe
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour immersiveadventure.net
Mise à jour le 14/06/2017
*/

#ifndef PARRALLELIZED_LOOP
#define PARRALLELIZED_LOOP

#include <mutex>
#include <thread>

template<typename I> //Type de l'itérateur
class LoopParallelizer {

private:

	const int nT; //Nombre de threads
	bool stop = false; //Variable d'arrêt des threads
	std::mutex* managerMutexes; //Pointeur du tableau contenant les mutexes qui contrôlent le manager
	std::mutex* threadMutexes; //Pointeur du tableau contenant les mutexes qui contrôlent les threads
	std::thread* threads; //Pointeur du tableau contenant les threads

	virtual bool iteration(I it) = 0; //Fonction virtuelle pure à implémenter qui sera exécutée à chaque itération et qui doit **absolument** renvoyer faux quand le traitement est terminé
	virtual I next() = 0; //Fonction virtuelle pure qui **doit** être thread safe à implémenter qui donne l'itérateur suivant

	void threadManager(const int id) { //Fonction qui gère un thread
		threadMutexes[id].lock(); //On tente de verouiller le mutex qui sera dévérouillé par le manager
		while(!stop) { //Tant que le manager n'a pas demandé que les threads s'arrêtent
			if(!iteration(next())) { //On lance le traitement et si la fonction renvoit faux
				managerMutexes[id].unlock(); //On informe le manager qu'on a finit le traitement
				threadMutexes[id].lock(); //On tente de verouiller le mutex qui sera dévérouillé par le manager
			}
		}
	}
	
	static void threadManagerWrapper(LoopParallelizer* lp, const int i) { lp->threadManager(i); } //Wrapper

public:

	virtual void compute() { //Lance les calculs des threads et retourne quand tous les objets on été traité
		for(int i=0;i<nT;++i) threadMutexes[i].unlock(); //Lance les threads
		for(int i=0;i<nT;++i) managerMutexes[i].lock(); //Attend la fin des threads
	}

	LoopParallelizer(const int ThreadCount = std::thread::hardware_concurrency()): nT(ThreadCount) { //Constructeur
		managerMutexes = new std::mutex[nT]; //Créé le tableau de mutexes du manager
		threadMutexes = new std::mutex[nT]; //Créé le tableau de mutexes des threads
		for(int i=0;i<nT;++i) threadMutexes[i].lock(); //Bloque les threads
		for(int i=0;i<nT;++i) managerMutexes[i].lock(); //Bloque le manager
		threads = new std::thread[nT]; //Créé le tableau de threads
		for(int i=0;i<nT;++i) threads[i] = std::thread(threadManagerWrapper, this, i); //Créé les threads
	}

	virtual ~LoopParallelizer() { //Destructeur
		stop = true; //Demande l'arrêt des threads
		for(int i=0;i<nT;++i) threadMutexes[i].unlock(); //Lance les threads
		for(int i=0;i<nT;++i) threads[i].join(); //Attend la fin des threads
		delete managerMutexes;
		delete threadMutexes;
		//delete threads;//TODO sigsegv
	}

};

#endif
