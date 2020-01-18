#include <iostream>
#include <vector>
#include <chrono>
#include <unistd.h>
#include "tools/ThreadPool.hpp"
#include <sstream>

#define NB_VALUE 500

class number {
public:
	int recherche_diviseurs (int a);
	bool myJob(int i);
	static bool static_myJob(number *a,int i) { return a->myJob(i); }
};

int number::recherche_diviseurs (int a) {
  int i, j = 0;
  for (i = 1; i < a + 1; i++){
    if ((a % i) == 0){
	  j = j + 1;
	}
  }
  return j;
}

bool number::myJob(int i)
{
	int j=0;
	int tmp;
	for(int k=NB_VALUE*i; k<NB_VALUE*(i+1); k++) {
		tmp=recherche_diviseurs(k);
		if (tmp>j)
			j=tmp;
	}
	return true;
};

int main(int argc, char **argv)
{
	if (argc < 3) {
		std::cout << "manque arguments : arg1 <frames> arg2 <nombre_paquets_500>" << std::endl;
		exit(-1);
	}

	int frames, paquets;
    std::istringstream tmp1( argv[1] );
	if (!((tmp1 >> frames) && tmp1.eof())) 		exit(-2); 
    std::istringstream tmp2( argv[2] );
	if (!((tmp2 >> paquets) && tmp2.eof()))		exit(-3); 

	std::cout << "debut du test no_pool" << std::endl;
	std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
	number t;

	//simulation de  frames
	for(int p=0; p<frames; p++) {
	    ThreadPool pool(4);
		std::vector< std::future<bool> > results;
		for(int i = 0; i < paquets; ++i) {
			//recherche le nombre de diviseur le plus important dans un intervalle de NB_VALUE
	        results.emplace_back( pool.enqueue(number::static_myJob, &t, i) );
	    }

		//ceci force le pool a tout traiter avant de passer à la suite
	    for(auto && result: results)
	        result.get();

	    results.clear();
	}
	end = std::chrono::system_clock::now();
    int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
    std::cout << "elapsed time: " << elapsed_milliseconds << " ms\n";

    return 0;
}
