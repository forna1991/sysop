/* 
 * File:   mkbkp.c
 * anno accademico 2012/2013
 * Progetto #1
 * Giulio Fornasaro     151991
 * Francesco Boschini   151840
 * Luca Zamboni         151759
 */

#include "mkbkp.h"

int fflag = 0;
int xflag = 0;
int cflag = 0;
int tflag = 0;
char *fvalue = NULL;
char *tvalue = NULL;
FILE *logger;

main(int argc, char **argv) {
    int c;
    int i;
    char string[512] = "(II) impostazioni inserite : ";

    openLog(); //apertura del file di log
    //output del comando inserito dall'utente nel log
    for (i = 0; i < argc; i += 1) {
        strcat(string, " ");
        strcat(string, argv[i]);
    }
    strcat(string, "\n");
    fprintf(logger, "%s", string);

    //la getopt si occupa di riempire i campi per le opzioni accettate
    while ((c = getopt(argc, argv, "f:xct:")) != -1) {
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
                abort();
        }
    }

    work(optind, argc, argv); //in base alle opzioni decide cosa fare

    return 0;
}

/**
 *  restituisce un codice in base alla correttezza degli input:
 * 
 *  0 -> tflag corretto -> lettura contenuto bkp
 *  1 -> xflag corretto -> estrazione backup
 *  2 -> cflag corretto -> Creazione archivio
 *	1* -> errore nel tflag
 *	2* -> errore nel xflag
 *	3* -> errore nel cflag
 * 	40 -> target senza azioni da eseguire
 *	50 -> don't know what to do
 */
int checkInputs() {
    if (tflag == 1) {
        if (fflag == 1 || xflag == 1 || cflag == 1) {
            //tflag viene usato da solo, se ci sono altri input torna 10
            printf("l\'opzione -t non puo\' essere usata insieme ad altre opzioni!\n");
            fprintf(logger, "(EE) -t usato insieme ad altre opzioni!\n");
            return 10;
        } else if (tvalue == NULL) {
            //se non viene immesso niente come parametro del tvalue ritorna 11
            printf("l\'opzione -t non ha un obiettivo!\n");
            fprintf(logger, "(EE) -t usato senza obiettivo!\n");
            return 11;
        } else {
            fprintf(logger, "(II) Lettura archivio\n");
            return 0;
        }
    }
    if (xflag == 1) {
        if (fflag == 0) {
            //-x prevede che gli venga passato un target attraverso -f
            printf("-x non puo\' venire passato senza -f\n");
            fprintf(logger, "(EE) -x inserito senza -f\n");
            return 21;
        } else if (fvalue == NULL) {
            //il valore del target non puo' essere null
            printf("il target passato con -f non e\' valido!\n");
            fprintf(logger, "(EE) -f ha come target NULL!\n");
            return 22;
        } else if (cflag == 1) {
            //impossibile decomprimere (-x) ed estrarre (-c) insieme
            printf("-x e -c sono incompatibili tra loro, il programma puo\'"
                    "estrarre o comprimere ma non entrambe le cose!\n");
            fprintf(logger, "(EE) -x inserito senza -f\n");
            return 23;
        } else {
            fprintf(logger, "(II) Estrazione archivio\n");
            return 1;
        }
    }
    if (cflag == 1) {
        if (fflag == 0) {
            //-c prevede che gli venga passato un target con -f
            printf("non e\' stato passato il target -f!\n");
            fprintf(logger, "(EE) -c passato senza -f!\n");
            return 31;
        } else if (fvalue == NULL) {
            //-f non puo' essere null
            printf("il target passato con -f non e\' valido!\n");
            fprintf(logger, "(EE) -f ha come target NULL!\n");
            return 32;
        } else {
            fprintf(logger, "(II) Compressione archivio\n");
            return 2;
        }
    }
    if (fflag == 1) {
        //target passato senza nessun altro parametro
        printf("-f non puo\' essere passato da solo!\n");
        fprintf(logger, "(EE) -f inserito da solo!\n");
        return 40;
    } else {
        //nessun input passato
        printf("Uso del programma \"mkbkp [options] [target]"
                "\nOpzioni:\n\t-f [target] indica l\'obiettivo, necessario a "
                "opzioni quale -c o -x"
                "\n\t-c indica la creazione del backup, deve essere associato"
                "a -f per indicare dove mettere il file"
                "\n\t-x indica l\'estrazione del file indicato da -f, i file "
                "verranno estratti nella posizione <nomebkp>_d"
                "\n\t-t [target] indica la lettura del file di bkp target!\n");
        fprintf(logger, "(II) nessun input passato stampa dell\'help\n");
        return 50;
    }
}

