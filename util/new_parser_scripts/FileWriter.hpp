#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <stdio.h>
#include <stdlib.h>
// #include "FileReader.hpp"
// #include "TextToHtml.hpp"

class FileReader;
class TextToHtml;

class FileWriter {
public:
    FileWriter();
	~FileWriter();
    
    void writetext(char* text);                                         //Ecrit du texte dans le fichier
    void writenchar(char* line, int  n);                                //Ecrit les n premier caractères
    void writenewline();                                                //Saute une ligne en html
    void writetextnchar(char* begin, char* line, int  n, char* end);    //Permet d'écrire autour de writenchar(line, n)
    char* writehtml(char* line);                                        //Ecrit dans le html et s'arrête à un des deux séparateur ou une balise après un saut de ligne (n'écrit pas les espaces après la balise et avant la fin)
    char* writetexthtml(char* begin, char* line, char* end);            //Permet d'écrire autour de writehtml(line)
    char* writetextdescpart(char* begin, char* line, char* end);        //Permet d'écrire autour de writetextdescpart(line)
    int argclose(int argopen);                                          //Ferme une liste d'arguments ouverte

private:
    FILE* rstream;      //Flux de lecture
    FILE* wstream;      //Flux d'écriture
    FileReader* Reader;
    TextToHtml* text;
};

#endif