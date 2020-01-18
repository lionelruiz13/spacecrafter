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
#define PORT 7805
//~ #define IPADRESS "192.168.0.12"
#define IPADRESS "127.0.0.1"

int main(int argc, char *argv[])
{
    int sockfd = 0;
    char recvBuff[SIZEBUFFER];
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

	//première connexion, pour reception, on demande au serveur de nous placer en mode reception
	char login[]="$LOGON";
	
	unsigned int nb_write;
	unsigned int nb_read;
	
	nb_write = write(sockfd, login, strlen(login)+1);
	
	if (nb_write == strlen(login)+1)
		printf("-> %s : %li\n",login, strlen(login));
	else
		printf("Error connection, serverlost ?\n");	
	
	//affichage des commandes émises par spacecrafter
	int isAlive =1;
	while(isAlive) {
		nb_read = read(sockfd, recvBuff, SIZEBUFFER); 
		if (nb_read>0) {
			printf("<- %s : %li\n",recvBuff, strlen(recvBuff));
			memset(recvBuff, '\0', SIZEBUFFER);
		} else
			isAlive = 0;
	}

    close(sockfd);    

    return 0;
}
