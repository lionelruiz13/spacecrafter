#include "FileReader.hpp"

FileReader::FileReader(std::string tf, std::string cf)
{
	loadFile(tf);
	Text_File = tf;
	loadFile(cf);
	Css_File = cf;
}
FileReader::~FileReader() {}

std::vector<std::string> FileReader::getText()
{
	return text;
}

void FileReader::loadFile(const std::string& fileName)
{
	std::ifstream fichier(fileName); //we open the file

	if( fichier.fail() ) { // On check if the opening went well
		std::cout << "File " + fileName + " non-existent or not readable.\n" << std::endl;
		exit(-1);
	}
}

void FileReader::readFileText()
{
	std::ifstream monFlux(Text_File);
	std::string ligne;
	std::string result = "";

	if(monFlux) { // if the file is well opened, we start the processing
		while(getline(monFlux, ligne)) { // We cut the file in a vector
			if(ligne.substr(0,4) == "NAME" && result != "") { // We want to recover a block, so from NAME to @@
				text.push_back(result);
				result = ligne + "\n";
			}
			else {
				result += ligne + "\n";
			}
		}
	}
	else {
		std::cout << "ERROR: Unable to open the .txt file for reading." << std::endl;
		exit(-3);
	}
}

std::string FileReader::readFileCss()
{

	std::ifstream monFlux(Css_File);
	std::string ligne;
	std::string result = "";

	if(monFlux) { // si le fichier est bien ouvert, on commence la lecture
		while(getline(monFlux, ligne)) {
			result += ligne + "\n";
		}
	}
	else {
		std::cout << "ERROR: Unable to open the .css file for reading." << std::endl;
		exit(-2);
	}

	return result;
}
