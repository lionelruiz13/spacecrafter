#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include "include/dynamic_printer.h"

class A  // construite pour l'exemple ...
{
public:
    A(int _nbr){ nbr=_nbr; std::cout << "Création de " << nbr << std::endl; }
    ~A(){std::cout << "suppression de " << nbr << std::endl; };
    int getNbr() {return nbr;}
    void setNbr(int i) {nbr=i;}
private:
    int nbr;
};

void printVector(const std::vector<std::shared_ptr<A>> &myVector)
{
    int cadre_size = std::max(8, (int) myVector.size() * 8 | 1);

    draw_cadre(cadre_size, 4);
    move_in_line(cadre_size / 2 - 3);
    std::cout << "Vector\n\e[C";
    for(auto const& l : myVector)
        std::cout << l->getNbr() << "\t";
    std::cout << "\n\n";
}

int main(int argc, char **argv)
{
    std::vector<std::shared_ptr<A>> myVector;
    std::shared_ptr<A> tmp = nullptr;

    //creation avec new
    for(int i=0;i<7; i++)
    myVector.push_back(std::make_shared<A>(i));

    // affichage du vector
    printVector(myVector);

    //suppression des éléments impairs
    for(auto it = myVector.begin(); it !=myVector.end(); ) { // plus de it++ on l'incrémente nous à la suite de la boucle
    if ((*it)->getNbr()%2) {
        it= myVector.erase(it);	// je détruis it. erase renvoie l'adresse de l'élément suivant:
    } else
        ++it;
    }

    // affichage du vector
    printVector(myVector);

    // recherche d'un élément particulier i=4
    for(auto it = myVector.begin(); it !=myVector.end(); it++) {
        if ((*it)->getNbr()==3) {
            tmp = (*it);
            break;
        }
    }
    // plus tard ... modification de l'élément tmp
    if (tmp != nullptr) {
        tmp->setNbr(3);
    }

    // affichage du vector
    printVector(myVector);

    //suppression totale
    for(auto it = myVector.begin(); it !=myVector.end(); ) {
        it=myVector.erase(it);
    }

    std::cout << "Taille du vecteur : " << myVector.size() << std::endl;

    printVector(myVector);

    //pour le fun, inutile certes.
    myVector.clear();

    return 0;
}
