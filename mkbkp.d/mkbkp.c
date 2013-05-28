/* 
 * File:   mkbkp.c
 */

#include "mkbkp.h"

int fflag = 0;
int xflag = 0;
int cflag = 0;
int tflag = 0;
char *fvalue = NULL;
char *tvalue = NULL;
FILE *logger;
const char * backup_begin = "bkpfilemkbkptool";

main(int argc, char **argv) {
    int index;
    int c;
    int error = -1;
    int i;

    openLog(); //apertura del file di log
    //output del comando inserito dall'utente nel log
    fprintf(logger, "(II) impostazioni inserite : ");
    for (i = 0; i < argc; i += 1) {
        fprintf(logger, "%s ", argv[i]);
    }
    fprintf(logger, "\n");

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
    FILE *file;
    if (code >= 10) {
        fprintf(logger, "(EE) Errore negli input abort! \n");
        exit(0);
    } else if (code == 0) {
        if (file = fopen(tvalue, "r")) {
            char * str = malloc(256 * sizeof (char));
            fscanf(file, "%s", str);
            if (strncmp(str, backup_begin,
                    sizeof (char) * strlen(backup_begin)) == 0) {
                fprintf(logger, "(II) Lettura del file di backup alla posizione "
                        "%s\n", tvalue);
                print(file);
            } else {
                fprintf(logger, "(EE) Il file indicato non e' stato "
                        "riconosciuto come file di backup %s\n", fvalue);
                printf("Il file non sembra essere un file di backup aborting\n");
                abort();
            }
        } else {
            fprintf(logger, "(EE) File non trovato alla posizione %s\n", tvalue);
            printf("file non trovato\n");
        }
    } else if (code == 1) {
        if (file = fopen(fvalue, "r")) {
            char * str = malloc(256 * sizeof (char));
            fscanf(file, "%s", str);
            printf("%s\t%s", str, backup_begin);
            if (strncmp(str, backup_begin,
                    sizeof (char) * strlen(backup_begin)) == 0) {
                fprintf(logger, "(II) Estrazione del backup alla posizione %s "
                        "nella posizione %s_d\n", fvalue, fvalue);
                extractBkp(file);
            } else {
                fprintf(logger, "(EE) Il file indicato non e' stato "
                        "riconosciuto come file di backup %s\n", fvalue);
                printf("Il file non sembra essere un file di backup aborting\n");
                abort();
            }
        } else {
            fprintf(logger, "(EE) File di backup non trovato alla posizione "
                    "%s\n", tvalue);
            printf("file di backup non trovato\n");
        }
    } else if (code == 2) {
        if (file = fopen(fvalue, "r")) {
            fclose(file);
            remove(fvalue);
        }
        fprintf(logger, "(II) Inizio creazione del backup in %s\n", fvalue);
        for (i = optindex; i < argc; i++) {
            createBackup("./", fvalue, targets[i]);
        }
        FILE *fl = fopen(fvalue, "a");
        fprintf(fl, "%s\n", backup_begin);
    }
}

/**
 * si occupa di creare il backup nella posizione indicata
 * @param path percorso base
 * @param bkpPath percorso in cui creare il backup
 * @param target percorso del file/cartella da aggiungere al backup
 */
void createBackup(char* path, char* bkpPath, char* target) {
    struct dirent *drnt;
    struct stat s;
    DIR *dr;
    char* tmp;
    FILE *ofile;
    FILE *ifile;

    tmp = malloc(snprintf(NULL, 0, "%s", target) + 1);
    sprintf(tmp, "%s", target); //salvo il target in una variabile temporanea

    if (stat(tmp, &s) == 0) { //metodo per verificare se il path tmp e' una cartella
        if (s.st_mode & S_IFREG) { //se e' un file lo aggiunge al bkp
            int file_size = 0;
            char ch;
            char* tm = malloc(snprintf(NULL, 0, "%s\n", tmp) + 1);
            sprintf(tm, "%s\n", tmp);
            fprintf(logger, "(!!) File trovato %s\n", tmp);
            ofile = fopen(bkpPath, "a");
            ifile = fopen(tmp, "r");
            fputs(tm, ofile); //out del path del file
            ch = getc(ifile);
            while (ch != EOF) { //controllo la lunghezza del file
                file_size++;
                ch = getc(ifile);
            }
            int i;
            ifile = fopen(tmp, "r");
            fprintf(ofile, "%d", file_size); //output della lunghezza del file
            for (i = 0; i < file_size; ++i) { //output del file
                ch = getc(ifile);
                fputc(ch, ofile);
            }
        } else if (s.st_mode & S_IFDIR) { //caso in cui target e' una cartella
            //aggiusto il path
            if (target[strlen(target) - 1] != '/') {
                sprintf(target, "%s%s", target, "/");
            }
            fprintf(logger, "(!!) Directory trovata %s\n", target);

            dr = opendir(target);
            while (drnt = readdir(dr)) { //riavvio la funzione ricorsivamente sugli elementi della cartella
                sprintf(tmp, "%s%s", target, drnt->d_name);
                if (strcmp(".", drnt->d_name) == 0 || strcmp("..", drnt->d_name) == 0)
                    continue;
                createBackup(path, bkpPath, tmp);
            }
            closedir(dr);
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
    char * path = malloc(snprintf(NULL, 0, "%s", "") + 1);
    char ch;
    int chars;
    //scorro il file fino in fondo
    while (fscanf(f, "%s", path) != EOF) {
        fscanf(f, "%d", &chars);
        printf("%s\tsize: %lu\n", path, (chars * sizeof (char)));
        int i;
        for (i = 0; i < chars; i++) { //scorro tutti i byte del file
            ch = getc(f);
        }
    }
}

/**
 * estrae il backup nella posizione %s_d con %s uguale al nome del file di bkp
 * @param bkp file di backup da cui estrarre
 */
void extractBkp(FILE * bkp) {
    char * path = malloc(snprintf(NULL, 0, "%s", "") + 1);
    char * basepath = malloc(snprintf(NULL, 0, "%s", "") + 1);
    char tmp[256] = "";
    char * full = malloc(256 * sizeof (char));
    char ch;
    int i, file_size;
    sprintf(basepath, "%s%s", fvalue, "_d");
    FILE * out;
    if (basepath[strlen(basepath) - 1] != '/') {
        sprintf(basepath, "%s%s", basepath, "/");
    }
    while (fscanf(bkp, "%s", path) != EOF) {
        fscanf(bkp, "%d", &file_size);
        sprintf(tmp, "%s%s%s", "./", basepath, path);
        strcpy(full, tmp);

        i = strlen(tmp) - 1;
        do {
            i--;
        } while (tmp[i] != '/');
        tmp[i + 1] = '\0';
        recMkdir(tmp);
        tmp[i + 1] = '/';

        out = fopen(full, "a+");
        for (i = 0; i < file_size; i++) {
            ch = getc(bkp);
            putc(ch, out);
        }
    }
    free(path);
    free(basepath);
    free(full);
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
