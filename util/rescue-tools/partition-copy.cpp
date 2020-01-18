/*
Programme de copie de partition psychologiquement agréable
Utilité : ce programme permet de copier des partitions de même taille et affiche une barre de défilement ainsi qu'une estimation du temps restant
Usage : à inclure dans un système
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour association-sirius.org
Mise à jour le 02/06/2016
*/

#include "partition-copy.hpp"

#define BAR_SYMBOL1 "_"
#define BAR_SYMBOL2 "#"
#define PROGRESS_MAX 10000
#define MARGIN 8
#define MARGINTOP 2
#define BAR_SIZE COLS-2-2*MARGIN
#define CONFIRM 'Y'

void PartitionCopy::error(int code) {
	line = MARGINTOP;
	erase();
	bkgd(COLOR_PAIR(1));
	mvprintw(line++, MARGIN, "Erreur");
	switch(code) {
		case EMPTY_LIST : mvprintw(line++, MARGIN, "Aucune partition correspondante trouvée !"); break;
		case WRONG_CHOICE :  mvprintw(line++, MARGIN, "Le choix saisis ne fait pas parti de la liste !"); break;
		case ABORT : mvprintw(line++, MARGIN, "L'opération a été annulée par l'utilisateur"); break;
		case INPUT_BLOCK_SIZE_NULL : mvprintw(line++, MARGIN, "La partition source est-elle un périphérique block ?"); break;
		case INPUT_SIZE_NULL : mvprintw(line++, MARGIN, "La partition source a une taille nulle"); break;
		case OUTPUT_BLOCK_SIZE_NULL : mvprintw(line++, MARGIN, "La partition de destiantion est-elle un périphérique block ?"); break;
		case OUTPUT_SIZE_NULL : mvprintw(line++, MARGIN, "La partition de destination a une taille nulle"); break;
		case DIFFERENT_SIZE : mvprintw(line++, MARGIN, "Les partitions sont de tailles différentes"); break;
		case DIFFERENT_UUID : mvprintw(line++, MARGIN, "Les UUIDs des partitions sont différents"); break;
		case READ_ERROR : mvprintw(line++, MARGIN, "Erreur de lecture !"); break;
		case WRITE_ERROR : mvprintw(line++, MARGIN, "Erreur d'écriture !"); break;
	}
	
	mvprintw(line++, MARGIN, "Tapez %c pour ignorer et continuer quand même", CONFIRM);
	refresh();
	
	char text;
	scanf("%c", &text);
	if(CONFIRM != text) {
		endwin();
		exit(code);
	}
	bkgd(COLOR_PAIR(3));
	erase();
}

void PartitionCopy::printBar(unsigned long progress) {
	
	unsigned long i, bar_position = ((progress%100)*100*(unsigned long)(BAR_SIZE))/(unsigned long)PROGRESS_MAX;
	for(i=0; i < bar_position; i++) mvprintw(line, MARGIN+1+i, BAR_SYMBOL1);
	line++;

	bar_position = (progress*(unsigned long)(BAR_SIZE))/(unsigned long)PROGRESS_MAX;
	mvprintw(line, MARGIN, "[");
	for(i=0; i < bar_position; i++) mvprintw(line, MARGIN+1+i, BAR_SYMBOL2);
	mvprintw(line++, COLS-MARGIN-1, "]");
}

void PartitionCopy::printPercent(unsigned long progress) {
	mvprintw(line++, MARGIN, "Progression : %lu.%02lu%%", progress/(PROGRESS_MAX/100), progress%(PROGRESS_MAX/100));
}

const char* PartitionCopy::humanReadable(unsigned long bytes) {
		if(bytes < 1000) sprintf(out, "%lu", bytes);
		else if(bytes < 1000000) sprintf(out, "%luK", bytes/1000);
		else if(bytes < 1000000000) sprintf(out, "%luM", bytes/1000000);
		else sprintf(out, "%luG", bytes/1000000000);
		return out;
}

void PartitionCopy::printSpeed(unsigned long bytes, unsigned long time) {
	if(time) {
		unsigned long speed =  bytes/time;
		mvprintw(line++, MARGIN, "Vitesse : %so/s", humanReadable(speed));
	}
}

