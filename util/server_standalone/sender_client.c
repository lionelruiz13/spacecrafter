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

#define SIZEBUFFER 256
#define PORT 7805
//~ #define IPADRESS "192.168.0.12"
#define IPADRESS "127.0.0.1"


int main(int argc, char *argv[])
{
	//creation de la socket
    int sockfd = 0;
    char sendBuff[SIZEBUFFER];
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

    //Connect to remote server
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }
    
    sleep(2);
    
    //preparation buffer avant envoi
    memset(sendBuff, '\0', SIZEBUFFER); 
    //~ strcpy(sendBuff,"audio action play filename monde_perdu.ogg\n");
    //~ strcpy(sendBuff,"script action play filename fscripts/06.sts");
    //~ strcpy(sendBuff,"script action play filename fscripts/K6.sts");
    //~ strcpy(sendBuff,"set sky_culture western");
    //~ strcpy(sendBuff,"set milky_way_texture default");
    //~ strcpy(sendBuff,"set milky_way_intensity 0.9");
    //~ strcpy(sendBuff,"illuminate display on HP 4906");
    //~ strcpy(sendBuff,"dso hidden false  name M27");
    //~ strcpy(sendBuff,"flag track_object on");
    //~ strcpy(sendBuff,"planet_scale name Saturn scale 20");
    //~ strcpy(sendBuff,"flag planets_axis on");
    //~ strcpy(sendBuff,"clear state natural");
    //~ strcpy(sendBuff,"audio volume 100");
    //~ strcpy(sendBuff,"audio action drop\n");
    //~ strcpy(sendBuff,"media action play type VIEWPORT videoname /home/olivier/.spacecrafter/videos/monde_perdu_HD.mp4 audioname /home/olivier/.spacecrafter/videos/monde_perdu.ogg\n");
    //~ strcpy(sendBuff,"media action play type VR360 videoname OrionNebula360.mp4 audioname OrionNebula360_fr.ogg\n");
    //~ strcpy(sendBuff,"media action play type VIEWPORT videoname trou_noir.mp4 audioname auto");
    //~ strcpy(sendBuff,"media action play type VR360 videoname F1_ExperienceVR360.mp4");
    //~ strcpy(sendBuff,"media action play type VIEWPORT videoname hugh1.mp4 audioname hugh_fr.ogg\n");
    //~ strcpy(sendBuff,"body name Earth skin_use toggle");
    //~ strcpy(sendBuff,"body name Earth skin_tex /home/olivier/.spacecrafter/textures/bodies/earth_skin_rg.png");
    //~ strcpy(sendBuff,"media action stop\n");
    //~ strcpy(sendBuff,"flag atmosphere toggle\n");
    //~ strcpy(sendBuff,"planet_scale name Saturn scale 1\n");
    //~ strcpy(sendBuff,"set ambient_light 0.10");
    //~ strcpy(sendBuff,"color property constellation_lines value x00ff80\n");
    //~ strcpy(sendBuff,"constellation name UMA type line_color color x00ffff\n");
    //~ strcpy(sendBuff,"constellation name UMA type label_color r 1.0 b 0.0 g 0.0\n");
    //~ strcpy(sendBuff,"constellation name UMA intensity 0.8\n");
    //~ strcpy(sendBuff,"set constellation_art_intensity 0.2\n");
    //~ strcpy(sendBuff,"text action load name test string \"!! Vive Spacecrafter !!\" azimuth 10 altitude 30 size X_LARGE r 0.2 g 0.4 b 0.4 display on");
    //~ strcpy(sendBuff,"text action update name test string Coucou");
    //~ strcpy(sendBuff,"text action load name test2 string \"!! Vive Spacecrafter !!\" azimuth 90 altitude 30 size LARGE r 0.2 g 0.4 b 0.4");
    //~ strcpy(sendBuff,"text name test2 display on");
    //~ strcpy(sendBuff,"text action load name test1 string \"!! Vive Spacecrafter TMP!!\" azimuth 180 altitude 30 r 0.2 g 0.4 b 0.4 size X_LARGE duration 12 display on");
    //~ strcpy(sendBuff,"text action load name test1 display on");
    //~ strcpy(sendBuff,"text name test display on");
    //~ strcpy(sendBuff,"set star_scale 0.9\n");
    //~ strcpy(sendBuff,"color property constellation_art r 0.7 g 0.7 b 1.0\n");
    //~ strcpy(sendBuff,"meteors zhr 10000000\n"); 
    //~ strcpy(sendBuff,"clear screenfader true"); 
    //~ strcpy(sendBuff,"image action purge"); 
    //~ strcpy(sendBuff,"flag stars toggle"); 
    //~ strcpy(sendBuff,"flag planets_axis toggle"); 
    //~ strcpy(sendBuff,"camera action switch name Mars"); 
    //~ strcpy(sendBuff,"select planet Mercury");
    //~ strcpy(sendBuff,"select planet home_planet");
    //~ strcpy(sendBuff,"set screen_fader 0");
    //~ strcpy(sendBuff,"zoom auto in zoom fov 1 duration 10"); 
    //~ strcpy(sendBuff,"flag stars_trace toggle"); 
    //~ strcpy(sendBuff,"body name Uranus color label color_value r12g215b215"); 
    //~ strcpy(sendBuff,"body mode in_universe action remove name test1"); 
    //~ strcpy(sendBuff,"body name Pluto hidden toggle"); 
    //~ strcpy(sendBuff,"set time_display_format 12h"); 
    //~ strcpy(sendBuff,"set date_display_format yyyymmdd"); 
    //~ strcpy(sendBuff,"body_trace pen true target Sun"); 
    //~ strcpy(sendBuff,"body mode in_universe action removeAll"); 
    //~ strcpy(sendBuff,"body mode in_universe action load name test2 filename lro.ojm pos_x 0.2 pos_y 0.4 pos_z 0.0 scale 1.0");
    //~ strcpy(sendBuff,"set mode InSolarSystem"); 
    //~ strcpy(sendBuff,"set mode InGalaxy"); 
    //~ strcpy(sendBuff,"set mode InUniverse"); 
    //~ strcpy(sendBuff,"set tully_color_mode white"); 
    //~ strcpy(sendBuff,"camera action transition_to target point name Space"); 
    //~ strcpy(sendBuff,"timerate rate 360"); 
    //~ strcpy(sendBuff,"constellation name UMa type line color xff0000"); 
    //~ strcpy(sendBuff,"constellation name UMa type label color xff0000"); 
    //~ strcpy(sendBuff,"constellation name UMa  intensity 1.0");
    //~ strcpy(sendBuff,"set date_display_number 1");
    //~ strcpy(sendBuff,"body tesselation moon_altimetry_factor value 5");
    strcpy(sendBuff,"body tesselation max_tes_level value 4");
    //~ strcpy(sendBuff,"timerate action decrement"); 
    
	
	unsigned int nb_write;
	
	nb_write = write(sockfd, sendBuff, strlen(sendBuff));
	
	if (nb_write == strlen(sendBuff))
		printf("-> %s : %li\n",sendBuff, strlen(sendBuff));
	else
		printf("Error connection, serverlost ?\n");
	
    close(sockfd);    

    return 0;
}
