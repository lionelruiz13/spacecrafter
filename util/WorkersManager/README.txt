Compiltation : 
g++ main.cpp -std=c++11 -lpthread ../PerformanceDebugger/perf_debug.cpp -o test

Ce test tente de simuler les conditions du logiciel afin de comparer les temps d'éxecution avec et sans parralélisation.

Sur un AMD FX8350 (8 threads : 4 blocks de 2 cores) avec 4 threads on a avec la parralélisation à ce stade de développement une perte de temps CPU d'environ 10% mais un ratio d'environ 3,5 en vitesse.

Données du test :

Images simulées : 1000 (sans pauses)
Calculs par images : 1000 (très équitablement répartis en temps de calcul sur chaque thread)

Timings (environ, en micro-secondes):

Sans parallelisation : 
CPU :	2 238 443
Réel :	2 238 586

Avec parallelisation : 
CPU :	2 389 505 (~107%)
Réel :	619 904 (~28%, ~x3.5)


Elimination des mutexes et utilisation d'un type std::atomic<int> à la place de int comme itérateur du for pour la version parallelisée

Sans parallelisation : 
CPU :	2 196 298
Réel :	2 197 877

Avec parallelisation : 
CPU :	1 687 904 (~77%, ~x1.3)
Réel :	432 296 (~20%, ~x5)

