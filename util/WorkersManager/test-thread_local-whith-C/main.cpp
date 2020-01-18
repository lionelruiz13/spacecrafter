//Compilation : g++ main.cpp print_id.c --std=c++11 -pthread -o test

#define __main__

#include <thread>
#include "threads.h"
#include "print_id.h"

void thread(const unsigned int ID) {
	id = ID;
	printid();
}

int main()
{
	std::thread a(thread, 1);
	a.join();
	printid();
}
