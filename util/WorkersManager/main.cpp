#include "parallelized_for.hpp"
#include "../PerformanceDebugger/perf_debug.hpp"

#include <stdio.h>
#include <math.h> 
#include <iostream>

#define FORCE_CPU_COUNT 4 //Valeur forcée du nombre de threads
#define FRAMES 1000 //Simule un nombre d'images
#define START 0 //Début du for
#define STOP 1000 //Fin du for (exclu)
#define CONSUME 1000 //Nombre d'itérations pour perdre du temps
#define DISPLAY 50 //Nombre maximum de cases tu tableau à afficher

#define SIZE STOP-START //Taille du tableau

void compute(const int i,  double* tab) { //Fonction qui perd un peu de temps avant de calculer une racine carrée
	for(int j=0; j < CONSUME; ++j);
	tab[i-START] = sqrt(i); //Range la racine carrée de i pour tester
} 

int main() {
PerformanceDebugger pd; //Instanciation d'un debugger de performances
double* valeurs = new double[SIZE]; //Création d'un tableau de valeurs qui sera rempli par un seul thread
double* valeursP = new double[SIZE]; //Création d'un tableau de valeurs qui sera rempli par plusieurs threads

//std::atomic<int> it; //Beaucoup plus rapide
for(int it=0; it<FRAMES; ++it) { //Simule FRAMES images sans paralélisation
	pd.startTimer("Normal Loop"); //Debug
	for(int j=START; j<STOP; ++j) compute(j, valeurs); //Lance le calcul sur un seul thread
	pd.stopTimer("Normal Loop"); //Debug
}

int error = 0;
ForParallelizer<double*> fp(FORCE_CPU_COUNT); //Création d'un parallelisateur de boucle for prenant en paramètre de la fonction : int*
for(int i=0; i<FRAMES; ++i) { //Simule FRAMES images avec parallelisation
	pd.startTimer("Parallelized Loop"); //Debug
	fp.compute(START, STOP, compute, valeursP); //Lancement du calcul en paralèle de START à STOP (exclu) avec la fonction compute et le tableau valeurs
	pd.stopTimer("Parallelized Loop"); //Debug
//	for(int i=0; i<SIZE; ++i) if (valeurs[i] != valeursP[i]) ++error; //Debug erreurs
//	for(int i=0; i<SIZE; ++i) valeursP[i] = 0; //Debug erreurs
}

pd.exportData("perf.csv"); //Export des données de débug des performances
for(int i = 0; i < std::min(DISPLAY, SIZE); ++i) std::cout << "sqrt(" << START+i << ") = " << valeurs[i] << " | " << valeursP[i] << std::endl; //Affichage du tableau
//std::cout << "Erreurs : " << error << " errors / " << SIZE << " values / " << FRAMES << " frames = " << 100.0*error/SIZE/FRAMES << "%" << std::endl; //Debug erreurs
delete valeurs;
}
