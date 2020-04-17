#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define SIZEBUFFER 128
#define SIZEBUFFERMAX 1024
#define PORT 7805
//~ #define IPADRESS "192.168.0.12"
#define IPADRESS "127.0.0.1"

int main(int argc, char *argv[])
{
    int sockfd = 0;
    char recvBuff[SIZEBUFFERMAX];

    struct sockaddr_in serv_addr; 

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); 

    if(inet_pton(AF_INET, IPADRESS , &serv_addr.sin_addr)<=0) 
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 


	unsigned int nb_write;
	unsigned int nb_read;
	
	
	//première connexion, pour reception, on demande au serveur de nous placer en mode reception
	char login[]="$LOGON\n";
	
	nb_write = write(sockfd, login, strlen(login)+1);
	if (nb_write == strlen(login)+1)
		printf("-> %s : %li\n",login, strlen(login)+1);
	else
		printf("Error connection, serverlost ?\n");		
	
	//reception confirmation de lecture de $LOGON
	nb_read = read(sockfd, recvBuff, SIZEBUFFERMAX); 
		if (nb_read>0) {
			printf("-> %s : %li\n",recvBuff, strlen(recvBuff));
			memset(recvBuff, '\0', SIZEBUFFERMAX);
		}
	

	//demande du status des constellations
	if (1) {
		char msg[]="get status constellation";
		nb_write = write(sockfd, msg, strlen(msg)+1);
		if (nb_write == strlen(msg)+1)
			printf("-> %s : %li\n",msg, strlen(msg)+1);
		else
			printf("Error connection, serverlost ?\n");		
	
		nb_read = read(sockfd, recvBuff, SIZEBUFFERMAX); 
			if (nb_read>0) {
				printf("<- %s : %li\n",recvBuff, strlen(recvBuff));
				memset(recvBuff, '\0', SIZEBUFFERMAX);
			}
	}


	//demande du status de la position
	if (1) {
		char msg[]="get status position";
		nb_write = write(sockfd, msg, strlen(msg)+1);
		if (nb_write == strlen(msg)+1)
			printf("-> %s : %li\n",msg, strlen(msg)+1);
		else
			printf("Error connection, serverlost ?\n");		
	
		nb_read = read(sockfd, recvBuff, SIZEBUFFERMAX); 
			if (nb_read>0) {
				printf("<- %s : %li\n",recvBuff, strlen(recvBuff));
				memset(recvBuff, '\0', SIZEBUFFERMAX);
			}
	}
	
	//demande du status des constellations
	if (1) {
		char msg[]="search name m1";// maxObject 3";
		nb_write = write(sockfd, msg, strlen(msg)+1);
		if (nb_write == strlen(msg)+1)
			printf("-> %s : %li\n",msg, strlen(msg)+1);
		else
			printf("Error connection, serverlost ?\n");		
	
		nb_read = read(sockfd, recvBuff, SIZEBUFFERMAX); 
			if (nb_read>0) {
				printf("<- %s : %li\n",recvBuff, strlen(recvBuff));
				memset(recvBuff, '\0', SIZEBUFFERMAX);
			}
	}

    close(sockfd);    

    return 0;
}
