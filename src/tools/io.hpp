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

//#include "spacecrafter.hpp"
//#include "tools/app_settings.hpp"
#include "tools/log.hpp"

#ifdef LINUX
//for pipe
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif


/*
Application control server
Utility: this program allows to talk with the application through the network
Usage: to include in the C++ program
Author: Aurélien Schwab <aurelien.schwab+dev@gmail.com> for association-sirius.org
Updated on 17/05/2016
*/

/* Error code */
#define IO_NO_ERROR 0 //By error

/* Error codes in the initialization */
#define SDL_NET_INIT_ERROR_CODE			1 //Error in SDL initialization
#define SDL_RESOLEVHOST_ERROR_CODE 		2 //Error when preparing the structure for the server
#define SDL_ALLOCSOCKETSET_ERROR_CODE 	3 //Error during SocketSet allocation
#define SDL_CREATEMUTEX_ERROR_CODE 		4 //Error when creating the thread activity lock mutex
#define NEW_TCPSOCKET_TAB_ERROR_CODE 	5 //Error when allocating the client sockets array
#define NEW_BOOL_TAB_ERROR_CODE 		6 //Error while allocating the client sockets array
#define NEW_BUFFER_ERROR_CODE 			7 //Error while allocating the buffer
/* Error codes in opening */
#define SERVER_SOCKET_OPEN_ERROR_CODE 	101 //Error when opening the server socket
#define SDL_ADDSOCKET_SERVER_ERROR_CODE	102 //Error when adding the server socket to the SocketSet
#define SDL_CREATETHREAD_ERROR_CODE 	103
/* Error codes in processing */
#define SDL_LOCKMUTEX_ERROR_CODE 		201
#define SDL_UNLOCKMUTEX_ERROR_CODE 		202
#define SDL_CHECKSOCKETS_ERROR_CODE 	203 //Error when checking the SocketSet
#define SDL_SEND_ERROR_CODE 			204
#define SDL_DELSOCKET_CLIENT_ERROR_CODE 205
/* Error codes in closing */
#define SERVER_NOT_OPEN_CODE 			301

#define CLIENT_SEPARATOR1 				"|"
#define CLIENT_SEPARATOR2 				"/"
#define DEBUG_SEPARATOR3 				" | " //Third error in the debug


//! A line of protocol together with the connection it belongs to.
//! INTENT §5.47: `setOutput` names no addressee, and the addressee used to be
//! dropped at the first hop - `computeNormalString` knows which connection a
//! command arrived on and pushed the bare string - so a `get`'s reply had
//! nowhere to go but the feedback subscription ($LOGON), and with nobody
//! subscribed it was popped off the queue and lost. The socket layer is the
//! only place that knows the origin of a request, so it is the place that has
//! to carry it. A connection is named by its SLOT **and** by the id that slot
//! held when the line was read: slots are reused, so the id is what makes a
//! reply follow the connection rather than the number (I5).
struct ClientMessage {
	unsigned int client = 0;	//!< index in clientSocketTab
	unsigned int id = 0;		//!< connection id, 0 = no connection asked for this
	std::string data;
};

class ServerSocket {
public:
	/* Constructors and destructor */
	ServerSocket(unsigned int port); //Simple constructor 
	ServerSocket(unsigned int port, unsigned int maxClients, unsigned int bufferSize); //Advanced constructor 
	~ServerSocket(); //Destructor

	/* Action functions on the server */
	int open(); //Function to open the server socket
	int close(); //Function to close the server socket

	/* Function to display non-zero statistics */
	void stats();

	//! Transfer incoming data from TCP/IP inside the program.
	//! Also latches WHICH connection this line came from, so that whatever the
	//! application produces while serving it can be sent back there. An empty
	//! return clears that latch: the caller has drained its batch, and output
	//! produced afterwards (a script, the TUI, a keypress) was asked for by
	//! nobody. So `getInput` and `setOutput` must be called from the SAME
	//! thread, which is the application's update thread - the server thread
	//! never touches either.
	std::string getInput();
	//! Transfer of internal data outside the program: to the connection that
	//! asked for it if there is one, and to the clients that subscribed to the
	//! feedback channel with $LOGON in any case (they were the only recipients
	//! before §5.47 and they keep receiving exactly what they received).
	void setOutput(std::string data);

private:
	/* Configurable variables */
	unsigned int port; //Server listening port
	unsigned int maxClients; //Maximum number of clients (as the server takes a place in the socket set, this number is equal to the size of the set -1)
	unsigned int bufferSize; //Receive buffer size
	LOG_TYPE logType; //Log type (application specific)

	/* Status variables */
	bool serverOpen; //Server status
	unsigned int clientCount; //Number of clients currently connected to the server
	unsigned int broadcastId; //Broadcast message id

	/* Statistics variables */
	unsigned int maxSimultaneousClient; //Maximum number of clients simulataneously connected

