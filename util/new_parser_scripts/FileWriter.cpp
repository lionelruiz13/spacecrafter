#include "FileWriter.hpp"
//#include "DefineFile.hpp"

FileWriter::FileWriter(std::string _outFile)
{
	loadFile(_outFile);
	outFile = _outFile;
}

FileWriter::~FileWriter() {}

void FileWriter::loadFile(const std::string& fileName)
{
	std::ofstream fichier(fileName); // We open the file

	if(!fichier) { // We test if everything is OK
		std::cout << "ERROR: Unable to open the .html file for writing." << std::endl;
	}
}

void FileWriter::writeInFile(const std::string inHtml)
{
	std::ofstream fichier(outFile);

	if(fichier) { // We test if everything is OK
		fichier << inHtml << std::endl;
	}
	else {
		std::cout << "ERROR: Unable to write to the file." << std::endl;
	}
}