/**
 * metodo per formattare il tempo corrente in maniera leggibile
 * @return la stringa che contiene l'orario formattato
 */
char* getTime() {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return asctime(timeinfo);
}

/**
 * si occupa dell'apertura del log
 * @return 1 se il log e' stato aperto con successo 0 altrimenti
 */
int openLog() {
    int retval;
    if (logger = fopen("/var/log/utility/mkbkp", "r")) {
        fclose(logger);
        retval = 1;
    } else
        retval = 0;
    logger = fopen("/var/log/utility/mkbkp", "a");
    if (retval == 0) {
        printf("Nuovo file di log creato %s", getTime());
        fprintf(logger, "Markers: (!!) debug, (II) informational, (WW) warning, (EE) error\n");
        fprintf(logger, "(II) Creazione log %s", getTime());
    }
    fprintf(logger, "(II) Nuova esecuzione del programma al tempo: %s", getTime());
    return retval;
}

/**
 * in base agli input del programma decide che procedura avviare o se terminare 
 * l'esecuzione
 * @param optindex index dopo il quale ci sono solo 
 * @param argc passato dal main e' il numero di parametri in input
 * @param targets passato dal main e' l'array di stringhe di valori in input
 */
void work(int optindex, int argc, char** targets) {
    int code = checkInputs();
    int i = 0;
    char * cmp;
    FILE *file;
    if (code >= 10) {
        fprintf(logger, "(EE) Errore negli input abort! \n");
        exit(0);
    } else if (code == 0) {
        if (file = fopen(tvalue, "r")) {
            char * str = tvalue + strlen(tvalue) - 4;
            int cmp = strcmp(str, ".bkp");
            if (cmp == 0) { //se finisce con ".bkp"
                fprintf(logger, "(II) Lettura del file di backup alla posizione "
                        "%s\n", tvalue);
                print(file);
            } else {
                fprintf(logger, "(EE) Il file indicato non e' stato "
                        "riconosciuto come file di backup %s\n", fvalue);
                printf("Il file non sembra essere un file di backup exiting\n");
                exit(0);
            }
        } else {
            fprintf(logger, "(EE) File non trovato alla posizione %s\n", tvalue);
            printf("file non trovato\n");
        }
    } else if (code == 1) {
        if (file = fopen(fvalue, "r")) {
            char * rts = (fvalue + strlen(fvalue) - 4);
            int cmp = strcmp(rts, ".bkp");
            if (cmp == 0) { //se finisce con .bkp
                fprintf(logger, "(II) Estrazione del backup alla posizione %s "
                        "nella posizione %s_d\n", fvalue, fvalue);
                extractBkp(file);
            } else {
                fprintf(logger, "(EE) Il file indicato non e' stato "
                        "riconosciuto come file di backup %s\n", fvalue);
                printf("Il file non sembra essere un file di backup exiting\n");
                exit(0);
            }
        } else {
            fprintf(logger, "(EE) File di backup non trovato alla posizione "
                    "%s\n", fvalue);
            printf("file di backup non trovato %s\n", fvalue);
        }
    } else if (code == 2) {
        char tmp[256];
        char * str = fvalue + strlen(fvalue) - 4;
        int cmp = strcmp(str, ".bkp");
        if (cmp != 0) {
            strcpy(tmp, fvalue);
            sprintf(tmp, "%s.bkp", fvalue);
        } else {
            strcpy(tmp, fvalue);
        }
        if (argc <= optindex) {
            fprintf(logger, "(EE) nessun parametro passato in input!\n");
            printf("nessun file da mettere nel backup\n");
        } else {
            if (file = fopen(tmp, "r")) {
                fclose(file);
                remove(tmp);
            }
            fprintf(logger, "(II) Inizio creazione del backup in %s\n", fvalue);
            
            for (i = optindex; i < argc; i++) {
                createBackup(tmp, targets[i]);
            }
    
        }
    }
}

