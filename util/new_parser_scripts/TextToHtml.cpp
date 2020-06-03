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
		transformation(inText[i]); //On transmet le string courant, pour la transformation
	}
}

void TextToHtml::transformation(std::string lines){
	std::string tempText = "";			
	std::string argument = "NAME";	//Garde en mémoire l'argument, par defaut, c'est NAME est premier.
	std::string nextargument = "";

	std::string S_Name = "";
	std::string S_Argument = "";
	std::string S_Parametre = "";
	std::string S_Exemple = "";

	while(argument != "END"){

		nextargument = findBloc(lines, argument);

		tempText = lines.substr(0, lines.find(nextargument));
		
		lines.erase(0, lines.find(nextargument));

		//std::cout << "------" + argument + "------\n" << tempText << std::endl;

		if(argument == "NAME") S_Name += tempText;
		if(argument == "ARGUMENT") S_Argument += tempText;
		if(argument == "PARAMETER") S_Parametre += tempText;
		if(argument == "EXEMPLE") S_Exemple += tempText;

		argument = nextargument ;

	}

	std::cout << "--------NAME--------" << std::endl << S_Name << std::endl;
	std::cout << "------ARGUMENT------" << std::endl << S_Argument << std::endl;
	std::cout << "------PARAMETER-----" << std::endl << S_Parametre << std::endl;
	std::cout << "------EXEMPLE-------" << std::endl << S_Exemple << std::endl;

	//std::cout << lines << std::endl;
}

std::string TextToHtml::findBloc(std::string lines, std::string arg) {
	
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