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

//#include "spacecrafter.hpp"
#include "tools/io.hpp" //ServerSocket
#include "tools/utility.hpp"
#include "tools/log.hpp" //ServerSocket
#include "tools/app_settings.hpp"

#ifdef WIN32
#define NOMINMAX
#include <direct.h>
#endif

/*
Application control server
Utility: this program allows to talk with the application through the network
Usage: to include in the C++ program
Author: Aurélien Schwab <aurelien.schwab+dev@gmail.com> for association-sirius.org
Updated on 17/07/2017
*/

template<class T>
std::string toString(const T& t) //ServerSocket
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}


/* Valeurs par défaut */
#define DEFAULT_PORT		1234 //Default port
#define MAX_CLIENTS			16 //Default simulated client limit
#define BUFFER_SIZE 		65536 //Default buffer size (in bytes)
#define STATS_PERIOD 		5000 //Minimum time between two recaps in the loop (in milliseconds)
#define MAX_BUFFER 			1024 //The size of a buffer to be sent as a response via TCP

/* Warning values */
#define LOT_OF_CLIENTS 		32 //Limit of simulated clients considered large and untested
#define SMALL_BUFFER_SIZE 	512 //Buffer size considered dangerously small (must be larger than the messages that can be sent by the server)
#define BIG_BUFFER_SIZE 	1048576 //Buffer size considered unnecessarily large


ServerSocket::ServerSocket(unsigned int port)
{
	initErrorCode = init(port, MAX_CLIENTS, BUFFER_SIZE);
	if(initErrorCode != IO_NO_ERROR) throw initErrorCode;
}

ServerSocket::ServerSocket(unsigned int port, unsigned int maxClients, unsigned int bufferSize)
{
	initErrorCode = init(port, maxClients, bufferSize);
	if(initErrorCode != IO_NO_ERROR) throw initErrorCode;
}

