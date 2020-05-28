#include <string>
#include <iostream>

#include "TextToHtml.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "DefineFile.hpp"

int main() {
	
    // FileReader Reader;
    // TextToHtml Text;
    // FileWriter Writer;

    std::string line;

    std::string source = "input_fr.txt";
    std::string sourceCSS = "style.css";
    std::string destination = "resultat.html";

	FileReader* reader = new FileReader(source, sourceCSS);


	reader->readFileText();

	reader->readVector();

    return 0;
}