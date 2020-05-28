#ifndef TEXT_TO_HTML_HPP
#define TEXT_TO_HTML_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
// #include "FileWriter.hpp"
// #include "FileReader.hpp"

class FileReader;
class FileWriter;

class TextToHtml {
public:
    TextToHtml();
	~TextToHtml();
    
    //Renvoi l'entier associé à chaque balise
    int checktag(std::string line);                               
    //Traite la balise NAME
    std::string name(std::string line,std::string* nametab, int  namenum);    
    //Traie la partie description et particularité des balises
    std::string descpart(std::string line);                             
    //Traite la balise ARGUMENT
    std::string argument(std::string line);                             
    //Traite la balise EXEMPLE qui est un cas particulier car on peut tout mettre dedans sauf @@ qui est la balise de fermeture
    std::string example(std::string line);                              
    //Traite la balise IMG qui est aussi un cas particulier car 
    std::string img(std::string line);                                  

private:
    FileReader* Reader;
    FileWriter* Writer;
};

#endif