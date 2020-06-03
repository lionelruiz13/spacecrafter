#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "TextToHtml.hpp"

class FileReader {
public:
    FileReader(std::string tf, std::string cf);
	~FileReader();
    
    /**
     * Permet d'instancier le fichier .txt dans le vecteur
     */
    void readFileText();
    /**
     * Permet d'instancier le fichier .css dans le string
     */
    std::string readFileCss();

    std::vector<std::string> getText();

private:
    /**
     * Charge les fichiers
     */
    void loadFile(const std::string& fileName);

    //Fichier à traiter
    std::string Text_File;
    std::string Css_File;

    //Vecteur de sortie, pour un traitement dans TextToHtml
    std::vector<std::string> text;
};

#endif