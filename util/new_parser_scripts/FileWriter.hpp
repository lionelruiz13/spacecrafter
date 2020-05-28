#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
// #include "FileReader.hpp"
// #include "TextToHtml.hpp"

class FileReader;
class TextToHtml;

class FileWriter {
public:
    FileWriter();
	~FileWriter();
    
    //Ecrit du texte dans le fichier
    void writetext(std::string text);
    //Ecrit les n premier caractères                                         
    void writenchar(std::string line, int  n);                                
    //Saute une ligne en html
    void writenewline();                                                
    //Permet d'écrire autour de writenchar(line, n)
    void writetextnchar(std::string begin, std::string line, int  n, std::string end);    
    //Ecrit dans le html et s'arrête à un des deux séparateur ou une balise après un saut de ligne (n'écrit pas les espaces après la balise et avant la fin)
    std::string writehtml(std::string line);                                        
    //Permet d'écrire autour de writehtml(line)
    std::string writetexthtml(std::string begin, std::string line, std::string end);            
    //Permet d'écrire autour de writetextdescpart(line)
    std::string writetextdescpart(std::string begin, std::string line, std::string end);        
    //Ferme une liste d'arguments ouverte
    int argclose(int argopen);                                          

private:
    FILE* rstream;      //Flux de lecture
    FILE* wstream;      //Flux d'écriture
    FileReader* Reader;
    TextToHtml* text;
};

#endif