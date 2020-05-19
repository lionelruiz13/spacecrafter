/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of the Association Sirius
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

#include <fstream>
#include <iostream> //ServerSocket
#include <sstream>
#include <string> //ServerSocket

#include "spacecrafter.hpp"
#include "tools/io.hpp" //ServerSocket
#include "tools/utility.hpp"
#include "tools/log.hpp" //ServerSocket


#ifdef WIN32
#include <direct.h>
#endif

/*
Serveur de contrôle de l'application
Utilité : ce programme permet de dialoguer avec l'application par le réseau
Usage : à inclure dans le programme C++
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour association-sirius.org
Mise à jour le 17/07/2017
*/

///* Configuration du niveau de débug dans la boucle de traitement */
// #define IO_DEBUG_TRACE_IN_LOOP "Niveau de debug TRACE activé dans la boucle"//Commenter pour annuler
// #define IO_DEBUG_DEBUG_IN_LOOP "Niveau de debug DEBUG totalement activé dans la boucle"//Commenter pour annuler
#define IO_DEBUG_INFO_IN_LOOP "Niveau de debug INFO totalement activé dans la boucle"//Commenter pour annuler
#define IO_DEBUG_WARN_IN_LOOP "Niveau de debug WARN totalement activé dans la boucle"//Commenter pour annuler
#define DEBUG_PERIODIC_STATS_ENABLED "Statistiques périodiques activées dans la boucle"//Commenter pour annuler

/* Valeurs par défaut */
#define DEFAULT_PORT 1234 //Port par défaut
#define MAX_CLIENTS 16 //Limite de clients simulatannés par defaut
#define BUFFER_SIZE 65536 //Taille du buffer par defaut (en octets)
#define STATS_PERIOD 5000 //Durée minimum entre deux récapitulatifs dans la boucle (en millisecondes)
#define MAX_BUFFER 1024 //la taille d'un buffer à envoyer comme réponse via le TCP

/* Valeurs d'avertissement */
#define LOT_OF_CLIENTS 32 //Limite de clients simulatannés considérée comme grande et non testée
#define SMALL_BUFFER_SIZE 512 //Taille de buffer considérée comme dangeureseument petite (doit être plus grande que les messages succeptibles d'être envoyés par le serveur)
#define BIG_BUFFER_SIZE 1048576 //Taille de buffer considérée comme inutilement grande


ServerSocket::ServerSocket(unsigned int port, int logLevel)
{
	initErrorCode = init(port, MAX_CLIENTS, BUFFER_SIZE, logLevel, MAX_DEBUG);
	if(initErrorCode != IO_NO_ERROR) throw initErrorCode;
}

ServerSocket::ServerSocket(unsigned int port, unsigned int maxClients, unsigned int bufferSize, int logLevel, int logScope)
{
	initErrorCode = init(port, maxClients, bufferSize, logLevel, logScope);
	if(initErrorCode != IO_NO_ERROR) throw initErrorCode;
}