void PartitionCopy::printRemainingTime(unsigned long progress, unsigned long elapsed) {
	if(elapsed && progress >= elapsed) {
		unsigned long remain = (((PROGRESS_MAX - progress)/(progress/elapsed))/60);
		if(!remain) mvprintw(line++, MARGIN, "Moins d'une minute restante");
		else if (remain == 1) mvprintw(line++, MARGIN, "Environ 1 minute restante", remain);
		else if (remain < 60) mvprintw(line++, MARGIN, "Environ %lu minutes restantes", remain);
		else if (remain < 60 * 10) {
			time_t now;
			time(&now);
			now += remain*60;
			strftime(out, 6,"%H:%M", localtime(&now));
			mvprintw(line++, MARGIN, "Environ %lu heures et %lu minutes restantes (%s)", remain/60, remain%60, out);
		}
		else mvprintw(line++, MARGIN, "(x__x)	(>10 heures)");
	}
}

const char* PartitionCopy::getPartitionInfo(const char* partition, const char* info) {
	blkid_probe pr = blkid_new_probe_from_filename(partition);
	blkid_do_probe(pr);
	const char *value = NULL;
	blkid_probe_lookup_value(pr, info, &value, NULL);
	blkid_free_probe(pr);
	return value;
}

const char* PartitionCopy::getPartitionInfos(const char* partition) {
	blkid_probe pr = blkid_new_probe_from_filename(partition);
	blkid_do_probe(pr);
	const char *label = NULL;
	const char *type = NULL;
	const char *uuid = NULL;
	blkid_probe_lookup_value(pr, "LABEL", &label, NULL);
	blkid_probe_lookup_value(pr, "TYPE", &type, NULL);
	blkid_probe_lookup_value(pr, "UUID", &uuid, NULL);
	blkid_free_probe(pr);
	sprintf(out, "%s\t%s\t%s", label, type, uuid);
	return out;

}

long unsigned int PartitionCopy::getPartitionSize(const char* partition) {

	long unsigned int size;
	int part = open(partition, O_RDONLY);
	ioctl(part, BLKGETSIZE , &size);
	close(part);
	return size;
	
}

void PartitionCopy::fillPartitionList() {

	while(!partitions.empty()) partitions.erase(partitions.end()); 

	FILE* procpartitions;
	procpartitions = fopen("/proc/partitions", "r");
	//if(procpartitions == NULL) printf("FAIL\n");
	char buffer[64];
	int linesize;
	int c;


	if(fgets(buffer, 60, procpartitions) != NULL) {
		while(fgets(buffer, 60, procpartitions) != NULL) {
			linesize = strlen(buffer);
			if(buffer[linesize-2] >= '0' && buffer[linesize-2] <= '9' ) {
				for(c=0; c < linesize; c++) {
					if(buffer[c] >= 'a' && buffer[c] <= 'z') {
						if(buffer[linesize-1] == '\n') buffer[linesize-1] = '\0';
						partitions.push_back(buffer+c);
						break;
					}
				}
			}
		}
	}

	fclose (procpartitions);

}

int PartitionCopy::filterPartitionList(unsigned long minBlockCount) {

	long unsigned int size;
	for(unsigned int j=0; j < partitions.size(); j++) {
//		printf("%s ", partitions[j].c_str());//DEBUG TODO
		size = getPartitionSize(("/dev/"+partitions[j]).c_str());
//		printf("%lu %lu %d\n", size*512, size, 512);//DEBUG TODO
		if(size < minBlockCount) {
			partitions.erase(partitions.begin()+j);
			j--;
		}
	}
	return partitions.size();

}

int PartitionCopy::filterSizePartitionList(unsigned int size) {

	for(unsigned int j=0; j < partitions.size(); j++) {
		if(getPartitionSize(("/dev/"+partitions[j]).c_str()) != size) {
			partitions.erase(partitions.begin()+j);
			j--;
		}
	}
	return partitions.size();

}


void PartitionCopy::filterTypePartitionList(const char* type) {

	const char* partType;
	for(unsigned int p=0; p < partitions.size(); p++) {
		partType = getPartitionInfo(("/dev/"+partitions[p]).c_str(), "TYPE");
		if(partType != NULL && !strcmp(partType , type)) {
			partitions.erase(partitions.begin()+p);
			p--;
		}
	}

}

