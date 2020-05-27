#define BUFFER_SIZE 8192
#define NAME_TAB_SIZE 512

//Définition du html

#define HTML_NEWLINE "\n<br/>"

#define BEFORE_STYLE "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>Documentation</title>\n"

#define AFTER_STYLE "</head>\n<body>\n<header>\n<h1>Documentation du logiciel</h1>\n</header>\n<img src=\"img/logo.png\" alt=\"Logo\" class=\"logo\">\n<section class=\"commande\">\n"

#define BEFORE_NAME "<article id=\""
#define AFTER_NAME_ID "\">\n<header>\n<h2><code>"
#define AFTER_NAME "</code></h2>\n"
#define AFTER_NAME_HEAD "</header>\n"

#define BEFORE_DESC "<p class=\"description\">"
#define AFTER_DESC "</p>\n"

#define BEFORE_PART "<p class=\"particularite\">"
#define AFTER_PART "</p>\n"

#define BEFORE_ARGUMENT_LIST "<section class=\"listearguments\">\n<h3>Arguments</h3>\n<ol>\n"
#define BEFORE_VARIABLE "<li>\n<h4><code class=\"argument\">"
#define AFTER_VARIABLE "</code>"
#define BEFORE_TYPE " : <code class=\"type\">"
#define AFTER_TYPE "</code></h4>\n"
#define AFTER_ARGUMENT_WITHOUT_TYPE "</h4>\n"
#define AFTER_ARGUMENT_LIST "</ol>\n</section>\n"

#define BEFORE_VALEUR_LIST "<section class=\"listevaleurs\">\n<ul>\n"
#define BEFORE_VALEUR "<li>\n<code class=\"valeur\">"
#define AFTER_VALEUR "</code>\n"
#define AFTER_VALEUR_ELEMENT "</li>\n"
#define AFTER_VALEUR_LIST "</ul>\n</section>\n"

#define BEFORE_EXEMPLE "<section class=\"exemple\">\n<h2>Exemple</h2>\n<pre>"
#define AFTER_EXEMPLE "</pre>\n</section>\n"

#define BEFORE_IMG "<img src=\"img/"
#define AFTER_IMG "\" alt=\"| Image |\">\n"

#define AFTER_BLOCK "</article>\n<a href=\"#Menu\" class=\"retour\">Retour à l'index</a>\n"

#define BEFORE_MENU "</section>\n<aside id=\"Menu\" class=\"menu\">\n<h3>Index</h3>\n<ol>\n"
#define BEFORE_MENU_NAME "<li><a href=\"#"
#define AFTER_MENU_LINK "\"><code>"
#define AFTER_MENU_NAME "</code></a></li>\n"
#define AFTER_MENU "</ol>\n</aside>\n"

#define FOOT "</body>\n</html>"

//Définition des séparateurs
#define SEPARATOR_1 '@'//Doublé pour la balise de fin de l'exemple
#define SEPARATOR_2 '$'

//Définition des caractères
#define SPACE ' '
#define TAB '\t'
#define NEWLINE '\n'

//Constantes internes
#define NAME 1
#define ARGUMENT 2
#define EXEMPLE 3
#define IMG 4
#define END 5

#define SIZE_NAME 4
#define SIZE_ARGUMENT 8
#define SIZE_EXEMPLE 7
#define SIZE_IMG 3

#define USAGE_ERROR 1
#define EMPTY_NAME_ERROR 2
#define MALLOC_ERROR 3
#define FOPEN_SOURCE_ERROR 4
#define FOPEN_DESTINATION_ERROR 5
#define FOPEN_STYLE_ERROR 6
#define FGETS_ERROR 7
#define FPUTS_ERROR 8
#define FWRITE_ERROR 9