int ServerSocket::init(unsigned int port, unsigned int maxClients, unsigned int bufferSize, int logLevel, int logScope)
{
	/* Constructeur */
	this->port = port;
	this->maxClients = maxClients;
	this->bufferSize = bufferSize;
	this->logLevel = logLevel;
	this->logScope = logScope;

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("IO_DEBUG_TRACE_IN_LOOP"); //Debug
	#endif

	#ifdef IO_DEBUG_DEBUG_IN_LOOP
	if(debug(DEBUG_FCT, SERVEUR_EVENT))
		debugOut("IO_DEBUG_DEBUG_IN_LOOP"); //Debug
	#endif

	#ifdef IO_DEBUG_INFO_IN_LOOP
	if(debug(INFO_CONNEXION, SERVEUR_EVENT))
		debugOut("IO_DEBUG_INFO_IN_LOOP"); //Debug
	#endif

	#ifdef DEBUG_PERIODIC_STATS_ENABLED
	if(debug(INFO_CONNEXION, SERVEUR_EVENT))
		debugOut("DEBUG_PERIODIC_STATS_ENABLED"); //Debug
	#endif

	#ifdef IO_DEBUG_WARN_IN_LOOP
	if(debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("IO_DEBUG_WARN_IN_LOOP"); //Debug
	#endif

	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("INIT_START"); //Debug


	if(debug(INFO_CONNEXION, SERVEUR_EVENT))
		debugOut("Port " + toString(port) + DEBUG_SEPARATOR3 + "Slots " + toString(maxClients) + DEBUG_SEPARATOR3 + "Buffer " + toString(bufferSize) + " B" + DEBUG_SEPARATOR3 + "Log " + toString(logLevel) + "(" + toString(logScope) + ")"); //Debug


	/* Avertissements de configuration */
	if(port == 0 && debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("NULL_PORT"); //Debug
	else if(port < 1024 && debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("ADMIN_PORT "+ toString(port)); //Debug

	if(maxClients == 0 && debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("NULL_MAXCLIENTS"); //Debug
	else if(maxClients > LOT_OF_CLIENTS && debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("LOT_OF_MAXCLIENTS "+ toString(maxClients)); //Debug

	if(bufferSize == 0 && debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("NULL_BUFFER"); //Debug
	else if(bufferSize < SMALL_BUFFER_SIZE && debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("SMALL_BUFFER "+ toString(bufferSize)); //Debug
	else if(bufferSize > BIG_BUFFER_SIZE && debug(WARN_CLIENT, SERVEUR_EVENT))
		debugOut("BIG_BUFFER "+ toString(bufferSize)); //Debug

	/*Initialisations */
	initErrorCode = -1;
	serverOpen = false;
	stopThread = false;
	activeSocketsCount = 0;
	clientCount = 0;
	broadcastId = 0;

	/* Initialisation des variables de statistiques */
	maxSimultaneousClient = 0;
	connection = 0;
	refusedConnectionServerFull = 0;
	cannotAcceptClient = 0;
	requestRecieved = 0;
	dataRecieved = 0;
	possibleBufferOverflow = 0;
	requestSend = 0;
	dataSend = 0;
	requestSendFailed = 0;
	statsPeriod = STATS_PERIOD;
	timeout = SDL_GetTicks();

	/* Initialisation de SDL_net */
	if (SDLNet_Init() < 0) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_NET_INIT_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		return SDL_NET_INIT_ERROR_CODE;
	}

	/* Préparation de la connexion du serveur */
	if(SDLNet_ResolveHost(&serverIP, NULL, this->port) < 0) {
		if(debug(ERROR_SOCKET, SERVEUR_EVENT))
			debugOut("SDL_RESOLEVHOST_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		return SDL_RESOLEVHOST_ERROR_CODE;
	}

	/* Création d'un SocketSet pour surveiller les sockets */
	socketSet = SDLNet_AllocSocketSet(maxClients + 1);
	if (socketSet == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_ALLOCSOCKETSET_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		return SDL_ALLOCSOCKETSET_ERROR_CODE;
	}

	/* Création du mutex pour protéger l'exécution du thread */
	running = SDL_CreateMutex();
	if (running == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_CREATEMUTEX_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		return SDL_CREATEMUTEX_ERROR_CODE;
	}

	/* Initialisation du tableau de sockets clients */
	clientSocketTab = new TCPsocket[maxClients]; //Allocation du tableau de sockets clients
	if(clientSocketTab == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("NEW_TCPSOCKET_TAB_ERROR"); //Debug
		return NEW_TCPSOCKET_TAB_ERROR_CODE;
	}
	clientBroadcastTab = new bool[maxClients];
	if(clientBroadcastTab == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("NEW_BOOL_TAB_ERROR"); //Debug
		return NEW_BOOL_TAB_ERROR_CODE;
	}
	for (unsigned int i = 0; i < maxClients; i++)	{
		clientSocketTab[i] = NULL; //Initialisation de tous les sockets clients à NULL
		clientBroadcastTab[i] = false;
	}

	/* Initialisation du buffer */
	buffer = new char[bufferSize];
	if(buffer == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("NEW_BUFFER_ERROR"); //Debug
		return NEW_BUFFER_ERROR_CODE;
	}

	/* Création du mutex pour protéger la file d'entrée des clients */
	outputting = SDL_CreateMutex();
	if (outputting == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_CREATEMUTEX_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		return SDL_CREATEMUTEX_ERROR_CODE;
	}

	/* Création du mutex pour protéger la file de sortie des clients */
	inputting = SDL_CreateMutex();
	if (inputting == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_CREATEMUTEX_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		return SDL_CREATEMUTEX_ERROR_CODE;
	}

	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("INIT_END"); //Debug

	return IO_NO_ERROR; //Pas d'erreur
}

int ServerSocket::open()
{
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("START_OPEN"); //Debug
	if(debug(INFO_CONNEXION, SERVEUR_EVENT))
		debugOut("SERVER_START "+ toString(port)); //Debug

	/* Ouverture du socket serveur */
	serverSocket = SDLNet_TCP_Open(&serverIP);
	if (!serverSocket) {
		if(debug(ERROR_SOCKET, SERVEUR_EVENT))
			debugOut("SDL_OPEN_SERVER_SOCKET_ERROR "+ (std::string)SDLNet_GetError() + " (port "  + toString(this->port) + ")"); //Debug
		return SERVER_SOCKET_OPEN_ERROR_CODE;
	}

	/* Changement du flag d'état du serveur */
	serverOpen = true;

	/* Ajout du socket serveur au socketSet pour le surveiller */
	if(SDLNet_TCP_AddSocket(socketSet, serverSocket) <= 0) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_ADDSOCKET_SERVER_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		SDLNet_TCP_Close(serverSocket); //Fermeture du socket serveur
		return SDL_ADDSOCKET_SERVER_ERROR_CODE;
	}

	/* Lancement du thread de traitement */
	thread = SDL_CreateThread(threadWrapper, "Loop thread", this);
	if (thread == NULL) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_CREATETHREAD_ERROR "+ (std::string)SDLNet_GetError()); //Debug
		return SDL_CREATETHREAD_ERROR_CODE;
	}

	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("END_OPEN"); //Debug

	return IO_NO_ERROR; //Pas d'erreur
}

int ServerSocket::close()
{
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("START_CLOSE"); //Debug

	if(!serverOpen) {
		if(debug(WARN_CLIENT, SERVEUR_EVENT))
			debugOut("SERVER_NOT_OPEN"); //Debug
		return SERVER_NOT_OPEN_CODE;
	}

	if(debug(INFO_CONNEXION, SERVEUR_EVENT))
		debugOut("SERVER_STOP"); //Debug

	killThread();

	//Fermerture de tous les clients ouverts
	strcpy(buffer, "GOODBYE"); //Préparation du message
	for (unsigned int client = 0; client < maxClients; client++) { //Parcourt tous les clients
		if(clientCount <= 0) break; //Si on a déjà fermé tous les sockets clients on s'arrête
		if (clientSocketTab[client] != NULL) { //S'il le socket est utilisé
			send(clientSocketTab[client]); //Envoi du message au client
			close(client); //Opérations de fermeture du socket du client
		}
	}

	SDLNet_TCP_Close(serverSocket);	//Fermeture du socket serveur

	serverOpen = false; //Changement du flag d'état du serveur

	return IO_NO_ERROR;

	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("END_CLOSE"); //Debug
}

ServerSocket::~ServerSocket()
{
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("START_DELETE"); //Debug

	if(serverOpen) close(); //Fermeture du serveur

	SDLNet_FreeSocketSet(socketSet);//Libération du SocketSet
	delete[] clientSocketTab; //Libération du tableau de sockets
	delete[] buffer; //Libération du buffer
	SDL_DestroyMutex(running); //Libération du mutex
	SDLNet_Quit(); //Fermeture de SDL_net

	stats(); //Affichage des statistiques

	while(!inputQueue.empty()) inputQueue.pop(); //Vidage de la file d'entrée
	while(!outputQueue.empty()) outputQueue.pop(); //Vidage de la file de sortie

	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("END_DELETE"); //Debug
}

void ServerSocket::stats()
{
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("START_STATS"); //Debug

	if(connection) {
		if(debug(DEBUG_FCT, SERVEUR_EVENT)) {
			debugOut("CONNECTION "+ toString(connection) + " (" + toString(maxSimultaneousClient) + " max)");
			debugOut("REQUEST_RECIEVED "+ toString(requestRecieved) + " requests (" + toString(dataRecieved) + " bytes)");
			debugOut("REQUEST_SEND "+ toString(requestSend) + " requests (" + toString(dataSend) + " bytes)");
		}
		if(debug(INFO_CONNEXION, SERVEUR_EVENT)) {
			debugOut("CONNECTION "+ humanReadable(connection) + " (" + humanReadable(maxSimultaneousClient) + " max)");
			debugOut("REQUEST_RECIEVED "+ humanReadable(requestRecieved) + " (" + humanReadable(dataRecieved) + "B)");
			debugOut("REQUEST_SEND "+ humanReadable(requestSend) + " (" + humanReadable(dataSend) + "B)");
		}
	}

	if(debug(DEBUG_FCT, CLIENT_EVENT))
		debugOut("DEBUG_FULL_COUNT "+ toString(refusedConnectionServerFull));
	if(refusedConnectionServerFull && debug(WARN_CLIENT, CLIENT_EVENT))
		debugOut("DEBUG_FULL_COUNT "+ humanReadable(refusedConnectionServerFull));

	if(debug(DEBUG_FCT, CLIENT_EVENT))
		debugOut("DEBUG_CANNOTACCEPT_COUNT "+ toString(cannotAcceptClient));
	if(cannotAcceptClient && debug(WARN_CLIENT, CLIENT_EVENT))
		debugOut("DEBUG_CANNOTACCEPT_COUNT "+ humanReadable(cannotAcceptClient));

	if(debug(DEBUG_FCT, FLUX_EVENT))
		debugOut("DEBUG_OVERFLOW_COUNT "+ toString(possibleBufferOverflow));
	if(possibleBufferOverflow && debug(WARN_CLIENT, FLUX_EVENT))
		debugOut("DEBUG_OVERFLOW_COUNT "+ humanReadable(possibleBufferOverflow));

	if(debug(DEBUG_FCT, FLUX_EVENT))
		debugOut("DEBUG_SENDFAILED_COUNT "+ toString(requestSendFailed));
	if(requestSendFailed && debug(WARN_CLIENT, FLUX_EVENT))
		debugOut("DEBUG_SENDFAILED_COUNT "+ humanReadable(requestSendFailed));

	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("END_STATS"); //Debug
}

std::string ServerSocket::humanReadable(unsigned int data)
{
	if(data < 1000) return toString(data);
	else if(data < 1000000) return toString(data / 1000) + "K";
	else if(data < 1000000000) return toString(data / 1000000) + "M";
	else return toString(data / 1000000000) + "G";
}

std::string ServerSocket::clientIp(unsigned int client)
{
	Uint32 ip = SDLNet_TCP_GetPeerAddress(clientSocketTab[client])->host;
	return (toString(ip & 0x000000ff) + "." + toString((ip & 0x0000ff00) >> 8) + "." + toString((ip & 0x00ff0000) >> 16) + "." + toString((ip & 0xff000000) >> 24));
}

std::string ServerSocket::getInput()
{
	if(lock(inputting) == IO_NO_ERROR) {
		std::string data;
		if(inputQueue.empty()) data = "";
		else {
			data = inputQueue.front();
			inputQueue.pop();
		}
		unlock(inputting);
		return data;
	} else return "";
}


void ServerSocket::setstatsPeriod(unsigned int statsPeriod)
{
	this->statsPeriod = statsPeriod;
	timeout = SDL_GetTicks() + this->statsPeriod;
}


void ServerSocket::setOutput(std::string data)
{
	if(lock(outputting) == IO_NO_ERROR) {
		unsigned sz = data.size();
		if (sz>MAX_BUFFER) {
			cLog::get()->write("ServerSocket data setOutput too big", LOG_TYPE::L_WARNING);
			data.resize(MAX_BUFFER);
		}
		outputQueue.push(data);
		unlock(outputting);
	}
}

// bool ServerSocket::debug(int level, int scope)
// {
// 	if(logLevel >= level && logScope >= scope) {
// 		switch(level) {
// 			case ERREUR_FATAL :
// 				logType = LOG_TYPE::L_ERROR;
// 				break;
// 			case ERROR_SOCKET :
// 				logType = LOG_TYPE::L_ERROR;
// 				break;
// 			case WARN_CLIENT :
// 				logType = LOG_TYPE::L_WARNING;
// 				break;
// 			case INFO_CONNEXION :
// 				logType = LOG_TYPE::L_INFO;
// 				break;
// 			case DEBUG_FCT :
// 				logType = LOG_TYPE::L_DEBUG;
// 				break;
// 			case TRACE_FCT :
// 				logType = LOG_TYPE::L_DEBUG;
// 				break;
// 			default :
// 				logType = LOG_TYPE::L_INFO;
// 		}
// 		return true;
// 	}
// 	return false;
// }

void ServerSocket::debugOut(std::string msg)
{
	cLog::get()->write("TCP : " + msg, logType, LOG_FILE::TCP);
}

/* thread */

int ServerSocket::threadWrapper(void *Data)
{
	return ((ServerSocket *)Data)->run();
}

int ServerSocket::run()
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT)) debugOut("RUN_START"); //Debug
	#endif

	while(true) {

		// unlock(running); //Thread inactif

		if(stopThread) {
			stopThread = false;
			return IO_NO_ERROR;
		}

		activeSocketsCount = SDLNet_CheckSockets(socketSet, 1); //Attente d'activité sur le SocketSet (timeout de 1ms)
		if(lock(running) == IO_NO_ERROR) { //Thread actif
			if(activeSocketsCount > 0) { //S'il y a des sockets actifs
				#ifdef IO_DEBUG_DEBUG_IN_LOOP
				if(debug(DEBUG_FCT, SERVEUR_EVENT))
					debugOut("SDL_CHECKSOCKETS_ACTIVITY " + toString(activeSocketsCount)); //Debug
				#endif
				checkNewClient(); //Regarde s'il y a des nouveaux clients et les traite
				if(activeSocketsCount) checkNewData(); //Regarde s'il y a des nouvelles données et les traites
			} else if(activeSocketsCount < 0) { //S'il y a eu une erreur lors de la récupération du nombre de sockets actifs
				if(debug(ERREUR_FATAL, SERVEUR_EVENT))
					debugOut("SDL_CHECKSOCKETS_ERROR " + (std::string)SDLNet_GetError()); //Debug
				close(); //Ferme de le serveur
				return SDL_CHECKSOCKETS_ERROR_CODE;
			}

			checkDataToSend(); //Vérifie s'il y a des données à envoyer aux clients

			#ifdef DEBUG_PERIODIC_STATS_ENABLED
			if(SDL_GetTicks() >= timeout) {
				timeout = SDL_GetTicks() + statsPeriod;
				stats();
			}
			#endif
		unlock(running); //Thread inactif
		}
	}

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT)) debugOut("RUN_END"); //Debug
	#endif

	return IO_NO_ERROR;
}

