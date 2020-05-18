/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015-2017 of Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef IO_H
#define IO_H

#include <SDL2/SDL_thread.h>
#include <queue> //ServerSocket
#include <cstring>
#include <SDL2/SDL_net.h> //ServerSocket
#include <iostream> //ServerSocket

#include "spacecrafter.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"

#ifdef LINUX
//for pipe
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif


template<class T>
std::string toString(const T& t) //ServerSocket
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}




/*
Serveur de contrôle de l'application
Utilité : ce programme permet de dialoguer avec l'application par le réseau
Usage : à inclure dans le programme C++
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour association-sirius.org
Mise à jour le 17/05/2016
*/

/* Niveau de debug */
#define MAX_DEBUG 100 //Niveau de debug maximum (valable aussi pour le scope)
#define TRACE_FCT 6 //Trace au début et à la fin de chaque fonction
#define DEBUG_FCT 5 //Debug avant et parfois après les appels de fonctions importantes
#define INFO_CONNEXION 4 //Infos comme les infos du serveur, la connexion d'un client
#define WARN_CLIENT 3 //Warnings comme une tentative de connexion d'un client sur un serveur plein ou une configuration douteuse
#define ERROR_SOCKET 2 //Erreur comme échec de l'ouverture de socket serveur
#define ERREUR_FATAL 1 //Erreur fatale comme l'échec du chargement de la librairie SDL_net ou d'allocation
//PAS UTILISEE
#define DEBUG_OFF 0 //Debug désactivé

/* Niveau de scope de debug */
#define FLUX_EVENT 3 //Evènements liés aux flux de données (réception, émission) 
#define CLIENT_EVENT 2 //Evènements en relation avec les client (connexion, déconexion, serveur plein, nombre de clients connectés)
#define SERVEUR_EVENT 1 //Evènements serveur (démarrage, ouverture de son socket, arrêt)

/* Code d'erreur */
#define IO_NO_ERROR 0 //Par d'erreur

/* Codes d'erreur dans l'initialisation */
#define SDL_NET_INIT_ERROR_CODE 1 //Erreur lors de l'initialisation de SDL
#define SDL_RESOLEVHOST_ERROR_CODE 2 //Erreur lors de la préparation de la structure pour le serveur
#define SDL_ALLOCSOCKETSET_ERROR_CODE 3 //Erreur lors de l'allocation du SocketSet
#define SDL_CREATEMUTEX_ERROR_CODE 4 //Erreur lors de la création du mutex de verouillage d'activité du thread
#define NEW_TCPSOCKET_TAB_ERROR_CODE 5 //Erreur lors de l'allocation du tableau de sockets clients
#define NEW_BOOL_TAB_ERROR_CODE 6 //Erreur lors de l'allocation du tableau de sockets clients
#define NEW_BUFFER_ERROR_CODE 7 //Erreur lors de l'allocation du buffer

/* Codes d'erreur dans l'ouverture */
#define SERVER_SOCKET_OPEN_ERROR_CODE 101 //Erreur lors de l'ouverture du socket serveur
#define SDL_ADDSOCKET_SERVER_ERROR_CODE 102 //Erreur lors de l'ajout du socket serveur au SocketSet
#define SDL_CREATETHREAD_ERROR_CODE 103

/* Codes d'erreur dans le traitement */
#define SDL_LOCKMUTEX_ERROR_CODE 201
#define SDL_UNLOCKMUTEX_ERROR_CODE 202
#define SDL_CHECKSOCKETS_ERROR_CODE 203 //Erreur lors de la vérification du SocketSet
#define SDL_SEND_ERROR_CODE 204
#define SDL_DELSOCKET_CLIENT_ERROR_CODE 205

/* Codes d'erreur dans la fermeture */
#define SERVER_NOT_OPEN_CODE 301

#define CLIENT_SEPARATOR1 "|"
#define CLIENT_SEPARATOR2 "/"
#define DEBUG_SEPARATOR3 " | " //Troisième déparateur dans le debug


class ServerSocket {
private:
	/* Variables configurables */
	unsigned int port; //Port d'écoute du serveur
	unsigned int maxClients; //Nombre maxium de clients (comme le serveur prends une place dans le set de sockets, ce nombre est égal à la taille du set -1)
	unsigned int bufferSize; //Taille du buffer de réception
	int logLevel; //Niveau de debug
	int logScope; //Scope du debug
	LOG_TYPE logType; //Type de log (spécifique à l'application)

	/* Variables d'état */
	bool serverOpen; //Etat du serveur
	unsigned int clientCount; //Nombre de clients actuellement connectés au serveur
	unsigned int broadcastId; //Id du message de broadcast

	/* Variables de statistiques */
	unsigned int maxSimultaneousClient; //Nombre maximum de clients connectés simulatannément

