/*
 * writer_fifo.c
 * 
 * Copyright 2017 olivier <olivier@orion>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main()
{
    int fd;
    char* myfifo = "/tmp/spacecrafter.fifo";
	char* msg = "flag constellation_drawing toggle";

    //~ /* create the FIFO (named pipe) */
    //~ mkfifo(myfifo, 0666);
	printf("Msg : %s\n", msg);
	int msg_size = strlen(msg);

    /* write "Hi" to the FIFO */
    fd = open(myfifo, O_WRONLY);
    if (fd < 0) {
		printf("Impossible d'ouvrir le fichier\n");
        return -1;
    }

    int resol = write(fd, msg , msg_size);
	if (resol != msg_size)
		printf("Erreur d'écriture: voulu %i, sur %i\n", resol, msg_size);

    close(fd);

    /* remove the FIFO */
    //~ unlink(myfifo);

    return 0;
}