void ServerSocket::checkNewClient()
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, CLIENT_EVENT)) debugOut("CHECKNEWCLIENT_START"); //Debug
	#endif

	if (SDLNet_SocketReady(serverSocket) != 0) { //S'il y a des nouveaux clients
		if (clientCount < maxClients) { //S'il y a de la place pour le client
			unsigned int freeSpot = maxClients - 1;
			for (unsigned int socket = 0; socket < maxClients; socket++) {
				if (clientSocketTab[socket] == NULL) {
					freeSpot = socket; //Garde le numéro du socket à utiliser
					break;
				}
			}

			clientSocketTab[freeSpot] = SDLNet_TCP_Accept(serverSocket); //Accept le client et le met dans le tableau de sockets

			#ifdef IO_DEBUG_INFO_IN_LOOP
			if(debug(INFO_CONNEXION, CLIENT_EVENT))
				debugOut("New client " + clientIp(freeSpot)); //Debug
			#endif

			if(clientSocketTab[freeSpot] == NULL) { //Erreu lors de l'acceptation du client
				cannotAcceptClient++; //Incrément le nombre total d'erreurs lors de l'acceptation du client
				if(debug(WARN_CLIENT, CLIENT_EVENT))
					debugOut("SDL_ACCEPT_CLIENT_ERROR"); //Debug
			} else {
				if(SDLNet_TCP_AddSocket(socketSet, clientSocketTab[freeSpot]) <= 0) { //Ajoute le socket client au SocketSet
					if(debug(ERROR_SOCKET, SERVEUR_EVENT))
						debugOut("SDL_ADDSOCKET_CLIENT_ERROR "+ (std::string)SDLNet_GetError()); //Debug
					close(freeSpot); //Ferme le socket temporaire
				} else {
					connection++; //Incrémente le nombre total de connexions
					clientCount++; //Incrémente le nombre de clients connectés
					if(clientCount > maxSimultaneousClient)
						maxSimultaneousClient = clientCount; //Mise à jour du nombre maximum de clients connectés simulatannément

					//strcpy(buffer, SERVER_MSG_HELLO_FINAL); //Prépare le message
					//send(clientSocketTab[freeSpot]); //Envoi du message au client //Supprimé pour rester plus sobre et mieux supporter les clients http ou autre

					#ifdef IO_DEBUG_INFO_IN_LOOP
					if(debug(INFO_CONNEXION, CLIENT_EVENT))
						debugOut("CLIENT_COUNT " + toString(clientCount) + "/" + toString(maxClients)); //Debug
					#endif
				}

			}
		} else { //S'il n'y a pas de place pour le client

			refusedConnectionServerFull++; //Incrémente le nombre de connexion refusées pour cause de serveur plein
			#ifdef IO_DEBUG_WARN_IN_LOOP
			if(debug(WARN_CLIENT, CLIENT_EVENT))
				debugOut("server full"); //Debug
			#endif

			TCPsocket tempSock = SDLNet_TCP_Accept(serverSocket); //Accepte le client dans un socket

			if(tempSock == NULL) { //Erreur lors de l'acceptation du client
				cannotAcceptClient++;
				#ifdef IO_DEBUG_WARN_IN_LOOP
				if(debug(WARN_CLIENT, CLIENT_EVENT))
					debugOut("SDL_ACCEPT_CLIENT_ERROR"); //Debug
				#endif
			}

			strcpy(buffer, "SERVER_FULL"); //Prépare le message
			send(tempSock); //Envoi du message au client
			SDLNet_TCP_Close(tempSock); //Ferme le socket

		}
		activeSocketsCount--; //Décrémente le nombre de sockets actifs
	}

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, CLIENT_EVENT))
		debugOut("CHECKNEWCLIENT_END"); //Debug
	#endif
}

