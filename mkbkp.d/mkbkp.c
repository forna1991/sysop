/* 
 * File:   mkbkp.c
 * Author: forna
 *
 * Created on 23 maggio 2013, 10.50
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int fflag = 0;
int xflag = 0;
int cflag = 0;
int tflag = 0;
char *fvalue = NULL;
char *tvalue = NULL;
FILE *logger;
const char * backup_begin = "bkpfilemkbkptool";

int checkInputs();
void work(int, int, char**);
void createBackup(char*, char*, char*);
int openLog();
char* getTime();
void print(FILE *);
void extractBkp(FILE *);
static void recMkdir(const char *);
char *shiftString(const char *, int, int);

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
        } else if (fvalue == NULL) {
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
        } else if (fvalue == NULL) {
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
    if (logger = fopen("./mkbkplog", "r")) {
        fclose(logger);
        retval = 1;
    } else
        retval = 0;
    logger = fopen("./mkbkplog", "a");
    if (retval == 0) {
        fprintf(logger, "Markers: (!!) notice, (II) informational, (WW) warning, (EE) error\n");
        fprintf(logger, "(!!) Creazione log %s", getTime());
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
        abort();
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
        fprintf(logger, "(!!) Inizio creazione del backup in %s\n", fvalue);
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
    printf("%s\t%s\t%s\n", path, bkpPath, target);
    struct dirent *drnt;
    struct stat s;
    DIR *dr;
    char* tmp;
    FILE *ofile;
    FILE *ifile;

    tmp = malloc(snprintf(NULL, 0, "%s", target) + 1);
    sprintf(tmp, "%s", target);

    if (stat(tmp, &s) == 0) {
        if (s.st_mode & S_IFREG) {
            int count = 0;
            char ch;
            char* tm = malloc(snprintf(NULL, 0, "%s\n", tmp) + 1);
            sprintf(tm, "%s\n", tmp);
            fprintf(logger, "(!!) File trovato %s\n", tmp);
            ofile = fopen(bkpPath, "a");
            ifile = fopen(tmp, "r");
            fputs(tm, ofile);
            ch = getc(ifile);
            while (ch != EOF) {
                count++;
                ch = getc(ifile);
            }
            int i;
            ifile = fopen(tmp, "r");
            fprintf(ofile, "%d", count);
            for (i = 0; i < count; ++i) {
                ch = getc(ifile);
                fputc(ch, ofile);
            }
        } else if (s.st_mode & S_IFDIR) {

            if (target[strlen(target) - 1] != '/') {
                sprintf(target, "%s%s", target, "/");
            }
            fprintf(logger, "(!!) Directory trovata %s\n", target);

            dr = opendir(target);
            while (drnt = readdir(dr)) {
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
void print(FILE *f) { //TODO fare un controllo sul file
    char * path = malloc(snprintf(NULL, 0, "%s", "") + 1);
    char ch;
    int chars;
    while (fscanf(f, "%s", path) != EOF) {
        fscanf(f, "%d", &chars);
        printf("%s\tsize: %lu\n", path, (chars * sizeof (char)));
        int i;
        for (i = 0; i < chars; i++) {

            ch = getc(f);
        }
    }
}

/**
 * 
 * @param bkp file di backup da cui estrarre
 */
void extractBkp(FILE * bkp) {
    char * path = malloc(snprintf(NULL, 0, "%s", "") + 1);
    char * basepath = malloc(snprintf(NULL, 0, "%s", "") + 1);
    char * tmp = malloc(snprintf(NULL, 0, "%s", "") + 1);
    char * full = malloc(256 * sizeof (char));
    char ch;
    int i, file_size, count;
    sprintf(basepath, "%s%s", fvalue, "_d");
    FILE * out;
    if (basepath[strlen(basepath) - 1] != '/') {
        sprintf(basepath, "%s%s", basepath, "/");
    }
    while (fscanf(bkp, "%s", path) != EOF) {
        fscanf(bkp, "%d", &count);
        //printf("%s %s\n", basepath, path);
        sprintf(tmp, "%s%s%s", "./", basepath, path);
        strcpy(full, tmp);

        i = strlen(tmp) - 1;
        do {
            i--;
        } while (tmp[i] != '/');
        tmp[i + 1] = '\0';
        //printf("tua madre e' %s\n", tmp);
        recMkdir(tmp);
        tmp[i + 1] = '/';

        out = fopen(full, "a+");
        for (i = 0; i < count; i++) {

            ch = getc(bkp);
            putc(ch, out);
        }
    }
}

static void recMkdir(const char *dir) {

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

char *shiftString(const char *string, int position, int length) {
    char *pointer;
    int c;

    printf("%s %s %i %i\n", "shifto la stringa ", string, position, length);

    pointer = malloc(length + 1);

    if (pointer == NULL) {
        printf("Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("%s", "before fors\n");
    for (c = 0; c < position - 1; c++)
        string++;


    printf("%s", "between fors\n");

    for (c = 0; c < length; c++) {
        *(pointer + c) = *string;
        string++;
    }

    *(pointer + c) = '\0';

    printf("%s\n", "finito di shiftare la stringa");

    return pointer;
}
