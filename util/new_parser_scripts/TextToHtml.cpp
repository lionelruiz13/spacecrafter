#include "TextToHtml.hpp"
#include "DefineFile.hpp"

TextToHtml::TextToHtml(std::vector<std::string> _text) {
	inText = _text;
	lecture();
}

TextToHtml::~TextToHtml() {}

void TextToHtml::lecture() {
	for(auto i = 0; i < inText.size(); i++) {
	 	transformation(inText[i]);
	}
}

void TextToHtml::transformation(std::string line){
	std::string _textToHtml;


}
