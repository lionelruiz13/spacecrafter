#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>

#define NB_SEC_SOMMEIL		5		/* Nombre de secondes de sommeil du démon */

/**
* Initialise le démon.
*/
static void demoniser()
{
	pid_t pid; /* PID après le fork() */
        
    /* On quitte la fonction si le processus est déjà un démon */
    if(getppid() == 1) return;
        
	/* On fork le processus. */
	pid = fork();
	if(pid < 0)
	{
		syslog(LOG_ERR,"le 'fork()' a echoue, errno = %d (%s).",errno,strerror(errno));
		exit(EXIT_FAILURE);
	}

    /* On quitte le processus père. */
    if(pid > 0) exit(EXIT_SUCCESS);
        
	/* Le processus fils est adopté par le processus init et devient donc un démon.
	A ce stade on exécute le processus fils. */

    /* On donne tous les droits au démon sur les fichiers qu'il crée. */
    umask(0);
       
    /* On crée un nouveau SID pour le processus fils. */
    if(setsid() < 0)
	{
        syslog(LOG_ERR,"le 'setsid()' a echoue, errno = %d (%s).",errno,strerror(errno));
        exit(EXIT_FAILURE);
    }
        
    /* On change le répertoire de travail courant pour que le démon
	n'empèche pas le démontage du FS contenant le dossier à partir
	duquel il a été lancé. */
    if((chdir("/")) < 0)
	{
        syslog(LOG_ERR,"le 'chdir()' a echoue, errno = %d (%s).",errno,strerror(errno));
        exit(EXIT_FAILURE);
    }
        
    /* On redirige les entrées/sorties standards vers /dev/null. */
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);

	syslog(LOG_INFO,"le demon vient de naitre.");
}

/**
* Fonction principale.
*/
int main(void)
{	
    demoniser(); /* On initialise le démon. */
        
    /* Boucle infinie dans laquelle on trouve l'action du démon */
    for(;;)
	{
		sleep(NB_SEC_SOMMEIL);
		system("spacecrafter&");
		syslog(LOG_INFO,"commande spacecrafter executee.");
	}
	return 0;
}