int ServerSocket::init(unsigned int port, unsigned int maxClients, unsigned int bufferSize)
{
	/* Constructor */
	this->port = port;
	this->maxClients = maxClients;
	this->bufferSize = bufferSize;

	debugOut("-- INIT --", LOG_TYPE::L_DEBUG); //Debug
	debugOut("Port " + toString(port) + DEBUG_SEPARATOR3 + "Slots " + toString(maxClients) + DEBUG_SEPARATOR3 + "Buffer " +
			toString(bufferSize) + " B" + DEBUG_SEPARATOR3 + ")", LOG_TYPE::L_INFO); //Debug

	/* Configuration warnings */
	if(port == 0)
		debugOut("NULL_PORT", LOG_TYPE::L_WARNING); //Debug
	else if(port < 1024)
		debugOut("ADMIN_PORT "+ toString(port), LOG_TYPE::L_WARNING); //Debug

	if(maxClients == 0)
		debugOut("NULL_MAXCLIENTS", LOG_TYPE::L_WARNING); //Debug
	else if(maxClients > LOT_OF_CLIENTS)
		debugOut("LOT_OF_MAXCLIENTS "+ toString(maxClients), LOG_TYPE::L_WARNING); //Debug

	if(bufferSize == 0)
		debugOut("NULL_BUFFER", LOG_TYPE::L_WARNING); //Debug
	else if(bufferSize < SMALL_BUFFER_SIZE)
		debugOut("SMALL_BUFFER "+ toString(bufferSize), LOG_TYPE::L_WARNING); //Debug
	else if(bufferSize > BIG_BUFFER_SIZE)
		debugOut("BIG_BUFFER "+ toString(bufferSize), LOG_TYPE::L_WARNING); //Debug

	/*Initializations */
	initErrorCode = -1;
	serverOpen = false;
	stopThread = false;
	activeSocketsCount = 0;
	clientCount = 0;
	broadcastId = 0;

	/* Initialization of statistics variables */
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

	/* Initialization of SDL_net */
	if (SDLNet_Init() < 0) {
		debugOut("SDL_NET_INIT_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_NET_INIT_ERROR_CODE;
	}

	/* Preparation of the server connection */
	if(SDLNet_ResolveHost(&serverIP, NULL, this->port) < 0) {
		debugOut("SDL_RESOLEVHOST_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_RESOLEVHOST_ERROR_CODE;
	}

	/* Creation of a SocketSet to monitor the sockets */
	socketSet = SDLNet_AllocSocketSet(maxClients + 1);
	if (socketSet == NULL) {
		debugOut("SDL_ALLOCSOCKETSET_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_ALLOCSOCKETSET_ERROR_CODE;
	}

	/* Creation of the mutex to protect the thread execution */
	running = SDL_CreateMutex();
	if (running == NULL) {
		debugOut("SDL_CREATEMUTEX_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_CREATEMUTEX_ERROR_CODE;
	}

	/* Initialization of the client sockets array */
	clientSocketTab = new TCPsocket[maxClients]; //Allocation of the client sockets array
	if(clientSocketTab == NULL) {
		debugOut("NEW_TCPSOCKET_TAB_ERROR", LOG_TYPE::L_ERROR); //Debug
		return NEW_TCPSOCKET_TAB_ERROR_CODE;
	}
	clientBroadcastTab = new bool[maxClients];
	if(clientBroadcastTab == NULL) {
		debugOut("NEW_BOOL_TAB_ERROR", LOG_TYPE::L_ERROR); //Debug
		return NEW_BOOL_TAB_ERROR_CODE;
	}
	for (unsigned int i = 0; i < maxClients; i++)	{
		clientSocketTab[i] = NULL; //Initialization of all client sockets to NULL
		clientBroadcastTab[i] = false;
	}

	/* Initialization of the buffer */
	buffer = new char[bufferSize];
	if(buffer == NULL) {
		debugOut("NEW_BUFFER_ERROR", LOG_TYPE::L_ERROR); //Debug
		return NEW_BUFFER_ERROR_CODE;
	}

	/* Creation of the mutex to protect the client input queue */
	outputting = SDL_CreateMutex();
	if (outputting == NULL) {
		debugOut("SDL_CREATEMUTEX_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_CREATEMUTEX_ERROR_CODE;
	}

	/* Creation of a mutex to protect the client exit queue */
	inputting = SDL_CreateMutex();
	if (inputting == NULL) {
		debugOut("SDL_CREATEMUTEX_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_CREATEMUTEX_ERROR_CODE;
	}

	return IO_NO_ERROR; //No error
}


std::string ServerSocket::replace(std::string base, const std::string from, const std::string to)
{
	std::string SecureCopy = base;
	for (size_t start_pos = SecureCopy.find(from); start_pos != std::string::npos; start_pos = SecureCopy.find(from,start_pos)) SecureCopy.replace(start_pos, from.length(), to);
	return SecureCopy;
}


int ServerSocket::open()
{
	debugOut("-- OPEN --", LOG_TYPE::L_DEBUG); //Debug
	debugOut("SERVER_START "+ toString(port), LOG_TYPE::L_INFO); //Debug

	/* Opening of the server socket */
	serverSocket = SDLNet_TCP_Open(&serverIP);
	if (!serverSocket) {
		debugOut("SDL_OPEN_SERVER_SOCKET_ERROR "+ (std::string)SDLNet_GetError() + " (port "  + toString(this->port) + ")", LOG_TYPE::L_ERROR); //Debug
		return SERVER_SOCKET_OPEN_ERROR_CODE;
	}

	/* Change the server status flag */
	serverOpen = true;

	/* Add the server socket to the socketSet to monitor it */
	if(SDLNet_TCP_AddSocket(socketSet, serverSocket) <= 0) {
		debugOut("SDL_ADDSOCKET_SERVER_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		SDLNet_TCP_Close(serverSocket); //Close the server socket
		return SDL_ADDSOCKET_SERVER_ERROR_CODE;
	}

	/* Launch the processing thread */
	thread = SDL_CreateThread(threadWrapper, "Loop thread", this);
	if (thread == NULL) {
		debugOut("SDL_CREATETHREAD_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_CREATETHREAD_ERROR_CODE;
	}

	return IO_NO_ERROR; //No error
}

int ServerSocket::close()
{
	debugOut("-- CLOSE --", LOG_TYPE::L_DEBUG); //Debug
	if(!serverOpen) {
		debugOut("SERVER_NOT_OPEN", LOG_TYPE::L_WARNING); //Debug
		return SERVER_NOT_OPEN_CODE;
	}

	debugOut("SERVER_STOP", LOG_TYPE::L_INFO); //Debug
	killThread();

	//Close all open clients
	strcpy(buffer, "GOODBYE"); //Preparation of the message
	for (unsigned int client = 0; client < maxClients; client++) { //Scans all clients
		if(clientCount <= 0) break; //Si on a déjà fermé tous les sockets clients on s'arrête
		if (clientSocketTab[client] != NULL) { //If the socket is used
			send(clientSocketTab[client]); //Send the message to the client
			close(client); //Closing operations of the client socket
		}
	}

	SDLNet_TCP_Close(serverSocket);	//Closing the server socket
	serverOpen = false; //Change the server status flag

	return IO_NO_ERROR;
}

ServerSocket::~ServerSocket()
{
	debugOut("-- DELETE --", LOG_TYPE::L_DEBUG); //Debug

	if(serverOpen) close(); //Closing the server

	SDLNet_FreeSocketSet(socketSet);//Release of the SocketSet
	delete[] clientSocketTab; //Release socket array
	delete[] buffer; //Release buffer
	SDL_DestroyMutex(running); //Release the mutex
	SDLNet_Quit(); //Closing of SDL_net

	stats(); //Display statistics

	while(!inputQueue.empty()) inputQueue.pop(); //Empty input queue
	while(!outputQueue.empty()) outputQueue.pop(); //Empty output queue
}

void ServerSocket::stats()
{
	debugOut("-- STATS --", LOG_TYPE::L_DEBUG); //Debug

	if(connection) {
		debugOut("CONNECTION "+ toString(connection) + " (" + toString(maxSimultaneousClient) + " max)", LOG_TYPE::L_DEBUG);
		debugOut("REQUEST_RECIEVED "+ toString(requestRecieved) + " requests (" + toString(dataRecieved) + " bytes)", LOG_TYPE::L_DEBUG);
		debugOut("REQUEST_SEND "+ toString(requestSend) + " requests (" + toString(dataSend) + " bytes)", LOG_TYPE::L_DEBUG);

		debugOut("CONNECTION "+ humanReadable(connection) + " (" + humanReadable(maxSimultaneousClient) + " max)", LOG_TYPE::L_INFO);
		debugOut("REQUEST_RECIEVED "+ humanReadable(requestRecieved) + " (" + humanReadable(dataRecieved) + "B)", LOG_TYPE::L_INFO);
		debugOut("REQUEST_SEND "+ humanReadable(requestSend) + " (" + humanReadable(dataSend) + "B)", LOG_TYPE::L_INFO);

	}

	debugOut("DEBUG_FULL_COUNT "+ toString(refusedConnectionServerFull), LOG_TYPE::L_DEBUG);
	if(refusedConnectionServerFull)
		debugOut("DEBUG_FULL_COUNT "+ humanReadable(refusedConnectionServerFull), LOG_TYPE::L_WARNING);

	debugOut("DEBUG_CANNOTACCEPT_COUNT "+ toString(cannotAcceptClient), LOG_TYPE::L_DEBUG);
	debugOut("DEBUG_CANNOTACCEPT_COUNT "+ humanReadable(cannotAcceptClient), LOG_TYPE::L_WARNING);
	debugOut("DEBUG_OVERFLOW_COUNT "+ toString(possibleBufferOverflow), LOG_TYPE::L_DEBUG);
	debugOut("DEBUG_OVERFLOW_COUNT "+ humanReadable(possibleBufferOverflow), LOG_TYPE::L_WARNING);
	debugOut("DEBUG_SENDFAILED_COUNT "+ toString(requestSendFailed), LOG_TYPE::L_DEBUG);
	debugOut("DEBUG_SENDFAILED_COUNT "+ humanReadable(requestSendFailed), LOG_TYPE::L_WARNING);
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
		if(inputQueue.empty())
			data = "";
		else {
			data = inputQueue.front();
			inputQueue.pop();
		}
		unlock(inputting);
		return data;
	} else return "";
}

void ServerSocket::setOutput(std::string data)
{
	debugOut("Data info OUT : " + data, LOG_TYPE::L_INFO);
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

void ServerSocket::debugOut(std::string msg, LOG_TYPE log)
{
	cLog::get()->write("TCP : " + msg, log, LOG_FILE::TCP);
}

/* thread */

int ServerSocket::threadWrapper(void *Data)
{
	return ((ServerSocket *)Data)->run();
}

int ServerSocket::run()
{
	debugOut("-- RUN --", LOG_TYPE::L_DEBUG); //Debug

	while(true) {
		if(stopThread) {
			stopThread = false;
			return IO_NO_ERROR;
		}
		activeSocketsCount = SDLNet_CheckSockets(socketSet, 1); //Wait for activity on the SocketSet (1ms timeout)
		if(lock(running) == IO_NO_ERROR) { //Thread actif
			if(activeSocketsCount > 0) { //If there are active sockets
				debugOut("SDL_CHECKSOCKETS_ACTIVITY " + toString(activeSocketsCount), LOG_TYPE::L_DEBUG); //Debug

				checkNewClient(); //See if there are new clients and process them
				if(activeSocketsCount) checkNewData(); //See if there are new data and process them
			} else if(activeSocketsCount < 0) { //If there was an error while retrieving the number of active sockets
				debugOut("SDL_CHECKSOCKETS_ERROR " + (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
				close(); //Shuts down the server
				return SDL_CHECKSOCKETS_ERROR_CODE;
			}

			checkDataToSend(); //Checks if there is data to send to clients

		unlock(running); //Thread inactif
		}
	}
	return IO_NO_ERROR;
}

void ServerSocket::checkNewClient()
{
	debugOut("-- CHECK NEW CLIENT --", LOG_TYPE::L_DEBUG); //Debug

	if (SDLNet_SocketReady(serverSocket) != 0) { //If there are new clients
		if (clientCount < maxClients) { //If there is room for the client
			unsigned int freeSpot = maxClients - 1;
			for (unsigned int socket = 0; socket < maxClients; socket++) {
				if (clientSocketTab[socket] == NULL) {
					freeSpot = socket; //Keeps the number of the socket to use
					break;
				}
			}

			clientSocketTab[freeSpot] = SDLNet_TCP_Accept(serverSocket); //Accept the client and put it in the sockets table
			debugOut("New client " + clientIp(freeSpot), LOG_TYPE::L_INFO); //Debug
			if(clientSocketTab[freeSpot] == NULL) { //Error when accepting the client
				cannotAcceptClient++; //Increment the total number of errors when accepting the client
				debugOut("SDL_ACCEPT_CLIENT_ERROR", LOG_TYPE::L_WARNING); //Debug
			} else {
				if(SDLNet_TCP_AddSocket(socketSet, clientSocketTab[freeSpot]) <= 0) { //Adds the client socket to the SocketSet
					debugOut("SDL_ADDSOCKET_CLIENT_ERROR "+ (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
					close(freeSpot); //Closes the temporary socket
				} else {
					connection++; //Increases the total number of connections
					clientCount++; //Increases the number of connected clients
					if(clientCount > maxSimultaneousClient)
						maxSimultaneousClient = clientCount; //Update the maximum number of simulataneously connected clients

					//strcpy(buffer, SERVER_MSG_HELLO_FINAL); //Prepares the message
					//send(clientSocketTab[freeSpot]); //Send message to client //Deleted to keep it more sober and better support http clients or other

					debugOut("CLIENT_COUNT " + toString(clientCount) + "/" + toString(maxClients), LOG_TYPE::L_INFO); //Debug
				}
			}
		} else { //If there is no space for the client
			refusedConnectionServerFull++; //Increment the number of refused connections due to a full server
			debugOut("server full", LOG_TYPE::L_WARNING); //Debug
			TCPsocket tempSock = SDLNet_TCP_Accept(serverSocket); //Accept the client in a socket
			if(tempSock == NULL) { //Error when accepting client
				cannotAcceptClient++;

			debugOut("SDL_ACCEPT_CLIENT_ERROR", LOG_TYPE::L_WARNING); //Debug
			}

			strcpy(buffer, "SERVER_FULL"); //Prepares the message
			send(tempSock); //Sends message to client
			SDLNet_TCP_Close(tempSock); //Closes the socket

		}
		activeSocketsCount--; //Decreases the number of active sockets
	}
}

void ServerSocket::checkNewData()
{
	debugOut("-- CHECK NEW DATA --", LOG_TYPE::L_DEBUG); //Debug

	int clientSocketActivity;
	for (unsigned int client = 0; client < maxClients; client++) { //Browse all connected clients

		if(clientSocketTab[client] != NULL) { //If the socket is open

			clientSocketActivity = SDLNet_SocketReady(clientSocketTab[client]); //Retrieve the socket activity
			if (clientSocketActivity) { //If there is activity on the socket

				int receivedByteCount = SDLNet_TCP_Recv(clientSocketTab[client], buffer, bufferSize); //Receive data from the client
				if (receivedByteCount <= 0) { //Disconnecting the client

					debugOut("RESETED_BY_PEER "+ clientIp(client), LOG_TYPE::L_INFO); //Debug

					close(client); //Close client socket operations
				} else if ((unsigned int)receivedByteCount >= bufferSize) { //Buffer overflow
					possibleBufferOverflow++; //Increments the total number of buffer overflows
					strcpy(buffer, "SERVER_OVERFLOW"); //Prepares the message
					send(clientSocketTab[client]); //Sends the message

					debugOut("BUFFER_OVERFLOW too many data "+ clientIp(client), LOG_TYPE::L_WARNING); //Debug

					close(client); //Closing the client socket
				} else {

					requestRecieved++; //Increment the total number of received requests
					dataRecieved += receivedByteCount; //Increment the total number of received data

					debugOut("CLIENT_DATA_QUANTITY " + clientIp(client) + " " + toString(receivedByteCount), LOG_TYPE::L_INFO); //Debug

					for(int i=0; i < receivedByteCount; i++) if(buffer[i] == '\r' || buffer[i] == '\0') buffer[i] = '\n';// Remplace \r et \0 par \n
					buffer[receivedByteCount] = '\0'; //Terminates the chain (you never know)
					computeNewData(client); //Processes new data
				}
				activeSocketsCount--; //Decreases the number of active sockets
			}
		}
		if(activeSocketsCount <= 0) break; //Stops if the number of active sockets has been processed
	}
}

void ServerSocket::computeNewData(unsigned int client)
{
	debugOut("-- COMPUTE NEW DATA --", LOG_TYPE::L_DEBUG); //Debug
	std::string data = buffer; //Transforms the buffer data into a string
	debugOut("DATA_AS_STRING "+ data, LOG_TYPE::L_DEBUG); //Debug
	unsigned int begin = 0; //Start of the string
	unsigned int end; //End of the string
	bool continues;
	do {
		end = data.find('\n', begin); //Finds the line break
		if(begin < data.size() && (end - begin >= 1))
			continues = computeString(client, data.substr(begin, end - begin)); //Before \n
		begin = end + 1; //After \n
	} while(end != (unsigned int)std::string::npos && continues); //As long as there are n's in the chain
}

bool ServerSocket::computeString(unsigned int client, std::string string)
{
	debugOut("-- COMPUTE STRING --", LOG_TYPE::L_DEBUG); //Debug

	if(computeHttp(client, string)) {
		return false; //HTTP client
	} else {
		computeNormalString(client, string);
		return true;
	}
}

bool ServerSocket::computeHttp(unsigned int client, std::string string)
{
	//TODO proprer
	#ifdef LINUX
	if(string.substr(0,3) == "GET") { //HTTP client

		/* Processing of the url */
		std::string path = AppSettings::Instance()->getWebDir();

		printf("%s\n", string.c_str()); //Debug

		std::string url = string.substr(4, string.substr(4, std::string::npos).find(' ')); //Retrieve the url only
		printf("URL : \"%s\"\n", url.c_str()); //Debug

		std::string urlFile = url.substr(0, url.find('?')); //Retrieve the url without parameters
		printf("FILE : \"%s\"\n", urlFile.c_str()); //Debug

		if(urlFile[urlFile.size()-1] == '/') urlFile.append("index.html"); //Modify the url to index.html

		std::string filename = path + urlFile; //Path to the file locally
		printf("\"FILE PATH : %s\"\n", filename.c_str()); //debug

		if(url.find('?') != std::string::npos) { //Parameters in the url
			std::string urlParam = url.substr(url.find('?')+1, std::string::npos); //Retrieve parameters only
			printf("PARAM : \"%s\"\n", urlParam.c_str()); //Debug

			if(urlParam.find("command=", 0) != std::string::npos) { //Parameter command present
				std::string command = urlParam.substr(urlParam.find("command=", 0), std::string::npos); //Retrieve from parameter
				command = command.substr(8, command.find('&')-8); //Retrieves command only
				command = replace(command, "%20", " "); //Decodes spaces
				command = replace(command, "+", " "); //Decodes spaces
				command = replace(command, "%3A", ":"); //Decodes ":"
				printf("COMMAND : \"%s\"\n", command.c_str());
				inputQueue.push(command); //Adds the string to the output queue
				broadcast(clientIp(client) + CLIENT_SEPARATOR2 + "HTTP" + CLIENT_SEPARATOR1 + command + '\n'); //Sends the string to all clients
			}
		}

		/* Send the file */
		struct stat filestat;
		if(!stat(filename.c_str(), &filestat)) { //Retrieves statistics from the file
			FILE* file = fopen(filename.c_str(), "r"); //Open the file in read mode
			if(file != NULL) { //File opened
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
				SDLNet_TCP_Send(clientSocketTab[client], (void *)buffer, strlen(buffer)); //Send headers
				unsigned int size;
				do {
					size = fread(buffer, 1, bufferSize, file); //Read the file in the buffer
					SDLNet_TCP_Send(clientSocketTab[client], (void *)buffer, size); //Send the buffer
				} while(size == bufferSize); //Not at the end of the file
				fclose(file); //Closing the file
			}
		} else { //Problem when opening the file (non-existent...)
			strcpy(buffer, "HTTP/1.0 500 Internal Error\r\nServer: SpaceCrafter (HTTP/BETA)\r\nContent-Length: 0\r\n\r\n");
			send(clientSocketTab[client]); //Sending of the headers
		}

		close(client); //Closing the connection
		return true;
	} else
	if (string.substr(0,4) == "POST") { //HTTP POST request (not supported)
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
	if(string.substr(0, 7) == "$NOTICE") { //command NOTICE
		strcpy(buffer, "$NOTICE $LOGON $LOGOFF");
		send(clientSocketTab[client]);
	} else
	if(string.substr(0, 4) == "$LOG") { //LOG command
		if(string.substr(4, 2) == "ON" && !clientBroadcastTab[client]) { //LOGON
			clientBroadcastTab[client] = true; //Change of customer preferences
			strcpy(buffer, "Vous receverez maintenant les logs\n");
		} else
		if(string.substr(4, 3) == "OFF" && clientBroadcastTab[client]) { //LOGOFF
			clientBroadcastTab[client] = false; //Change of customer's preferences
			strcpy(buffer, "Vous receverez maintenant PLUS les logs\n");
		} else
			strcpy(buffer, "REQUEST ERROR");
		send(clientSocketTab[client]); //Send buffer to client
	} else {
		inputQueue.push(string); //Add string to output queue
		//broadcast(clientIp(client) + CLIENT_SEPARATOR1 + string + '\n'); //Send string to all clients
	}
}

void ServerSocket::checkDataToSend()
{
	if(lock(outputting) == IO_NO_ERROR) {
		while(!outputQueue.empty()) { //Non-empty queue
			broadcast(outputQueue.front() + '\n'); //Send to all clients from the head of the queue
			outputQueue.pop(); //Scrolls
		}
		unlock(outputting);
	}
}

int ServerSocket::broadcast(std::string data)
{
	debugOut("-- BROADCAST --", LOG_TYPE::L_DEBUG); //Debug
	debugOut("BROADCAST_DATA "+ data, LOG_TYPE::L_DEBUG); //Debug

	strcpy(buffer, data.c_str()); //Prepares the message
	unsigned int sent = 0; //Number of clients to which the data is sent
	for (unsigned int client = 0; client < maxClients; client++) { //Path of all connected clients
		if(clientBroadcastTab[client]) { //If the client requests feedback
			send(clientSocketTab[client]); //Sends to client
			sent++; //Increates the total number of requests sent
		}
	}
	return sent;
}

int ServerSocket::send(TCPsocket client)
{
	debugOut("-- SEND --", LOG_TYPE::L_DEBUG); //Debug

	unsigned int size = strlen(buffer) + 1; //Size of the string
	unsigned int sendCount = SDLNet_TCP_Send(client, (void *)buffer, size); //Sends the content of the buffer to the client
	if(sendCount < size) { //Problem while sending
		requestSendFailed++; //Increments the total number of request sending errors
		debugOut("SDL_SEND_ERROR", LOG_TYPE::L_WARNING); //Debug
		return SDL_SEND_ERROR_CODE;
	} else	requestSend++; //Increment the total number of requests sent

	dataSend += sendCount; //Increments the total amount of data sent

	return IO_NO_ERROR;
}

int ServerSocket::close(unsigned int client)
{
	debugOut("-- CLIENT CLOSE --", LOG_TYPE::L_DEBUG); //Debug
	debugOut("CLIENT_DISCONECTED "+ clientIp(client), LOG_TYPE::L_INFO); //Debug
	int socketCount = SDLNet_TCP_DelSocket(socketSet, clientSocketTab[client]); //Removal of the client socket from the SocketSet
	if(socketCount < 0) {
		debugOut("SDL_DELSOCKET_CLIENT_ERROR", LOG_TYPE::L_ERROR); //Debug
		return SDL_DELSOCKET_CLIENT_ERROR_CODE;
	}
	SDLNet_TCP_Close(clientSocketTab[client]); //Closing the client socket
	clientSocketTab[client] = NULL; //Nullation of the client socket
	clientBroadcastTab[client] = false; //Falsify the status of the feedback request
	clientCount--; //Decrease the number of connected clients

	debugOut("CLIENT_COUNT " + toString(clientCount) + "/" + toString(maxClients), LOG_TYPE::L_INFO); //Debug

	return IO_NO_ERROR;
}

int ServerSocket::killThread()
{
	debugOut("-- KILL THREAD --", LOG_TYPE::L_DEBUG); //Debug

	stopThread = true; //Stop thread request
	SDL_WaitThread(thread, &threadReturnValue); //Wait for the thread

	return threadReturnValue;
}

int ServerSocket::lock(SDL_mutex *mutex)
{
	if (SDL_LockMutex(mutex)) {
		debugOut("SDL_LOCKMUTEX_ERROR " + (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_LOCKMUTEX_ERROR_CODE;
	}
	return IO_NO_ERROR;
}

int ServerSocket::unlock(SDL_mutex *mutex)
{
	if (SDL_UnlockMutex(mutex)) {
		debugOut("SDL_UNLOCKMUTEX_ERROR " + (std::string)SDLNet_GetError(), LOG_TYPE::L_ERROR); //Debug
		return SDL_UNLOCKMUTEX_ERROR_CODE;
	}
	return IO_NO_ERROR;
}
