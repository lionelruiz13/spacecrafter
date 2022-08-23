#include "SpecialArray.hpp"
#include <stdlib.h>
#include <iostream>

void print_tableau(SpecialArray<int> &tableau){
	for(int i = 0; i < tableau.size(); i++) std::cout << "table[" << i << "] = " << tableau[i] << std::endl;
	std::cout << std::endl;
}

int main() {
	SpecialArray<int> tableau(10);
	
	tableau[5] = 5;
	print_tableau(tableau);
	
	tableau.push_front(1);
	print_tableau(tableau);
	
	tableau.push_front() = 2;
	print_tableau(tableau);
	
	tableau.push_front(3);
	print_tableau(tableau);
	
	tableau.push_back(7);
	print_tableau(tableau);
	
	tableau.push_back() = 8;
	print_tableau(tableau);
	
	tableau.push_back(9);
	print_tableau(tableau);
}