void PartitionCopy::filterNamePartitionList(const char* name) {
//	printf("%s\n", name);
	for(unsigned int p=0; p < partitions.size(); p++) {
		if(!strcmp(name , partitions[p].c_str())) {
			partitions.erase(partitions.begin()+p);
			p--;
		}
	}

}

int PartitionCopy::printPartitonList() {
	if(partitions.empty()) error(EMPTY_LIST);
	mvprintw(line++, MARGIN,"*   Partition\tTaille\tLabel\tType\tUUID");
	int num = 'a';
	char partitioninfos[128];
	for(unsigned int j=0; j < partitions.size(); j++) {
		strcpy(partitioninfos ,getPartitionInfos(("/dev/"+partitions[j]).c_str()));
		mvprintw(line++, MARGIN,"%c : %s\t%so\t%s\n", num++, partitions[j].c_str(), humanReadable(getPartitionSize(("/dev/"+partitions[j]).c_str())*512), partitioninfos);
	}
	return num--;
}

const char* PartitionCopy::chosePartition() {

	int num = printPartitonList();
	refresh();
	char inputnum;
	scanf("%c", &inputnum);
	if(inputnum > num || inputnum < 'a') error(WRONG_CHOICE);
	const char* choosen = partitions[inputnum - 'a'].c_str();
	return choosen;

}

void PartitionCopy::initNcurses() {

	initscr();			/* Start curses mode 		*/
	noecho();			/* Don't echo() while we do getch */
	
	if(has_colors() == FALSE) {
//	fprintf(stderr, "Le terminal ne supporte pas les couleurs.\n");
	exit(EXIT_FAILURE);
	}

	/* Activation des couleurs */
	start_color();

	/* Definition de la palette */
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_WHITE, COLOR_BLUE);
	
	
	/* Background Color */
	bkgd( COLOR_PAIR(3) );
	
	line=MARGINTOP;

}

void PartitionCopy::renamePartition(char* partition, char* type, char* label) {
	erase();
	line = MARGINTOP;
	mvprintw(line, MARGIN, "Mise à jour du nom de la partition");
	endwin();
	unsigned int pid = fork();
	if(!pid) {
		if(!strcmp("ext4", type) || !strcmp("ext3", type) || !strcmp("ext2", type)) execl("/sbin/e2label", "e2label", partition, label, (char *)NULL);
		else if(!strcmp("ntfs", type)) execl("/sbin/ntfslabel", "ntfslabel", partition, label, (char *)NULL);
		else if(!strcmp("ntfs", type)) execl("/sbin/fatlabel", "fatlabel", partition, label, (char *)NULL);
		exit(0);
	}
	initNcurses();
	refresh();
	line = MARGINTOP;

}

