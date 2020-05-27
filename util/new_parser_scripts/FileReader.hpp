#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <stdio.h>
#include <stdlib.h>

class FileReader {
public:
    FileReader();
	~FileReader();
    
    char* init(char* source, char* destination);    //Ouvre les fichiers et alloue le buffer de la ligne courante
    void end(int code);                             //Termine le programme en fermant les fichiers

    char* nextline();                               //Place le poineur à la ligne suivante du fichier
    char* noblank(char* line);                      //Ignore tous les espaces et tabulations
    char* noblanknorline(char* line);               //Ignore tous les espaces et les tabulations ainsi que les lignes qui ne contiennent que ça

private:
    FILE* rstream;      //Flux de lecture
    FILE* wstream;      //Flux d'écriture
    char* realline;     //Constante contenant la position initiale du pointeur du buffer
};

#endif