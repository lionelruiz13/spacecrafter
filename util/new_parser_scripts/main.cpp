#include <string>
#include <iostream>

#include "TextToHtml.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "DefineFile.hpp"

int main()
{

	//Déclaration des fichiers entrée et sortie
	std::string source = "input_fr.txt";
	std::string sourceCSS = "style.css";
	std::string destination = "resultat.html";

	//Lecture des fichier d'entrèe
	FileReader* reader = new FileReader(source, sourceCSS);
	reader->readFileText();

	//Lecture du fichier de sortie
	FileWriter* writer = new FileWriter(destination);

	//Transformation de fichier txt en html
	TextToHtml* parser = new TextToHtml(reader->getText(), reader->readFileCss());

	writer->writeInFile(parser->getHtml());

	std::cout << "DONE" << std::endl;

	return 0;
}