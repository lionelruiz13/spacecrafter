#include <iostream>
#include <vector>
#include <chrono>

#include <unistd.h>
#include "tools/ThreadPool.hpp"
#include <sstream>


class number {
public:
	int recherche_diviseurs (int a);
	bool myJob(int i);
	static bool static_myJob(number *a,int i) { return a->myJob(i); }
};


int number::recherche_diviseurs (int a)
{
  int i, j;
  j = 0;
  for (i = 1; i < a + 1; i++)
    {
      if ((a % i) == 0)
	{
	  //printf (" %i", i);
	  j = j + 1;
	}
    }
  return j;
}

#define NB_VALUE 1000



bool number::myJob(int i)
{
	int j=0;
	int tmp;
	//~ std::cout << "task " << i << std::endl;
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
	if (!((tmp1 >> frames) && tmp1.eof())) // Check eofbit
		exit(-2); 
    std::istringstream tmp2( argv[2] );
	if (!((tmp2 >> paquets) && tmp2.eof())) // Check eofbit
		exit(-3); 
		
	std::cout << "debut du test pool" << std::endl;

	std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    ThreadPool pool(4);
	std::vector< std::future<bool> > results;

	number t;

	//simulation de  frames
	for(int p=0; p<frames; p++) {


    
	    for(int i = 0; i < paquets; ++i) {
	        results.emplace_back( //recherche le nombre de diviseur le plus important dans un intervalle de NB_VALUE
	            pool.enqueue(number::static_myJob, &t, i)
	            //~ pool.enqueue([i] {
	                //~ int j=0;
	                //~ int tmp;
	                //~ //std::cout << "task " << i << std::endl;
	                //~ for(int k=NB_VALUE*i; k<NB_VALUE*(i+1); k++) {
						//~ tmp=recherche_diviseurs(k);
						//~ if (tmp>j)
							//~ j=tmp;
					//~ }
	                //~ return true;
	            //~ })
	        );
	    }
	
		//~ std::cout << "debut" << std::endl;
		//ceci force le pool a tout traiter avant de passer à la suite
	    for(auto && result: results)
	        //std::cout << 
	        result.get();// << ' ';
	    //~ std::cout << std::endl;
	    //~ std::cout << "fin" << std::endl;

	    //~ std::cout << "attente..." << std::endl;
	    results.clear();
	    //~ sleep(1);
	}

	end = std::chrono::system_clock::now();

    int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
 
    std::cout << "elapsed time: " << elapsed_milliseconds << " ms\n";

    return 0;
}