void ServerSocket::checkNewData()
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, CLIENT_EVENT)) debugOut("CHECKNEWDATA_START"); //Debug
	#endif

	int clientSocketActivity;
	for (unsigned int client = 0; client < maxClients; client++) { //Parcours de tous les clients connectés

		if(clientSocketTab[client] != NULL) { //Si le socket est ouvert

			clientSocketActivity = SDLNet_SocketReady(clientSocketTab[client]); //Récupération de l'activité du socket
			if (clientSocketActivity) { //S'il y a de l'activité sur le socket

				int receivedByteCount = SDLNet_TCP_Recv(clientSocketTab[client], buffer, bufferSize); //Réception des données du client
				if (receivedByteCount <= 0) { //Déconnexion du client
					#ifdef IO_DEBUG_INFO_IN_LOOP
					if (receivedByteCount < 0 && debug(INFO_CONNEXION, CLIENT_EVENT))
						debugOut("RESETED_BY_PEER "+ clientIp(client)); //Debug
					#endif
					close(client); //Opérations de fermeture du socket du client
				} else if ((unsigned int)receivedByteCount >= bufferSize) { //Buffer overflow
					possibleBufferOverflow++; //Incrémente le nombre total de buffer overflow
					strcpy(buffer, "SERVER_OVERFLOW"); //Prépare le message
					send(clientSocketTab[client]); //Envoi du message
					#ifdef IO_DEBUG_WARN_IN_LOOP
					if(debug(WARN_CLIENT, FLUX_EVENT))
						debugOut("BUFFER_OVERFLOW too many data "+ clientIp(client)); //Debug
					#endif
					close(client); //Fermeture du socket client
				} else {

					requestRecieved++; //Incrémente le nombre total de requêtes reçues
					dataRecieved += receivedByteCount; //Incrémente le nombre total de données réçues

					#ifdef IO_DEBUG_INFO_IN_LOOP
					if(debug(INFO_CONNEXION, FLUX_EVENT))
						debugOut("CLIENT_DATA_QUANTITY " + clientIp(client) + " " + toString(receivedByteCount)); //Debug
					#endif

					for(int i=0; i < receivedByteCount; i++) if(buffer[i] == '\r' || buffer[i] == '\0') buffer[i] = '\n';// Remplace \r et \0 par \n
					buffer[receivedByteCount] = '\0'; //Termine la chaîne (on sait jamais)
					computeNewData(client); //Traite les nouvelles données
				}
				activeSocketsCount--; //Décrément le nombre de sockets actifs
			}
		}
		if(activeSocketsCount <= 0) break; //S'arrête si on a traité le nombre de sockets actifs
	}

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT)) debugOut("CHECKNEWDATA_END"); //Debug
	#endif
}

