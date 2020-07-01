#include "TextToHtml.hpp"
#include "DefineFile.hpp"

TextToHtml::TextToHtml(std::vector<std::string> _text, std::string _css)
{
	inText = _text;
	inCss = _css;
	OutHtml = "";
	lecture();
}

TextToHtml::~TextToHtml() {}

std::string TextToHtml::getHtml()
{
	return OutHtml;
}

void TextToHtml::lecture()
{
	OutHtml += "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>Documentation</title>\n<style>";
	OutHtml += inCss; //On place le style CSS
	OutHtml += "</style>\n</head>\n<body>\n<header>\n<h1>Documentation des commandes script du logiciel</h1>\n</header>\n";
	//OutHtml += "<img src=\"logo.png\" alt=\"Logo\" class=\"logo\">";
	OutHtml += "<section class=\"commande\">";

	for(auto i = 0; i < inText.size(); i++) {
		transformation(inText[i]); //On transmet le string courant, pour la transformation
	}

	OutHtml += "</section>\n";
	OutHtml += "<aside id=\"Menu\" class=\"menu\">\n";
	OutHtml += "<h3>Index</h3>\n<ol>\n";
	OutHtml += index; //On place l'index
	OutHtml += "</ol>\n</aside>\n</body>\n</html>";
}

void TextToHtml::transformation(std::string lines)
{
	std::string tempText = ""; //String temp pour le traitement
	std::string argument = "NAME";	//Garde en mémoire l'argument, par defaut, c'est NAME est premier.
	std::string nextargument = "";  //Garde en mémoire le prochain argument

	//String pour chaque "bloc" d'argument
	std::string S_Name = "";
	std::string S_Argument = "";
	std::string S_Parametre = "";
	std::string S_Exemple = "";

	lines = lines.erase(lines.find("@@"), lines.length()); //On supprime la partie inutile

	while(argument != "END") { //Tant qu'il existe un argument valide, on découpe le string

		nextargument = findBloc(lines, argument); //On cherche si on est dans un NAME / ARGUMENT / PARAMETER / EXEMPLE

		tempText = lines.substr(0, lines.find(nextargument)); //on garde en mémoire la partie traitée

		lines.erase(0, lines.find(nextargument)); //On supprime la partie traitée

		if(argument == "NAME") S_Name += tempText;
		if(argument == "ARGUMENT") S_Argument += tempText;
		if(argument == "PARAMETER") S_Parametre += tempText;
		if(argument == "EXEMPLE") S_Exemple += tempText;

		argument = nextargument; //On passe au Prochain argument  NAME / ARGUMENT / PARAMETER / EXEMPLE
	}

	//On envoie en traitement toutes les sections, si elles sont présente.
	if(S_Name != "") NameInHtml(S_Name);
	if(S_Argument != "") ArgumentInHtml(S_Argument);
	if(S_Parametre != "") ParameterInHtml(S_Parametre);
	if(S_Exemple != "") ExempleInHtml(S_Exemple);

	index += "<li><a href=\"#" + title + "\"><code>" + title +"</code></a></li>\n"; //instanciation de l'Index

	OutHtml += "</article>\n";
	OutHtml += "<a href=\"#Menu\" class=\"retour\">Retour à l'index</a>\n";
}

