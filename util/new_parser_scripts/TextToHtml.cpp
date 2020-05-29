#include "TextToHtml.hpp"
#include "DefineFile.hpp"

TextToHtml::TextToHtml(std::vector<std::string> _text) {
	inText = _text;
	lecture();
}

TextToHtml::~TextToHtml() {}

void TextToHtml::lecture() {
	//sert a chercher les lignes dans le string courrant
	std::string delimiter = "\n"; 	//Délimiteur de ligne
	std::string bloc;				//le string courrant

	for(auto i = 0; i < 1; i++) {		
		transformation(inText[2]); //On transmet le string courant, pour la transformation
	}
}

void TextToHtml::transformation(std::string lines){
	std::string _textToHtml = "";
	std::string delimiter1 = "\n"; 	//Délimiteur de ligne
	std::string delimiter2 = " "; 	//Délimiteur de mot
	std::string tempText = "";			
	std::string argument = "NAME";	//Garde en mémoire l'argument, par defaut, c'est NAME est premier.
	std::string nextargument = "";

	_textToHtml += "<article id=" + lines.substr(5, lines.length()) + ">"; // on récupère la première information, qui défini le nom du bloc

	while(argument != "END"){

		nextargument = findBloc(lines, argument);

		tempText = lines.substr(0, lines.find(nextargument));
		
		lines.erase(0, lines.find(nextargument));

		std::cout << "------" + argument + "------\n" << tempText << std::endl;

		argument = nextargument ;

	}

	

	//std::cout << lines << std::endl;
}

std::string TextToHtml::findBloc(std::string& lines, std::string arg){
	
	if(arg == "NAME") { // BLOC NAMES
		//On cherche le prochain argument
		if(lines.find("ARGUMENT")!=std::string::npos) return "ARGUMENT";
		if(lines.find("PARAMETER")!=std::string::npos) return "PARAMETER";
		if(lines.find("EXEMPLE")!=std::string::npos) return "EXEMPLE";
	}

	if(arg == "ARGUMENT") { // BLOC ARGUMENT
		//On cherche le prochain argument
		if(lines.find("PARAMETER")!=std::string::npos) return "PARAMETER";
		if(lines.find("EXEMPLE")!=std::string::npos) return "EXEMPLE";
	}

	if(arg == "PARAMETER") { // BLOC PARAMETER
		//On cherche le prochain argument
		if(lines.find("EXEMPLE")!=std::string::npos) return "EXEMPLE";
	}

	return "END";
}