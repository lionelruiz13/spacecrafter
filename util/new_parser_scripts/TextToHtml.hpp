#ifndef TEXT_TO_HTML_HPP
#define TEXT_TO_HTML_HPP

#include <stdio.h>
#include <stdlib.h>
// #include "FileWriter.hpp"
// #include "FileReader.hpp"

class FileReader;
class FileWriter;

class TextToHtml {
public:
    TextToHtml();
	~TextToHtml();
    
    int checktag(char* line);                               //Renvoi l'entier associé à chaque balise
    char* name(char* line,char** nametab, int  namenum);    //Traite la balise NAME
    char* descpart(char* line);                             //Traie la partie description et particularité des balises
    char* argument(char* line);                             //Traite la balise ARGUMENT
    char* example(char* line);                              //Traite la balise EXEMPLE qui est un cas particulier car on peut tout mettre dedans sauf @@ qui est la balise de fermeture
    char* img(char* line);                                  //Traite la balise IMG qui est aussi un cas particulier car 

private:
    FileReader* Reader;
    FileWriter* Writer;
};

#endif