#include "FileWriter.hpp"
#include "DefineFile.hpp"

FileWriter::FileWriter(std::string _outFile)
{
	loadFile(_outFile);
	outFile = _outFile;
}

FileWriter::~FileWriter() {}

void FileWriter::loadFile(const std::string& fileName)
{
	std::ofstream fichier(fileName); //On ouvre le fichier

	if(fichier) { //On teste si tout est OK
		std::cout << "Fichier " + fileName + " existant et charger.\n" << std::endl;
	}
	else {
		std::cout << "ERREUR: Impossible d'ouvrir le fichier .html en écriture." << std::endl;
	}
}

void FileWriter::writeInFile(const std::string inHtml)
{
	std::ofstream fichier(outFile);

	if(fichier) { //On teste si tout est OK
		fichier << inHtml << std::endl;
	}
	else {
		std::cout << "ERREUR: Impossible d'ouvrir le fichier." << std::endl;
	}
}