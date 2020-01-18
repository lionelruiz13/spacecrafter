/*
Auteur: Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour association-sirius.org
Mise à jour le: 20/05/2016
*/

Parser (texte -> HTML/CSS)


Convention de fichier source (un fichier CSS peut-être ajouté en paramètre à l'exécution) :

Les espaces ou tabulations au début de la ligne (avant les balises) seront ignorés pour permettre d'indenter, pareil pour les lignes vides
Fermeture implicite d'une balise par un nouveau NAME ou END
Les balises peuvent s'écrire sur plusieurs lignes tant qu'on n'en ouvre pas une nouvelle


Format :

NAME NomDeLaCommande@Descrpition@Particularity	#Sert d'ouverture, obligatoirement une et au début.

ARGUMENT Variable@Type@Descrpition@Particularity [Listedevaleurs]	#Élément d'une liste d'arguments

$ Valeur@Description@Particularity #Élément d'une liste des valeurs possibles dans un argument

EXEMPLE Exemple #C'est une balise spéciale qui doit obligatoirement être fermée explicitement par @@ (et la seule)

IMG url	#Image associéé : chemin relatif, absolu ou url

END #Définit la fin de toutes les commandes (ne pas utiliser @@ à la fin d'une commande mais soit une nouvelle commande avec directement @NAME soit END à la fin de la liste de commandes)


Exemple :

NAME date @ this command manage all time and date modification ....
ARGUMENT jday@string @ date time in julian calendar
ARGUMENT local@string @ date time in local @ if string begin with T, only time will be modified.
ARGUMENT utc@string @ date time in utc time
ARGUMENT sideral@string @ date time in sideral time
ARGUMENT load@string ; current @ load saved date ; preset @ save date and time to be use with current
ARGUMENT sun@string ; set @ compute time to be change saved date ; ride @ compute time to ; meridian @ compute time to ...
EXEMPLE	L'exemple qui montre comment ça marche @@
END


peut s'écrire :

NAME date
	@ this command manage all time and date modification ....
ARGUMENT jday@string
	@ date time in julian calendar
ARGUMENT local@string
	@ date time in local
	@ if string begin with T, only time will be modified.
ARGUMENT utc@string
	@ date time in utc time
ARGUMENT sideral@string
	@ date time in sideral time
ARGUMENT load@string ;
	current @ load saved date ;
	preset @ save date and time to be use with current
ARGUMENT sun@string ;
	set @ compute time to be change saved date ;
	ride @ compute time to ; meridian @ compute time to ...
EXEMPLE	
	L'exemple qui
	montre comment
	ça marche
@@

END