std::string TextToHtml::findBloc(std::string lines, std::string arg)
{

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

void TextToHtml::NameInHtml(std::string lines)
{
	std::string delimiter = "\n"; //Défini le délimiteur de fin de ligne
	title = lines.substr(5, lines.find(delimiter)-5); //On récupère le premier argument après NAME, qui définit le titre du bloc
	int i = 1; //Entier permettant de savoir s'il y a 2 ligne a traitée ou pas, car les 2 ligne de texte sont défini différement dans le fichier HTML

	OutHtml += "<article id=\"" + title + "\">\n";
	OutHtml += "<header>\n";
	OutHtml += "<h2><code>" + title + "</code></h2>\n";

	lines = lines.erase(0, lines.find(delimiter)+4); //On supprime la première ligne + la tabulation et le preier repère de la seconde ligne

	while(lines != "") {
		if(i == 1) {
			OutHtml += "<p class =\"description\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n"; //On récupère la ligne, donc du premier caractère, au saut de ligne.
			lines = lines.erase(0, lines.find(delimiter)+4); //On supprime la ligne + la tabulation  et le premier repère de la seconde ligne
			i++;
		}
		else {
			OutHtml += "<p class =\"particularite\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n"; //On récupère la ligne, donc du premier caractère, au saut de ligne.
			lines = lines.erase(0, lines.find(delimiter)+4); //On vide le string NAME en cours
		}
	}
	OutHtml += "</header>";
}

void TextToHtml::ArgumentInHtml(std::string lines)
{
	//Paramètre entier pour savoir si on est en début ou en fin de bloc (ex bloc: <li> ... </li>)
	int nbArg = 1; //Compteur de nombre de ARGUMENT, pour la bonne mise en forme
	int nbParam = 1; //Compteur de nombre de paramètre @, pour la bonne mise en forme
	std::string delimiter = "\n"; //Défini le délimiteur de fin de ligne

	OutHtml += "<section class=\"listearguments\">\n<h3>Argument</h3>\n<ol>\n";

	while(lines != "") {
		if(lines.substr(0, lines.find(" ")) == "ARGUMENT") { //On verifie la nature du premier mot.
			if(nbArg != 1) { //On vérifie si c'est la premoière liste d'argument, on non.
				OutHtml += "</li>\n</ul>\n</section>\n";
				nbParam =1; //On remet le compteur a 1, par sécurité
			}
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot ARGUMENT
			OutHtml += "<li>\n<h4>\n";
			OutHtml += "<code class=\"argument\">" + lines.substr(0, lines.find(" ")) + "</code> : "; //On récupère l'argument avant @
			lines = lines.erase(0, lines.find("@")+2); //On supprime le premier paramètre + @
			OutHtml += "<code class=\"argumenttype\">" + lines.substr(0, lines.find(delimiter)) + "</code>\n"; //On récupère l'argument après @
			OutHtml += "</h4>\n<section class=\"listevaleurs\">\n<ul>\n";
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
			nbArg++; //On indique, si besoin, qu'on va reçevoir un nouveau bloc Argument dans le string courrant
		}
		else if(lines.substr(0, lines.find(" ")) == "	$") {
			if(nbParam != 1) {
				OutHtml += "</li>\n";
				nbParam = 1; //On remet le compteur a 1
			}
			OutHtml += "<li>\n";
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot $
			OutHtml += "<code class=\"valeur\">" + lines.substr(0, lines.find(delimiter)) + "</code>\n"; //On récupère l'argument
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
			//nbParam++;
		}
		else if(lines.substr(0, lines.find(" ")) == "	@") {
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot @
			if(nbParam == 1) {
				OutHtml += "<p class=\"description\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
				nbParam++;
			}
			else {
				OutHtml += "<p class=\"particularite\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
				nbParam++;
			}
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
		}
	}

	OutHtml += "</li>\n</ul>\n</section>\n</ol>\n</section>";
}

void TextToHtml::ParameterInHtml(std::string lines)
{
	//Paramètre entier pour savoir si on est en début ou en fin de bloc (ex bloc: <li> ... </li>)
	int nbParam = 1; //Compteur de nombre de paramètre @, pour la bonne mise en forme
	std::string delimiter = "\n"; //Défini le délimiter de fin de ligne

	OutHtml += "<section class=\"listeparameters\">\n<h3>Paramètre</h3>\n<ul>\n";

	while(lines != "") {
		if(lines.substr(0, lines.find(" ")) == "PARAMETER") { //On verifie la nature du premier mot.
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot PARAMETER
			OutHtml += "<li>\n<h4>\n";
			OutHtml += "<code class=\"parameter\">" + lines.substr(0, lines.find(" ")) + "</code> : "; //On récupère l'argument avant @
			lines = lines.erase(0, lines.find("@")+2); //On supprime le premier paramètre + @
			OutHtml += "<code class=\"parametertype\">" + lines.substr(0, lines.find(delimiter)) + "</code>\n"; //On récupère l'argument après @
			OutHtml += "</h4>\n</li>\n";
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
			nbParam = 1;
		}
		else if(lines.substr(0, lines.find(" ")) == "	$") {
			OutHtml += "<li>\n";
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot $
			OutHtml += "<code class=\"valeur\">" + lines.substr(0, lines.find(delimiter)) + "</code>\n</li>\n"; //On récupère l'argument
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
		}
		else if(lines.substr(0, lines.find(" ")) == "	@") {
			lines = lines.erase(0, lines.find(" ")+1); //On enlève le mot @
			if(nbParam == 1) {
				OutHtml += "<p class=\"description\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
				nbParam++;
			}
			else {
				OutHtml += "<p class=\"particularite\">" + lines.substr(0, lines.find(delimiter)) + "</p>\n";
			}
			lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne
		}
	}

	OutHtml += "</ul>\n</section>";

}

void TextToHtml::ExempleInHtml(std::string lines)
{
	std::string delimiter = "\n"; //Défini le délimiteur de fin de ligne
	lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la premiere ligne + la tabulation
	OutHtml += "<section class=\"exemple\">\n<h2>Exemple</h2>\n<pre>";
	while(lines != "") {
		OutHtml += lines.substr(1, lines.find(delimiter));
		lines = lines.erase(0, lines.find(delimiter)+1); //On supprime la ligne + la tabulation
	}
	OutHtml += "</pre>\n</section>";
}
