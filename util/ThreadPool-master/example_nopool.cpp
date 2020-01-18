#include <iostream>
#include <vector>
#include <chrono>

#include <unistd.h>
//~ #include "tools/ThreadPool.hpp"

#include "parallelized_for.hpp"
#define NB_VALUE 625


class number {
public:
	int recherche_diviseurs (int a);
	bool myJob(int i);
	static bool static_myJob(int i, int j, number *a) { return a->myJob(j); }
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


int main()
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

	number t;

	//simulation de 30 frames
	for(int p=0; p<30; p++) {

		std::cout << "debut" << std::endl;

		ForParallelizer<void> fp;
		fp.compute(0,80,number::static_myJob, ?);

	    std::cout << "fin" << std::endl;
	}

	end = std::chrono::system_clock::now();

    int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
 
    std::cout << "elapsed time: " << elapsed_milliseconds << " ms\n";

    return 0;
}
