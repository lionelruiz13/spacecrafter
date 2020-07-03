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
    /// note ces deux lignes qui t'empèche de faire des copies implicites d'objets
    A(A const &) = delete;
	A& operator = (A const &) = delete;
    /// cf spacecrafter tools/No_copy.hpp
    int getNbr() {return nbr;}
    void setNbr(int i) {nbr=i;}
private:
    int nbr;
};




/// mais ou est le makefile ? :)


/// cette fonction ne devrait elle pas être statique ?
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

/// pour la suite ..., note le mot static.
static bool isImpair(const std::unique_ptr<A> &a) {
	return a->getNbr()%2==1;
}

int main(int argc, char **argv)
{
    std::vector<std::shared_ptr<A>> myVector; /// pourquoi shared_ptr ?? unique_ptr était tout désigné
    std::shared_ptr<A> tmp = nullptr;

    //creation avec new
    for(int i=0;i<7; i++)
    myVector.push_back(std::make_shared<A>(i)); /// très bien le make_shared

    // affichage du vector
    printVector(myVector);

    //suppression des éléments impairs
    for(auto it = myVector.begin(); it !=myVector.end(); ) { // plus de it++ on l'incrémente nous à la suite de la boucle
    if ((*it)->getNbr() & 1) {
        it= myVector.erase(it);	// je détruis it. erase renvoie l'adresse de l'élément suivant:
    } else
        ++it;
    }

    /// oui mais on peut faire plus "simple"
	// suppression des éléments impairs avec une belle lambda fonction
	//myVector.erase(std::remove_if(myVector.begin(), myVector.end(), [](std::unique_ptr<A> &a) { return a->getNbr()%2==1; } ), myVector.end());

	// la même chose sans lambda fonction: ce n'est donc qu'une rapidité d'écriture
	//myVector.erase(std::remove_if(myVector.begin(), myVector.end(), isImpair), myVector.end());

    /// dans le cadre d'un vecteur, le remove_if est associé à erase
    /// en effet, il est beaucoup plus simple de déplacer les élements à supprimer à la fin du vecteur puis je supprimer physiquement la fin du vecteur
    /// regarde l'idiome remove-erase
    /// https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
    ///
    /// En voici la raison profonde:
    ///
    /// Doing the same using only erase results in as many passes as there are elements to remove. For each of these passes, 
    /// all elements after the erased element have to be moved, which is more time-consuming than shifting elements in a single pass. 

    // affichage du vector
    printVector(myVector);

    // recherche d'un élément particulier i=4
    for(auto it = myVector.begin(); it !=myVector.end(); it++) {
        if ((*it)->getNbr()==4) {
            tmp = (*it);            /// si on déclare A* tmp; on peut très bien écrire tmp = (*it).get(); en s'interdisant de faire un delete tmp plus tard
            break;
        }
    }

    // plus tard ... modification de l'élément tmp
    if (tmp != nullptr) {
        tmp->setNbr(3);
    } else {
        std::cout << "ERR\n";
    }

    // affichage du vector
    printVector(myVector);

    //suppression totale
    for(auto it = myVector.begin(); it !=myVector.end(); ) {
        it=myVector.erase(it);
    }
    /// je pencherai plus pour cette ligne car justement on profite des smartpointers
    /// myVector.clear();

    std::cout << "Taille du vecteur : " << myVector.size() << std::endl;

    printVector(myVector);

    //pour le fun, inutile certes.
    //myVector.clear();

    return 0;
}
