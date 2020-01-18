#ifndef THREADS
#define THREADS

////////////////////////////* Tunnable *////////////////////////////////////////


/* Active le multithreading si MAX_CPU_COUNT >=1
 * Indique la taille maximale des tableaux de threads dans le programe
 */
#define MAX_CPU_COUNT 12


/* Si MAX_CPU_COUNT >=1 alors ce qui suit est actif:
 * 
 * FORCE_CPU_COUNT
 * permet de définir manuellement le nombre de threads utilisable par le systeme
 * Si FORCE_CPU_COUNT n'est pas défini alors le nombre de threads sera égal à 
 * std::thread::hardware_concurrency()
 * 
 */
//#define FORCE_CPU_COUNT 8


// active la gestion des threads persistants dans les parallelized loop
//#define THREAD_DISABLE_PERSISTANCE //TODO this line has absolutely no effect now
// désactive la gestion des boucles d'iterators avec les threads
#define THREAD_DISABLE_FOR_ITER

////////////////////////////////////////////////////////////////////////////////

/* Liste de define permettant de désactiver en cas de multithreading une partie 
 * du code threadé.
 */

//#define THREAD_DISABLE_ATMOSPHERE
#define THREAD_DISABLE_COMPUTE_POSITIONS
//#define THREAD_DISABLE_DRAW_SOLAR_SYSTEM
//#define THREAD_DISABLE_STARNAVIGATOR_COMPUTE_POSITION

////////////////////////////////////////////////////////////////////////////////

        /* STATIC don't configure manually the following code */

////////////////////////////////////////////////////////////////////////////////

#ifndef FORCE_CPU_COUNT
	#if MAX_CPU_COUNT >= 1
		#define MULTITHREADING
		#define THREAD_COUNT std::thread::hardware_concurrency()
		#define THREAD_BUF_COUNT MAX_CPU_COUNT
	#endif //MAX_CPU_COUNT >= 1
#else //FORCE_CPU_COUNT
	#if FORCE_CPU_COUNT >= 1
		#define MULTITHREADING
		#define THREAD_COUNT FORCE_CPU_COUNT
		#define THREAD_BUF_COUNT FORCE_CPU_COUNT
		#define FORCE_CPU_COUNT_WARNING std::cerr << "(WW) Thread count has been statically forced to " << FORCE_CPU_COUNT << " for " << std::thread::hardware_concurrency() << " threads available !" << std::endl;
	#endif //FORCE_CPU_COUNT >= 1
#endif //FORCE_CPU_COUNT

#ifndef MULTITHREADING
	#ifndef THREAD_DISABLE_ATMOSPHERE
		#define THREAD_DISABLE_ATMOSPHERE
	#endif //THREAD_DISABLE_ATMOSPHERE

	#ifndef THREAD_DISABLE_COMPUTE_POSITIONS
		#define THREAD_DISABLE_COMPUTE_POSITIONS
	#endif //THREAD_DISABLE_COMPUTE_POSITIONS

	#ifndef THREAD_DISABLE_DRAW_SOLAR_SYSTEM
		#define THREAD_DISABLE_DRAW_SOLAR_SYSTEM
	#endif //THREAD_DISABLE_DRAW_SOLAR_SYSTEM

	#ifndef THREAD_DISABLE_STARNAVIGATOR_COMPUTE_POSITION
		#define THREAD_DISABLE_STARNAVIGATOR_COMPUTE_POSITION
	#endif //THREAD_DISABLE_STARNAVIGATOR_COMPUTE_POSITION
#endif //MULTITHREADING

#endif //THREADS