void ServerSocket::computeNewData(unsigned int client)
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, FLUX_EVENT)) debugOut("COMPUTENEWDATA_START"); //Debug
	#endif

	std::string data = buffer; //Transforme les données du buffer en string

	#ifdef IO_DEBUG_DEBUG_IN_LOOP
	if(debug(DEBUG_FCT, FLUX_EVENT)) debugOut("DATA_AS_STRING "+ data); //Debug
	#endif

	unsigned int begin = 0; //Début de la chaine
	unsigned int end; //Fin de la chaine
	bool continues;
	do {
		end = data.find('\n', begin); //Cherche le saut de ligne
		if(begin < data.size() && (end - begin >= 1))
			continues = computeString(client, data.substr(begin, end - begin)); //Avant \n
		begin = end + 1; //Après \n
	} while(end != (unsigned int)std::string::npos && continues); //Tant qu'il y a des \n dans la chaine

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, FLUX_EVENT)) debugOut("COMPUTENEWDATA_END"); //Debug
	#endif
}

bool ServerSocket::computeString(unsigned int client, std::string string)
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, FLUX_EVENT))
		debugOut("COMPUTENEWDATA_START"); //Debug
	#endif

	if(computeHttp(client, string)) {

		#ifdef IO_DEBUG_TRACE_IN_LOOP
		if(debug(TRACE_FCT, FLUX_EVENT))
			debugOut("COMPUTENEWDATA_END"); //Debug
		#endif

		return false; //Client HTTP
	} else {

		computeNormalString(client, string);

		#ifdef IO_DEBUG_TRACE_IN_LOOP
		if(debug(TRACE_FCT, FLUX_EVENT))
			debugOut("COMPUTENEWDATA_END"); //Debug
		#endif

		return true;
	}
}

