#include <string>
#include <iostream>

#include "TextToHtml.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
//#include "DefineFile.hpp"

int main()
{

	//Declaration of input and output files
	std::string source = "input_fr.txt";
	std::string sourceCSS = "style.css";
	std::string destination = "resultat.html";

	//Read input files
	FileReader* reader = new FileReader(source, sourceCSS);
	reader->readFileText();

	//Reading the output file
	FileWriter* writer = new FileWriter(destination);

	//Transformation of txt file to html
	TextToHtml* parser = new TextToHtml(reader->getText(), reader->readFileCss());

	writer->writeInFile(parser->getHtml());

	//std::cout << "DONE" << std::endl;

	return 0;
}