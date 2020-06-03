#include "TextToHtml.hpp"
#include "DefineFile.hpp"

TextToHtml::TextToHtml(std::vector<std::string> _text) {
	inText = _text;
	ToHtml = "";
	lecture();
}

TextToHtml::~TextToHtml() {}

void TextToHtml::lecture() {
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

	NameInHtml(S_Name);
	//std::cout << "--------NAME--------" << std::endl << S_Name << std::endl;
	//std::cout << "------ARGUMENT------" << std::endl << S_Argument << std::endl;
	//std::cout << "------PARAMETER-----" << std::endl << S_Parametre << std::endl;
	//std::cout << "------EXEMPLE-------" << std::endl << S_Exemple << std::endl;

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

std::string TextToHtml::NameInHtml(std::string lines) {
	std::string delimiter = "\n"; //Défini le délimiter de fin de ligne
	std::string arg = lines.substr(5, lines.find(delimiter)-5); //On récupère le premier argument après NAME
	int i = 1;

	ToHtml += "<article id=\"" + arg + "\">\n";
	ToHtml += "<header>\n";
	ToHtml += "<h2><code>" + arg + "</code></h2>\n";

	lines = lines.erase(0, lines.find(delimiter)+4); //On supprime la première ligne + la tabulation et le preier repère de la seconde ligne

	while(lines != ""){
		if(i == 1){
			ToHtml += "<p class =\"description\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
			lines = lines.erase(0, lines.find(delimiter)+4); //On supprime la ligne + la tabulation  et le premier repère de la seconde ligne
			i++;
		} else {
			ToHtml += "<p class =\"particularite\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
			lines = lines.erase(0, lines.length()); //On vide le string
		}
	}
	ToHtml += "</header>";
}

std::string TextToHtml::ArgumentInHtml(std::string lines) {
	
}

std::string TextToHtml::ParameterInHtml(std::string lines) {
	
}

std::string TextToHtml::ExempleInHtml(std::string lines) {
	
}