bool ServerSocket::computeHttp(unsigned int client, std::string string)
{
	//TODO proprer
	#ifdef LINUX
	if(string.substr(0,3) == "GET") { //Client HTTP

		/* Traitement de l'url */
		std::string path = AppSettings::Instance()->getWebDir();

		printf("%s\n", string.c_str()); //Debug

		std::string url = string.substr(4, string.substr(4, std::string::npos).find(' ')); //Récuèpre l'url uniquement
		printf("URL : \"%s\"\n", url.c_str()); //Debug

		std::string urlFile = url.substr(0, url.find('?')); //Récupère l'url sans les paramètres
		printf("FILE : \"%s\"\n", urlFile.c_str()); //Debug

		if(urlFile[urlFile.size()-1] == '/') urlFile.append("index.html"); //Modifie l'url en index.html

		std::string filename = path + urlFile; //Chemin vers le fichier en local
		printf("\"FILE PATH : %s\"\n", filename.c_str()); //debug

		if(url.find('?') != std::string::npos) { //Paramètres dans l'url
			std::string urlParam = url.substr(url.find('?')+1, std::string::npos); //Récupère les paramètres uniquement
			printf("PARAM : \"%s\"\n", urlParam.c_str()); //Debug

			if(urlParam.find("command=", 0) != std::string::npos) { //Paramètre command présent
				std::string command = urlParam.substr(urlParam.find("command=", 0), std::string::npos); //Récupère à partir du paramètre
				command = command.substr(8, command.find('&')-8); //Récupère la commande uniquement
				command = replace(command, "%20", " "); //Décode les espaces
				command = replace(command, "+", " "); //Décodes les espaces
				command = replace(command, "%3A", ":"); //Décodes les ":"
				printf("COMMAND : \"%s\"\n", command.c_str());
				inputQueue.push(command); //Rajoute la chaine à la file de sortie
				broadcast(clientIp(client) + CLIENT_SEPARATOR2 + "HTTP" + CLIENT_SEPARATOR1 + command + '\n'); //Envoi la chaîne à tous les clients
			}
		}

		/* Envoi du fichier */

		struct stat filestat;
		if(!stat(filename.c_str(), &filestat)) { //Récupère les statistiques du fichier
			FILE* file = fopen(filename.c_str(), "r"); //Ouvre le fichier en lecture
			if(file != NULL) { //Fichier ouvert
				std::string extension = urlFile.substr(urlFile.find_last_of(".", std::string::npos)+1, std::string::npos);
				std::string type;
				if(extension == "html" || extension == "HTML") type = "text/html;charset=UTF-8";
				else if(extension == "css" || extension == "CSS") type = "text/css;charset=UTF-8";
				else if(extension == "js" || extension == "JS") type = "application/x-javascript;charset=UTF-8";
				else if(extension == "jpeg" || extension == "JPEG" || extension == "jpg" || extension == "JPEG") type = "image/jpeg";
				else if(extension == "png" || extension == "PNG") type = "image/png";
				else if(extension == "gif" || extension == "GIF") type == "image/gif";
				else type = "text/plain";

				strcpy(buffer, ("HTTP/1.0 200 OK\r\nServer: SpaceCrafter (HTTP/BETA)\r\nContent-Length: " + toString(filestat.st_size) + "\nContent-Type: " + type + "\r\n\r\n").c_str());
				SDLNet_TCP_Send(clientSocketTab[client], (void *)buffer, strlen(buffer)); //Envoi des entêtes
				unsigned int size;
				do {
					size = fread(buffer, 1, bufferSize, file); //Lecture du fichier dans le buffer
					SDLNet_TCP_Send(clientSocketTab[client], (void *)buffer, size); //Envoi du buffer
				} while(size == bufferSize); //Pas à la fin du fichier
				fclose(file); //Fermeture du fichier
			}
		} else { //Problème à l'ouverture du fichier (inexistant...))
			strcpy(buffer, "HTTP/1.0 500 Internal Error\r\nServer: SpaceCrafter (HTTP/BETA)\r\nContent-Length: 0\r\n\r\n");
			send(clientSocketTab[client]); //Envoi des entêtes
		}

		close(client); //Fermeture de la connection
		return true;
	} else 
	if (string.substr(0,4) == "POST") { //Requête HTTP POST (pas supportée)
		strcpy(buffer, "HTTP/1.0 500 Internal Error\r\nServer: SpaceCrafter (HTTP/BETA)\r\nContent-Length: 0\r\n\r\n");
		send(clientSocketTab[client]);
		close(client);
		return true;
	} else 
		return false;
	#else
	return true;
	#endif
}