	unsigned int connection; //Total number of connections
	unsigned int refusedConnectionServerFull; //Total number of refused connections due to server full
	unsigned int cannotAcceptClient; //Total number of errors during client acceptance

	unsigned int requestRecieved; //Total number of requests
	unsigned int dataRecieved; //Total received data
	unsigned int possibleBufferOverflow; //Total number of buffer overflows

	unsigned int requestSend; //Total number of requests sent
	unsigned int dataSend; //Total data sent
	unsigned int requestSendFailed; //Total number of errors while sending the request

	/* Server variables */
	IPaddress serverIP; //Server IP (0.0.0.0 to listen on all server IPs)
	TCPsocket serverSocket; //Server listening socket
	SDLNet_SocketSet socketSet; //Socket monitoring table
	TCPsocket* clientSocketTab; //Client sockets table
	bool* clientBroadcastTab; //Feedback request table
	unsigned int* clientIdTab; //Connection id per slot (0 = free); never reused
	unsigned int lastClientId; //Last id handed out

	/* Thread variables */
	SDL_Thread *thread; //Thread of the server that waits for the packets
	int threadReturnValue; //Return value of the thread
	bool stopThread;
	int lock(SDL_mutex *mutex);
	int unlock(SDL_mutex *mutex);
	SDL_mutex *running; //Mutex of active server
	int activeSocketsCount; //Number of active sockets
	//! The RECEIVE buffer, and only that (§5.73): `tcp_buffer_in_size` bytes,
	//! filled by `SDLNet_TCP_Recv` and read back by `computeNewData`. The HTTP
	//! branch also streams a file through it, in `bufferSize` chunks - bounded
	//! by the buffer by construction. Nothing that the server SENDS is copied
	//! into it any more: an answer's length is the answer's business.
	char* buffer;

	/* Data storage variables */
	std::queue<ClientMessage> inputQueue; //Input queue
	std::queue<ClientMessage> outputQueue; //Output queue
	SDL_mutex *inputting; //Input queue mutex
	SDL_mutex *outputting; //Mutex of the output queue
	//! The request currently being served, latched by getInput: the answer
	//! goes back to this connection. Written and read by the application
	//! thread only (see getInput).
	unsigned int servingClient;
	unsigned int servingId;

	/* Initialization function and code */
	int init(unsigned int port, unsigned int maxClients, unsigned int bufferSize); //Initialization function called by the constructors
	int initErrorCode; //Initialization error code

	/* Processing functions */
	static int threadWrapper(void *Data); //Function that delegates run
	int run(); //Incoming data processing loop
	void checkNewClient(); //Function to check new clients
	void checkNewData(); //New data verification function
	void computeNewData(unsigned int client); //Data processing function
	bool computeString(unsigned int client, std::string string); //Chain processing function
	bool computeHttp(unsigned int client, std::string string);//HTTP request processing function (BETA)
	void computeNormalString(unsigned int client, std::string string);//Normal request processing function
	void pushRequest(unsigned int client, const std::string &data); //Queues a request with the connection it came from
	void checkDataToSend(); //Sending function of data received from the application
	void deliver(const ClientMessage &out); //Sends one answer where it belongs
	//! Broadcast to the feedback subscribers. `excludeClient` is the slot that
	//! has already been served as the addressee, so that a client which is both
	//! the issuer and a subscriber gets one copy and not two.
	int broadcast(const std::string &data, int excludeClient = -1);
	int close(unsigned int client); //Function to close the client socket

	/* FFactoring or assistance functions */
	//! Send ONE message to one client. The message is the caller's own, and
	//! its length is bounded by itself.
	//! INTENT §5.73: every sender used to `strcpy` its message into `buffer`
	//! first, and `buffer` is the RECEIVE buffer - `tcp_buffer_in_size` bytes,
	//! 1024 in the shipped config. An answer is clamped to `MAX_BUFFER` = 1024
	//! by `setOutput` and then sent as `data + '\n' + '\0'`, i.e. L + 2 bytes,
	//! so an answer of 1023 bytes or more wrote past the end of a buffer whose
	//! size answers a different question. Nothing here truncated anything, so
	//! nothing about what a client receives changes: the size rule is still
	//! `strlen + 1` (the terminator is part of the message - that is how a
	//! client frames it), only the storage is now the message's own.
	int send(TCPsocket client, const char *data);
	int resetThread(); //Fonction qui redémarre le thread de traiement quand il est inactif
	int killThread(); //Function that kills the processing thread when it is inactive

	std::string humanReadable(unsigned int); //Function that returns a formatted string (B, KB, MB, GB)
	std::string clientIp(unsigned int client); //Function that returns the IP address of the client as a string

	/* Debug functions */
	void debugOut(std::string msg, LOG_TYPE log);//Debug

	std::string replace(std::string base, const std::string from, const std::string to);
};



#endif // IO_H