/**
 * si occupa di creare il backup nella posizione indicata
 * @param path percorso base
 * @param bkpPath percorso in cui creare il backup
 * @param target percorso del file/cartella da aggiungere al backup
 */
void createBackup(char* bkpPath, char* target) {
    struct dirent **drnt;
    struct stat s;
    unsigned long dir_size;
    char tmp[512];
    FILE *ofile;
    FILE *ifile;
    strcpy(tmp, target); //salvo il target in una variabile temporanea

    if (stat(tmp, &s) == 0) { //metodo per verificare se il path tmp e' una cartella
        if (s.st_mode & S_IFREG) { //se e' un file lo aggiunge al bkp
            char ch;
            char tm[512];
            sprintf(tm, "%s\n", tmp);
            fprintf(logger, "(!!) File trovato %s\n", tmp);
            ofile = fopen(bkpPath, "a");
            ifile = fopen(tmp, "r");
            fputs(tm, ofile); //out del path del file
            sprintf(tm, "%lu ", s.st_size);
            fputs(tm, ofile); //output della lunghezza del file
            int i;
            ifile = fopen(tmp, "r");
            for (i = 0; i < s.st_size; ++i) { //output del file
                ch = fgetc(ifile);
                fputc(ch, ofile);
            }
            fputc('\n', ofile);
            fclose(ofile);
            fclose(ifile);
        } else if (s.st_mode & S_IFDIR) { //caso in cui target e' una cartella
            //aggiusto il path
            char tmp2[512];
            strcpy(tmp2, tmp);
            if (tmp[strlen(tmp) - 1] != '/') {
                sprintf(tmp, "%s%s", tmp, "/");
            }
            fprintf(logger, "(!!) Directory trovata %s\n", tmp);
            
            dir_size = scandir(tmp, &drnt, 0, alphasort);
            int i;
            for (i = 0; i < dir_size; i++) { //riavvio la funzione ricorsivamente sugli elementi della cartella
                sprintf(tmp2, "%s%s", tmp, drnt[i]->d_name);
                if (strcmp(".", drnt[i]->d_name) == 0 || strcmp("..", drnt[i]->d_name) == 0)
                    continue;
                createBackup(bkpPath, tmp2);
            }
        }
    } else {
        fprintf(logger, "(WW) File non esistente \"%s\"\n", tmp);
    }
}

/**
 * metodo che restituisce in output il contenuto di un file di backup
 * @param f file di backup da leggere
 */
void print(FILE *f) {
    char path[512];
    char ch;
    unsigned long chars;
    //scorro il file fino in fondo
    while (fscanf(f, "%s", path) != EOF) {
        fscanf(f, "%lu", &chars);
        printf("%s\tsize: %lu\n", path, (chars * sizeof (char)));
        int i;
        for (i = 0; i < chars; i++) { //scorro tutti i byte del file
            ch = fgetc(f);
        }
    }
}

/**
 * estrae il backup nella posizione %s_d con %s uguale al nome del file di bkp
 * @param bkp file di backup da cui estrarre
 */
void extractBkp(FILE * bkp) {
    char path[512];
    char basepath[512];
    char tmp[512];
    char full[512];
    char ch;
    int i;
    unsigned long file_size;
    sprintf(basepath, "%s%s", fvalue, "_d");
    FILE * out;
    if (basepath[strlen(basepath) - 1] != '/') {
        sprintf(basepath, "%s%s", basepath, "/");
    }
    while (fscanf(bkp, "%s", path) != EOF) {
        fscanf(bkp, "%lu", &file_size);
        sprintf(tmp, "%s%s%s", "./", basepath, path);
        strcpy(full, tmp);

        i = strlen(tmp) - 1;
        do {
            i--;
        } while (tmp[i] != '/');
        tmp[i + 1] = '\0';
        recMkdir(tmp);
        tmp[i + 1] = '/';

        out = fopen(full, "w");
        for (i = 0; i < file_size; i++) {
            ch = fgetc(bkp);
            fputc(ch, out);
        }
    }
}

/**
 * metodo per creare directory ricorsivamente (da stackoverflow)
 * @param dir path da create
 */
static void recMkdir(const char *dir) {

    fprintf(logger, "(!!) Creo il path %s\n", dir);

    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof (tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {

            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);

}