void ServerSocket::computeNormalString(unsigned int client, std::string string)
{
	//TODO proprer
	if(string.substr(0, 7) == "$NOTICE") { //Commande NOTICE
		strcpy(buffer, "$NOTICE $LOGON $LOGOFF");
		send(clientSocketTab[client]);
	} else 
	if(string.substr(0, 4) == "$LOG") { //Commande LOG
		if(string.substr(4, 2) == "ON" && !clientBroadcastTab[client]) { //LOGON
			clientBroadcastTab[client] = true; //Changement des préférences du client
			strcpy(buffer, "Vous receverez maintenant les logs\n");
		} else 
		if(string.substr(4, 3) == "OFF" && clientBroadcastTab[client]) { //LOGOFF
			clientBroadcastTab[client] = false; //Changement des préférences du client
			strcpy(buffer, "Vous receverez maintenant PLUS les logs\n");
		} else
			strcpy(buffer, "REQUEST ERROR");
		send(clientSocketTab[client]); //Envoi du buffer au client
	} else {
		inputQueue.push(string); //Rajoute la chaine à la file de sortie
		//broadcast(clientIp(client) + CLIENT_SEPARATOR1 + string + '\n'); //Envoi la chaîne à tous les clients
	}
}

void ServerSocket::checkDataToSend()
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT)) debugOut("CHECKDATATOSEND_START"); //Debug
	#endif

	if(lock(outputting) == IO_NO_ERROR) {
		while(!outputQueue.empty()) { //File non-vide
			broadcast(outputQueue.front() + '\n'); //Envoi à tous les clients de la tête de la file
			outputQueue.pop(); //Défile
		}
		unlock(outputting);
	}

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT)) debugOut("CHECKDATATOSEND_END"); //Debug
	#endif
}

