#include "FileReader.hpp"

FileReader::FileReader(std::string tf, std::string cf) {
	loadFile(tf);
	Text_File = tf;
	loadFile(cf);
	Css_File = cf;
}
FileReader::~FileReader() {}

std::vector<std::string> FileReader::getText() {
	return text;
}

void FileReader::loadFile(const std::string& fileName) {
    std::ifstream fichier(fileName); //On ouvre le fichier

    if( !fichier.fail() ) //On vérifier si l'ouverture s'est bien déroulé
    {  
        std::cout << "Fichier " + fileName + " existant et charger.\n" << std::endl;  
    }  
    else 
    {  
        std::cout << "Fichier " + fileName + " inexistant ou non lisible.\n" << std::endl;
    }
}

void FileReader::readFileText() {
	std::ifstream monFlux(Text_File);
	std::string ligne;
	std::string result = "";

    if(monFlux) { // si le fichier est bien ouvert, on commence le traitement
        while(getline(monFlux, ligne)) //On découpe le fichier dans un vecteur
        {
            if(ligne.substr(0,4) == "NAME" && result != "") { //On veux récupérer un bloc, donc de NAME à @@
				text.push_back(result);
				result = ligne + "\n";
			}
			else {
				result += ligne + "\n";
			}
        }
    }
    else {
        std::cout << "ERREUR: Impossible d'ouvrir le fichier .css en lecture." << std::endl;
    }
}

std::string FileReader::readFileCss() {
    
    std::ifstream monFlux(Css_File);
	std::string ligne;
	std::string result = "";

    if(monFlux) { // si le fichier est bien ouvert, on commence la lecture
        while(getline(monFlux, ligne))
        {
            result += ligne;
        }
    }
    else {
        std::cout << "ERREUR: Impossible d'ouvrir le fichier .css en lecture." << std::endl;
    }

	return result;
}