int PartitionCopy::run(int argc, char **argv) {

	/* Initialisation */

	if(getuid() > 0) {
		printf("Ce programme requière des privilèges administateurs (essayez avec sudo %s)\n", argv[0]);
		exit(0);
	}

	if(argc < 2) {
		printf("Usage : %s MinBlockCount [ingoredType1] [ignoredType2] ...\n", argv[0]);
		exit(0);
	}

	out = (char*)malloc(128*sizeof(char));

	fillPartitionList();
	unsigned long minBlockCount = atol(argv[1]);
	if(!minBlockCount) exit(0);
	filterPartitionList(minBlockCount);
	for(int i=2; i < argc; i++) filterTypePartitionList(argv[i]);

	//for(int j=0; j < partitions.size(); j++) printf("%s\n", partitions[j].c_str());

	//	for(int p=0; p < partitions.size(); p++) {
	//	
	//		// Get UUID, label and type
	//		const char *uuid = NULL;
	//		const char *label = NULL;
	//		const char *type = NULL;

	//		uuid = getPartitionInfo(("/dev/"+partitions[p]).c_str(), "UUID");
	//		label = getPartitionInfo(("/dev/"+partitions[p]).c_str(), "LABEL");
	//		type = getPartitionInfo(("/dev/"+partitions[p]).c_str(), "TYPE");
	//		
	//		printf("Name=%s", partitions[p].c_str());
	//		if(uuid != NULL) printf(", UUID=%s", uuid);
	//		if(label != NULL) printf(", LABEL=%s", label);
	//		if(type != NULL) printf(", TYPE=--%s--", type);
	//		printf("\n");
	//	}

	//for(int j=0; j < partitions.size(); j++) printf("%s\n", partitions[j].c_str());

	initNcurses();

	/* Paramètrage */

	mvprintw(line++, MARGIN, "Quelle partition voulez-vous écraser ?");
	char choosen[32];
	strcpy(choosen, chosePartition());
	char outputdev[64];
	strcpy(outputdev, ("/dev/" + (std::string)choosen).c_str());

	fillPartitionList();
	filterNamePartitionList(choosen);
	filterSizePartitionList(getPartitionSize(outputdev));

	erase();
	line = MARGINTOP;
	mvprintw(line++, MARGIN, "Quelle partition voulez vous utiliser comme source ?");
	char inputdev[64];
	strcpy(inputdev, ("/dev/" + (std::string)chosePartition()).c_str());


	/* Vérification */

	unsigned long inputblocks = getPartitionSize(inputdev);
	unsigned long outputblocks = getPartitionSize(outputdev);

	if(inputblocks != outputblocks) error(DIFFERENT_SIZE);

	char inputuuid[128];
	char outputuuid[128];
	strcpy(inputuuid, getPartitionInfo(inputdev, "UUID"));
	strcpy(outputuuid, getPartitionInfo(outputdev, "UUID"));

	if(strcmp(inputuuid, outputuuid)) error(DIFFERENT_UUID);

	erase();
	line = MARGINTOP;
	mvprintw(line++, MARGIN, "Attention, cette opération va écraser la partition %s (%lu octets) par la partition %s !", outputdev, inputblocks * 512, inputdev);
	mvprintw(line++, MARGIN, "Entrez '%c' pour confirmer : ", CONFIRM);
	refresh();
	char text;
	scanf("%c", &text);
	if(CONFIRM != text) error(ABORT);
	erase();


	/* Copie */

	char inputtype[32];
	strcpy(inputtype, getPartitionInfo(inputdev, "TYPE"));

	FILE* input = fopen(inputdev, "r");
	FILE* output = fopen(outputdev, "w");

	char* buffer = (char*)malloc(512);

	unsigned long i, progress;
	time_t starttime, now, lasttime = 0;
	unsigned long elapsed;
	unsigned int read;

	time(&starttime);
	for(i=1; i <= inputblocks; i++) {
		if(!(read = fread(buffer, 1, 512, input)))  error(READ_ERROR);;
		if(fwrite(buffer, 1, read, output) != read) error(WRITE_ERROR);
	
		if(i == 8) {
			renamePartition(outputdev, inputtype, (char *)"CORRUPTED");
		}
	
		time(&now);
		if(difftime(now, lasttime) || i == inputblocks) {
			progress = (i * PROGRESS_MAX) / inputblocks;
			elapsed = (unsigned long)difftime(now,starttime);
			erase();
			line = MARGINTOP;
			printBar(progress);
			printPercent(progress);
			printSpeed(i*512, elapsed);
			printRemainingTime(progress, elapsed);
			mvprintw(line++, MARGIN, "(%lu/%lu)", i, inputblocks);
			refresh();
			lasttime = now;
		}
	}

	fclose(input);

	mvprintw(line, MARGIN,"Synchronisation");
	refresh();
	sync();
	fclose(output);
	time(&now);
	const char* inputlabel = getPartitionInfo(inputdev, "LABEL");
	unsigned int inputlabelsize;
	if(inputlabel == NULL)	inputlabelsize = 0;
	else {
		inputlabelsize = strlen(inputlabel);
		strcpy(out, inputlabel);
	}
	if(inputlabelsize < 4) memset(out+inputlabelsize, '_', 4-inputlabelsize);
	strftime(out+4, 13,"-%y%m%d-%H%M", localtime(&now));
	renamePartition(outputdev, inputtype, out);
	sync();
	bkgd( COLOR_PAIR(2) );
	erase();
	mvprintw(line, MARGIN,"Copie terminée avec succès");
	refresh();


	sleep(2);
	endwin();			/* End curses mode		  */

	return 0;

}