	unsigned int connection; //Nombre total de connexions
	unsigned int refusedConnectionServerFull; //Nombre total de connexions refusées pour cause de serveur plein
	unsigned int cannotAcceptClient; //Nombre total d'erreurs lors de l'acceptation du client

	unsigned int requestRecieved; //Nombre total de requêtes
	unsigned int dataRecieved; //Total de données reçues
	unsigned int possibleBufferOverflow; //Nombre total de buffer overflow

	unsigned int requestSend; //Nombre total de requêtes envoyées
	unsigned int dataSend; //Total de données envoyées
	unsigned int requestSendFailed; //Nombre total d'erreurs lors de l'envoi de la requête

	unsigned int statsPeriod;
	Uint32 timeout;

	/* Variables du serveur */
	IPaddress serverIP; //IP du serveur (0.0.0.0 pour écouter sur toutes les IPs du serveur)
	TCPsocket serverSocket; //Socket d'écoute du serveurs
	SDLNet_SocketSet socketSet; //Tableau de surveillance des sockets
	TCPsocket* clientSocketTab; //Tableau de sockets client
	bool* clientBroadcastTab; //Tableau de demandes de feedback

	/* Variables du thread */
	SDL_Thread *thread; //Thread du serveur qui attends les paquets
	int threadReturnValue; //Valeur de retour du thread
	bool stopThread;
	int lock(SDL_mutex *mutex);
	int unlock(SDL_mutex *mutex);
	SDL_mutex *running; //Mutex de serveur actif
	int activeSocketsCount; //Nombre de sockets actifs
	char* buffer; //Buffer de réception

	/* Variables de stockage des données */
	std::queue<std::string> inputQueue; //File d'entrée
	std::queue<std::string> outputQueue; //File de sortie
	SDL_mutex *inputting; //Mutex de la file d'entrée
	SDL_mutex *outputting; //Mutex de la file de sortie

	/* Fonction et code d'initialisation */
	int init(unsigned int port, unsigned int maxClients, unsigned int bufferSize, int logLevel, int logScope); //Fonction d'initialisation appellée par les constructeurs
	int initErrorCode; //Code d'erreur de l'initialisation

	/* Fonctions de traitement */
	static int threadWrapper(void *Data); //Fonction qui délègue run
	int run(); //Boucle de traitement des données entrantes
	void checkNewClient(); //Fonction de vérification de nouveaux clients
	void checkNewData(); //Fonction de vérification de nouvelles données
	void computeNewData(unsigned int client); //Fonction de traitement des données
	bool computeString(unsigned int client, std::string string); //Fonction de traitement de la chaîne
	bool computeHttp(unsigned int client, std::string string);//Fonction de traitement d'une requête HTTP (BETA)
	void computeNormalString(unsigned int client, std::string string);//Fonction de traitement d'une requête normale
	void checkDataToSend(); //Fonction d'envoi de données reçues de l'application
	int broadcast(std::string data); //Fonction de broadcast aux clients
	int close(unsigned int client); //Fonction de fermeture du socket client

	/* Fonctions de factorisation ou d'assistance */
	int send(TCPsocket client); //Fonction qui envoi la chaine contenue dans le buffer
	int resetThread(); //Fonction qui redémarre le thread de traiement quand il est inactif
	int killThread(); //Fonction qui kill le thread de traitement quand il est inactif

	std::string humanReadable(unsigned int); //Fonction qui renvoi une chaine formatée (B, KB, MB, GB)
	std::string clientIp(unsigned int client); //Fonction qui renvoi l'adresse IP du client sous forme de chaîne

	/* Fonctions de debug */
	bool debug(int level, int scope);//Debug
	void debugOut(std::string msg);//Debug


	std::string replace(std::string base, const std::string from, const std::string to) {
		std::string SecureCopy = base;
		for (size_t start_pos = SecureCopy.find(from); start_pos != std::string::npos; start_pos = SecureCopy.find(from,start_pos)) SecureCopy.replace(start_pos, from.length(), to);
		return SecureCopy;
	}


public:
	/* Constructeurs et destructeur */
	ServerSocket(unsigned int port, int logLevel); //Contructeur simple
	ServerSocket(unsigned int port, unsigned int maxClients, unsigned int bufferSize, int logLevel, int logScope); //Constructeur avancé
	~ServerSocket(); //Destructeur

	/* Fonctions d'action sur le serveur */
	int open(); //Fonction d'ouverture du socket serveur
	int close(); //Fonction de fermeture du socket serveur

	/* Fonction d'affichage des statistiques non-nulles */
	void stats();

	std::string getInput();

	unsigned int getstatsPeriod() {
		return statsPeriod;
	}

	/* Setters */
	void setLogLevel(int logLevel) {
		this->logLevel = logLevel;
	}
	void setLogScope(int logScope) {
		this->logScope = logScope;
	}
	void setOutput(std::string data);

	void setstatsPeriod(unsigned int statsPeriod);
};



#endif // IO_H
