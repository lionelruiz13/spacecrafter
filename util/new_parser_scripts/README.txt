/*
Auteur: Nicolas Barile <n.barile.57@gmail.com> pour association-sirius.org
Mise à jour le 3/06/2020
*/

Parser (texte -> HTML/CSS)


Les espaces ou tabulations au début de la ligne (avant les balises) seront ignorés, mais nécéssaire pour la lecture, pareil pour les lignes vides
Fermeture implicite d'un bloc par un nouveau NAME ou par un NAME END en fin de fichier


Format d'un bloc :

NAME NomDeLaCommande #Sert d'ouverture, obligatoirement une et au début.
	@Descrpition
	@Particularity	

ARGUMENT Variable @ Type 		#Élément d'une liste d'arguments
	@Descrpition
	@Particularity [Listedevaleurs]	

$ Valeur 				#Élément d'une liste des valeurs possibles dans un argument
@Description
@Particularity 

EXEMPLE Exemple 			#C'est une balise spéciale

@@ 					#Definit la fin d'une commande

NAME END 				#Définit la fin de toutes les commandes (Et donc la fin du fichier)

IMG url					#Image associéé : chemin relatif, absolu ou url


Exemple Bloc:

NAME date
	@ this command manage all time and date modification ....
ARGUMENT jday @ string
	@ date time in julian calendar
ARGUMENT local @ string
	@ date time in local
	@ if string begin with T, only time will be modified.
ARGUMENT utc @ string
	@ date time in utc time
ARGUMENT sideral @ string
	@ date time in sideral time
ARGUMENT load @ string ;
	current @ load saved date ;
	preset @ save date and time to be use with current
ARGUMENT sun @ string ;
	set @ compute time to be change saved date ;
	ride @ compute time to ; meridian @ compute time to ...
EXEMPLE	
	L'exemple qui
	montre comment
	ça marche
@@
								# les deux saut de ligne
								# sont nécéssaire entre chaque blocs
NAME END