int ServerSocket::broadcast(std::string data)
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("BROADCAST_START"); //Debug
	#endif

	#ifdef IO_DEBUG_DEBUG_IN_LOOP
	if(debug(DEBUG_FCT, FLUX_EVENT))
		debugOut("BROADCAST_DATA "+ data); //Debug
	#endif

	strcpy(buffer, data.c_str()); //Prépare le message
	unsigned int sent = 0; //Nombre de clients auquels est envoyé la données
	for (unsigned int client = 0; client < maxClients; client++) { //Parcours de tous les clients connectés
		if(clientBroadcastTab[client]) { //Si le client demande des feedback
			send(clientSocketTab[client]); //Envoi au client
			sent++; //Incrémente le nombre total de requêtes envoyées
		}
	}

	return sent;

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("BROADCAST_END"); //Debug
	#endif
}

int ServerSocket::send(TCPsocket client)
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("SEND_START"); //Debug
	#endif

	unsigned int size = strlen(buffer) + 1; //Taille de la chaine
	unsigned int sendCount = SDLNet_TCP_Send(client, (void *)buffer, size); //Envoi le contenu du buffer au client
	if(sendCount < size) { //Problème lors de l'envoi
		requestSendFailed++; //Incrémente le nombre total de d'erreurs d'envoi de requête
		#ifdef IO_DEBUG_WARN_IN_LOOP
		if(debug(WARN_CLIENT, FLUX_EVENT))
			debugOut("SDL_SEND_ERROR"); //Debug
		#endif
		return SDL_SEND_ERROR_CODE;
	} else	requestSend++; //Incrémente le nombre total de requêtes envoyées

	dataSend += sendCount; //Incrément le total de données envoyées

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("SEND_END"); //Debug
	#endif

	return IO_NO_ERROR;
}

int ServerSocket::close(unsigned int client)
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, CLIENT_EVENT))
		debugOut("CLIENT_CLOSE_START"); //Debug
	#endif

	#ifdef IO_DEBUG_INFO_IN_LOOP
	if(debug(INFO_CONNEXION, CLIENT_EVENT))
		debugOut("CLIENT_DISCONECTED "+ clientIp(client)); //Debug
	#endif

	int socketCount = SDLNet_TCP_DelSocket(socketSet, clientSocketTab[client]); //Suppression du socket client du SocketSet
	if(socketCount < 0) {
		if(debug(ERROR_SOCKET, FLUX_EVENT)) debugOut("SDL_DELSOCKET_CLIENT_ERROR"); //Debug
		return SDL_DELSOCKET_CLIENT_ERROR_CODE;
	}
	SDLNet_TCP_Close(clientSocketTab[client]); //Fermeture du socket client
	clientSocketTab[client] = NULL; //Nullation du socket client
	clientBroadcastTab[client] = false; //Falsation de l'état de la demande de feedback
	clientCount--; //Décrémentation du nombre de clients connectés

	#ifdef IO_DEBUG_INFO_IN_LOOP
	if(debug(INFO_CONNEXION, CLIENT_EVENT))
		debugOut("CLIENT_COUNT " + toString(clientCount) + "/" + toString(maxClients)); //Debug
	#endif

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("CLIENT_CLOSE_END"); //Debug
	#endif

	return IO_NO_ERROR;
}

int ServerSocket::killThread()
{
	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("KILLTHREAD_START"); //Debug
	#endif

	stopThread = true; //Demande d'arrêt du thread
	SDL_WaitThread(thread, &threadReturnValue); //Attente du thread

	#ifdef IO_DEBUG_TRACE_IN_LOOP
	if(debug(TRACE_FCT, SERVEUR_EVENT))
		debugOut("KILLTHREAD_END"); //Debug
	#endif

	return threadReturnValue;
}

int ServerSocket::lock(SDL_mutex *mutex)
{
	if (SDL_LockMutex(mutex)) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_LOCKMUTEX_ERROR " + (std::string)SDLNet_GetError()); //Debug
		return SDL_LOCKMUTEX_ERROR_CODE;
	}

	return IO_NO_ERROR;
}

int ServerSocket::unlock(SDL_mutex *mutex)
{
	if (SDL_UnlockMutex(mutex)) {
		if(debug(ERREUR_FATAL, SERVEUR_EVENT))
			debugOut("SDL_UNLOCKMUTEX_ERROR " + (std::string)SDLNet_GetError()); //Debug
		return SDL_UNLOCKMUTEX_ERROR_CODE;
	}
	return IO_NO_ERROR;
}
