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

	//NameInHtml(S_Name);
	ArgumentInHtml(S_Argument);
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

void TextToHtml::NameInHtml(std::string lines) {
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

void TextToHtml::ArgumentInHtml(std::string lines) {
	int nbArg = 1; //Compteur de nombre de ARGUMENT, pour la bonne mise en forme
	int nbParam = 1; //Compteur de nombre de paramètre @, pour la bonne mise en forme
	std::string delimiter = "\n"; //Défini le délimiter de fin de ligne
	
	ToHtml += "<section class=\"listearguments\">\n<h3>Argument</h3>\n<ol>\n";

	while(lines != ""){
		if(lines.substr(0, lines.find(" ")) == "ARGUMENT") { //On verifie la nature du premier mot.
			if(nbArg != 1){ //On vérifie si c'est la premoière liste d'argument, on non.
				ToHtml += "</li>\n</ul>\n</section>\n";
				nbParam =1; //On remet le compteur a 1, par sécurité
			}
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot ARGUMENT
			ToHtml += "<li>\n<h4>\n";
			ToHtml += "<code class=\"argument\">" + lines.substr(0, lines.find(" ")) + "</code> : "; //On récupère l'argument avant @
			lines = lines.erase(0, lines.find("@")+2); //On supprime le premier paramètre + @
			ToHtml += "<code class=\"argumenttype\">" + lines.substr(0, lines.find(delimiter)) + "</code>\n"; //On récupère l'argument après @
			ToHtml += "</h4>\n<section class=\"listevaleurs\">\n<ul>\n";
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
			nbArg++;
		}
		else if(lines.substr(0, lines.find(" ")) == "	$"){
			if(nbParam != 1){
				ToHtml += "</li>\n";
				nbParam = 1; //On remet le compteur a 1
			}
			ToHtml += "<li>\n";
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot $
			ToHtml += "<code class=\"valeur\">" + lines.substr(0, lines.find(delimiter)) + "</code>\n"; //On récupère l'argument
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
			nbParam++;
		}
		else if(lines.substr(0, lines.find(" ")) == "	@"){
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot @
			if(nbParam == 2){
				ToHtml += "<p class=\"description\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
				nbParam++;
			} else {
				ToHtml += "<p class=\"particularite\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
			}
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
		}
	}

	ToHtml += "</li>\n</ul>\n</section>\n</ol>\n</section>";
}

void TextToHtml::ParameterInHtml(std::string lines) {
	
}

void TextToHtml::ExempleInHtml(std::string lines) {
	
}