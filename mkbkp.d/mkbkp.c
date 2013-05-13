#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int fflag = 0;
int xflag = 0;
int cflag = 0;
int tflag = 0;
char *fvalue = NULL;
char *tvalue = NULL;
FILE *logger;

int checkInputs();
void work(int, int, int, char**);
void createBackup(char*, char*, char*);
int openLog();
char* getTime();
void print(FILE *);
void extractBkp(FILE *);
void recMkdir(const char *);

main (int argc, char **argv) {
    int index;
	int c;
	int error = -1;
	int i;
	
	openLog();
	fprintf(logger, "(II) impostazioni inserite : "); 
	for (i = 0; i < argc; i += 1) {
		fprintf(logger, "%s ", argv[i]);
	}
	fprintf(logger, "\n");

	opterr = 0;

	while ((c = getopt (argc, argv, "f:xct:")) != -1)
		switch (c) {
			case 'f':
		 		fflag = 1;
		 		fvalue = optarg;
		 		break;
	   		case 'x':
		 		xflag = 1;
		 		break;
	   		case 'c':
		 		cflag = 1;
		 		break;
		 	case 't':
		 		tflag = 1;
		 		tvalue = optarg;
		 		break;
	   		default:
	   			fprintf(logger, "(EE) opzione non riconosciuta\n");
		 		abort ();
	   }
	   
	error = checkInputs();
	
	work(error, optind, argc, argv);

	for (index = optind; index < argc; index++)
		//printf ("Non-option argument %s\n", argv[index]);
	return 0;
}

/**
*   0 -> tflag corretto -> lettura contenuto bkp
*   1 -> xflag corretto -> estrazione backup
*   2 -> cflag corretto -> Creazione archivio
*	1* -> errore nel tflag
*	2* -> errore nel xflag
*	3* -> errore nel cflag
* 	40 -> target senza azioni da eseguire
*	50 -> don't know what to do -.-"
*/
int checkInputs() {
	if (tflag == 1) {
		if (fflag == 1 || xflag == 1 || cflag == 1) {
			return 10;
		} else if (tvalue == NULL) {
			return 11;
		} else {
			fprintf(logger, "(II) Lettura archivio\n");
			return 0;
		}
	}
	if (xflag == 1) {
		if (fflag == 0) {
			return 21;
		} else if (fvalue ==NULL) {
			return 22;
		} else if (cflag == 1) {
			return 23;
		} else {
			return 1;
		}
	}
	if (cflag == 1) {
		if (fflag == 0) {
			return 31;
		} else if (fvalue ==NULL) {
			return 32;
		} else {
			return 2;
		}
	}
	if (fflag == 1) {
		return 40;
	} else {
		return 50;
	}
}

char* getTime() {
	time_t rawtime;
  	struct tm * timeinfo;	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return asctime(timeinfo);
}

int openLog() {
	int retval;
    if (logger = fopen("./mkbkp", "r")) {
        fclose(logger);
        retval = 1;
    } else
    	retval = 0;
    logger = fopen("./mkbkp", "a");
    if (retval == 0) {
    	fprintf(logger, "Markers: (!!) notice, (II) informational, (WW) warning, (EE) error\n");
    	fprintf(logger, "(!!) Creazione log %s", getTime());
    }
   	fprintf(logger, "(II) Nuova esecuzione del programma al tempo: %s", getTime()); 
    return retval;
}

void work(int code, int optindex, int argc, char** targets) {
	int i=0;
	FILE *file;
	if (code >= 10) {
		abort();
	} else if (code == 0) {
		if(file = fopen(tvalue, "r")) {
			//logger info eseguo lettura
			print(file);
		} else {
			fprintf(logger, "(EE) File non trovato alla posizione %s\n", tvalue);
			printf("file non trovato\n");
		}
	} else if (code == 1) {
		if (file = fopen(fvalue, "r")) {
			extractBkp(file);
		} else {
			fprintf(logger, "(EE) File di backup non trovato alla posizione %s\n", tvalue);
			printf("file di backup non trovato\n");
		}
	} else if (code == 2) {
		if(file = fopen(fvalue, "r")) {
			fclose(file);
			remove(fvalue);
		}
		for(i=optindex; i<argc; i++) {
			createBackup("./", fvalue, targets[i]);
		}
	}
}

void createBackup(char* path, char* bkpPath, char* target) {
	printf("%s\n" , target);
	struct dirent *drnt;
 	struct stat s;
	DIR *dr;
	char* tmp;
	FILE *ofile;
	FILE *ifile;
	
	tmp = malloc(snprintf(NULL, 0, "%s",target) + 1);
	sprintf(tmp, "%s",target);	
		
	if( stat(tmp,&s) == 0 ) {
	    if( s.st_mode & S_IFREG ) {
	    	char* tm = malloc(snprintf(NULL, 0, "%s\n",tmp) + 1);
			sprintf(tm, "%s\n",tmp);
	        printf("sono uno bellissimo fil %s\n", tmp);
	        char ch;
	        ofile = fopen(bkpPath, "a");
	        ifile = fopen(tmp, "r");
	        fputs(tm, ofile);
	        while(1) {
			    ch = getc(ifile);
			    putc(ch,ofile);
			    if(ch==EOF)
			    	break;
	    	}
		} else if ( s.st_mode & S_IFDIR ) {
				
			if(target[strlen(target)-1]!='/') {
				sprintf(target, "%s%s",target ,"/");
			}
			printf("sono una bellissima dir %s\n", target);
		
			dr=opendir(target);
			while(drnt=readdir(dr)) {
				sprintf(tmp, "%s%s",target ,drnt->d_name);
				if (strcmp(".", drnt->d_name)==0 || strcmp("..", drnt->d_name)==0)
					continue;
			    createBackup(path, bkpPath, tmp);
			}
			closedir(dr);
		}
	}
}

void print(FILE *f) {
	char * path = malloc(snprintf(NULL, 0, "%s","") + 1);
	char ch;
	while(fscanf(f, "%s", path) != EOF) {
		printf("%s\n", path);
		while(1) {
			ch = getc(f);
			if(ch==EOF)
				break;
		}
	}
}

void extractBkp(FILE * bkp) {
	char * path = malloc(snprintf(NULL, 0, "%s","") + 1);
	char * basepath = malloc(snprintf(NULL, 0, "%s","") + 1);
	char * tmp = malloc(snprintf(NULL, 0, "%s","") + 1);
	char ch;
	int i;
	sprintf(basepath, "%s", fvalue);
	FILE * out;
	if(basepath[strlen(basepath)-1]!='/') {
		sprintf(basepath, "%s%s",basepath ,"/");
	}
	while(fscanf(bkp, "%s", path) != EOF) {
		printf("%s %s\n", basepath, path);
		sprintf(tmp, "%s%s%s","./", basepath, path);
		
		i = strlen(tmp)-1;
		do {
			i--;
		} while(tmp[i] !='/');
		tmp[i+1] = '\0';
		printf("tua madre e' %s\n", tmp);
		recMkdir(tmp);
		
		out = fopen(path, "a+");
		while(1) {
			ch = getc(bkp);
			//putc(ch, out);
			if(ch==EOF)
				break;
		}
	}
}

void recMkdir(const char *dir){
	int i = 0;
	int j = 0;
	char * tmp;
	strcpy(tmp, dir);
	if (tmp[i]=='.' || tmp[i]=='/') {
		strncpy(tmp,tmp,)
		printf("%s\n", tmp);
	}
}

char * shiftString(char * source) {
	int i;
	int len = strlen(source);
	char * retval = malloc();
	for (i = 0; i++, i<len-1) {

	}
}