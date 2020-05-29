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
    std::ifstream fichier(fileName); 

    if( !fichier.fail() )
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
        while(getline(monFlux, ligne))
        {
            if(ligne.substr(0,4) == "NAME" && result != "") {
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

void FileReader::readVector() {
	
	// for(auto i = 0; i < text.size(); i++) {
	// 	std::cout << text[i] << std::endl ;
	// }

	std::cout << text[0] << std::endl ;

}