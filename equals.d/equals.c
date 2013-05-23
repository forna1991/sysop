#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>

FILE *logger;

int openLog();
char* getTime();
int equals(char *, char *);

int main(int argc, char **argv) {
	openLog();

	// stampa il comando inserito nel file di log
	fprintf(logger, "(II) Comando inserito : "); 
	int i;
	for (i = 0; i < argc; i += 1) {
		fprintf(logger, "%s ", argv[i]);
	}
	fprintf(logger, "\n");
}

int equals(char * patha, char * pathb) {
	//controllo sul pathing
	//se e' una cartella rilanciare su tutti gli elementi
	//se e' un file vedere se nell'altro path c'e' un file e nel caso confrontarli tra loro
	/*int retval = 1; //TRUE
	char * tmpa, tmpb;
	struct stat a,b;
	tmpa = malloc(256 * sizeof(char));
	sprintf(tmpa, "%s",patha);
	tmpb = malloc(256 * sizeof(char));
	sprintf(tmpb, "%s",pathb);

	if( stat(tmpa,&a) == 0 && stat(tmpb,&b)) {	
		if( a.st_mode & S_IFREG ) {
			if( s.st_mode & S_IFREG ) {
				//scorrere tutti e due i file e se differiscono stampare i path
				//e scrivere che sono differenti
			} else if( s.st_mode & S_IFDIR ) {
				//stampare che uno dei due e' una cartella
			}	
		} else if( a.st_mode & S_IFDIR ) {
			if( s.st_mode & S_IFREG ) {
				//stampare che uno dei due file e' una cartella
			} else if( s.st_mode & S_IFDIR ) {
				//lanciare ricorsivamente su tutti i sottofile delle due cartelle
			}
		}
	}*/
}

//funzione che ritorna la data sottoforma di stringa
char* getTime() {
	time_t rawtime;
  	struct tm * timeinfo;	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return asctime(timeinfo);
}

/*Funzione che serve per apre il file di log*/
int openLog() {
	int retval;

	// controlla se c'è già un file di log creato
    if (logger = fopen("./LogEquals", "r")) {
        fclose(logger);
        retval = 1;
    } else
    	retval = 0;
    logger = fopen("./LogEquals", "a");

    // se non esiste lo crea e ci scrive 2 righe di informazione
    if (retval == 0) {
    	fprintf(logger, "Markers: (!!) notice, (II) informational, (WW) warning, (EE) error\n");
    	fprintf(logger, "(!!) Creazione log %s", getTime());
    }
   	fprintf(logger, "(II) Nuova esecuzione del programma al tempo: %s", getTime()); 
    return retval;
}