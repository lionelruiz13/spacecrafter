#ifndef OBJET_HPP
#define OBJET_HPP

#include<iostream>

class Objet {

public:
	Objet(std::string n): name(n) {}
	~Objet() {}

    void update() {
        std::cout << "update de " + name << std::endl;
    }

	void draw(){
        std::cout << "draw de " + name << std::endl;
    }

    Objet getObjet(){
        return name;
    }

    void setObjet(std::string newName){
        name = newName;
    }

private:
    std::string name;

};

#endif