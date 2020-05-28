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

	for(auto i = 0; i < inText.size(); i++) {
		bloc = inText[i]; //On charge le string courrant

		//On veux lire, ligne par ligne, pour le traitement des données. 
		while(!bloc.empty()) { //Tant que la chaine de caractère n'est pas vide, on continue
			tempText.push_back(bloc.substr(0, bloc.find(delimiter))); //On place la ligne dans un vecteur
			bloc.erase(0, bloc.find(delimiter) + delimiter.length()); //on supprime la ligne trouv"
		}
		transformation(tempText); //On transmet le vecteur, pour la transformation

		tempText.clear(); //On supprime le vecteur, et on recommence avec un nouveau bloc
	}
}

void TextToHtml::transformation(std::string line){
	std::string _textToHtml;


}
