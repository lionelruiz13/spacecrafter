#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

class FileReader {
public:
    FileReader();
	~FileReader();
    
    //Ouvre les fichiers et alloue le buffer de la ligne courante
    std::string init(std::string source, std::string destination);    
    //Termine le programme en fermant les fichiers
    void end(int code);                            

    //Place le poineur à la ligne suivante du fichier
    std::string nextline();                               
    //Ignore tous les espaces et tabulations
    std::string noblank(std::string line);                      
    //Ignore tous les espaces et les tabulations ainsi que les lignes qui ne contiennent que ça
    std::string noblanknorline(std::string line);               

private:
    FILE* rstream;      //Flux de lecture
    FILE* wstream;      //Flux d'écriture
    std::string realline;     //Constante contenant la position initiale du pointeur du buffer
};

#endif