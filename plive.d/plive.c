/** 
 * File:    plive.c
 * anno accademico 2012/2013
 * Progetto #1
 * Giulio Fornasaro     151991
 * Francesco Boschini   151840
 * Luca Zamboni         151759
 */

#include <sys/syslog.h>

#include "plive.h"

//File di log
FILE *logger;

const int LUNGHEZZA_ARRAY = 100000;
const int NUMERO_MAX_PROC = 1000;

/*Num_vis è è il numero di processi da visualizzare nella shell*/
int num_vis = 10;

/*Lo sleep*/
int dormi = 1;

/*Boooleano che dice se è stata premuta la q*/
int ext = 0;

/*Dichiarazione dei 2 thread
mthread è il thread che fa il check dei tasti premuti
mainthread è il thread che stampa i processi
 */
pthread_t mthread, mainthread;

int main(int argc, char **argv) {
    int c;
    int error = -1;
    int i;
    int nflag;
    char temp[512]="(II) impostazioni inserite :";

    openlog("plive", LOG_ODELAY || LOG_PID, LOG_SYSLOG);

    // stampa il comando inserito nel file di log
    for (i = 0; i < argc; i += 1) {
        strcat(temp, " ");
        strcat(temp, argv[i]);
    }
    strcat(temp, "\n");
    syslog(LOG_NOTICE, "%s", temp);

    // controlla se è stata inserita l'opzione n e se è statsa inserita un opzione non valida
    // termina l'esecuzione del programma
    while ((c = getopt(argc, argv, "n:")) != -1) {
        switch (c) {
            case 'n':
                num_vis = atoi(optarg);
                break;
            default:
                syslog(LOG_NOTICE, "(EE) opzione non riconosciuta\n");
                exit(EXIT_SUCCESS);
        }
    }
       
    initscr();

    // crea e starta i due thread
    pthread_create(&mainthread, NULL, &controllocaratteri, NULL);
    pthread_create(&mthread, NULL, &printProc, NULL);
    pthread_join(mthread, NULL);
    pthread_join(mainthread, NULL);

    return 0;
}


//funzione che controlla gli input da tastiera durante l'esecuzione del programma

void* controllocaratteri(void *arg) {
    while (!ext) {
        char w;
        w = getch();

        if (w == 'q') {
            syslog(LOG_NOTICE, "(II) Premuto q -- Uscita dal programma al tempo: %s", getTime());
            ext = 1;
        } else {
            if (w < '9' && w >= '0') {
                syslog(LOG_NOTICE, "(II) Premuto %c -- Aggiornamento ogni %c secondi: %s", w, w, getTime());
                dormi = w - '0';
            }
        }
    }
    //return NULL;
}

//funzione che stampa i processi

void* printProc(void *arg) {
    int i;
    int index = 0;

    struct data totalTimeCpu[LUNGHEZZA_ARRAY];
    struct data totalTimeCpuBefore[LUNGHEZZA_ARRAY];

    struct data user_util[1000];

    float time_total_after, time_total_before;

    // inizializzo l'array dei processi
    getArrayUserTime(totalTimeCpuBefore);

    while (!ext) {

        // prendo il nuovo valore
        getArrayUserTime(totalTimeCpu);

        //time_total_after = getUserCpu();
        azzeraArray(user_util, NUMERO_MAX_PROC);
        index = 0;

        // con questo for calcola per ogni processo il tempo di cpu
        for (i = 0; i < LUNGHEZZA_ARRAY; i++) {
            if (totalTimeCpu[i].userTime != 0) {
                user_util[index].userTime = 100 * (totalTimeCpu[i].userTime - totalTimeCpuBefore[i].userTime) / (100 * dormi);
                user_util[index].pid = i;
                index++;
            }
        }


        //faccio il sort dell'array in modo da da avere quelli che usano piu cpu in cima all'array
        sortArray(user_util);

        //faccio il clear screen
        //printf("\x1B[2J\x1B[1;1H");
        clear();

        int result;

        //Stampo a video i vari dati
        printw("Pid: \tCpu: \tPpid: \tName:\n\n");
        for (i = 0; i < num_vis; i++) {
            printw("%d \t%d ", user_util[i].pid, user_util[i].userTime);
            printInfo(user_util[i].pid);
            printw("\n");
            refresh();
        }

        copiaArray(totalTimeCpu, totalTimeCpuBefore);
        //time_total_before = time_total_after;

        sleep(dormi);
    }
    endwin();
    exit(EXIT_SUCCESS);
}


//stampa a video il parent pid e il nome del processo

void printInfo(int pid) {
    char target[256];
    char stringa[256];
    char name[256];
    sprintf(target, "/proc/%d/stat", pid);

    int i = 0;
    FILE *ifile;
    char * aux;
    if (ifile = fopen(target, "r")) {
        while (fscanf(ifile, "%s", stringa) != EOF) {
            if (i == 1) {
                snprintf(name, sizeof (char) * 256, "%s", stringa);

                // substring per togliere le parentesi
                aux = name + 1;
                aux[strlen(aux) - 1] = '\0';

            }
            if (i == 3) {
                printw("\t%s", stringa);
                fclose(ifile);
            }
            i++;
        }
    }
    printw("\t%s", aux);
}

//Funzione che mette nell'array le informazioni di ogni processo

void getArrayUserTime(struct data * array) {
    struct dirent *drnt;
    struct stat s;
    DIR *dr;
    char tmp[256];
    char target[256];
    int pid = 0;

    // prima della lettura azzera l'array
    azzeraArray(array, LUNGHEZZA_ARRAY);

    /*target = malloc(snprintf(NULL, 0, "%s","") + 1);
    tmp = malloc(snprintf(NULL, 0, "%s","") + 1);*/
    sprintf(target, "%s", "/proc/");

    //apre la cartella e guarda tutti i file e cartelle
    dr = opendir(target);
    while (drnt = readdir(dr)) {
        if (strcmp(".", drnt->d_name) == 0 || strcmp("..", drnt->d_name) == 0)
            continue;

        //le cartelle che hanno come prima lettera un numero vuol dire 
        // che sono le cartelle che contengono le info dei processi
        if (drnt->d_name[0] < '9' && drnt->d_name[0] >= '0') {
            //creo la stringa contenente il percorso del file stat dento proc/<pid>
            sprintf(tmp, "%s%s%s", target, drnt->d_name, "/stat");
            pid = atoi(drnt->d_name);
            int auux = getProcCpu(tmp);
            struct data aux = {pid, auux};

            // riempio l'array con le info che mi interessan
            array[pid] = aux;

        }
    }
    closedir(dr);

    /*free(target);
    free(tmp);*/
}

//funzione di suppoto che fa il sort di un array di tipo struct data

void sortArray(struct data * array) {
    int i, j;
    struct data temp;
    for (i = 0; i < NUMERO_MAX_PROC; i++) {
        for (j = i + 1; j < NUMERO_MAX_PROC; j++) {
            if (array[i].userTime < array[j].userTime) {
                temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}

//funzione di supporto che copia l' array1 nell array2

void copiaArray(struct data * array, struct data * array2) {
    int i;
    for (i = 0; i < LUNGHEZZA_ARRAY; i++) {
        array2[i] = array[i];
    }
}

//funzione che ritorna quanto un processo ha occupato la cpu
//per semplicità gli passo il percorso del file dove è contenuta l'informazione

float getProcCpu(char *percorso) {
    int i = 0;
    int indexProc = 13;
    char stringa[100];
    FILE *ifile;
    ifile = fopen(percorso, "r");
    while (fscanf(ifile, "%s", stringa) != EOF) {

        //ciclo fino a indexProc che è la posizione del' informazione che mi interessa
        if (i == indexProc) {
            fclose(ifile);
            return atof(stringa);
        }
        i++;
    }
    return 0;
}

//funzione di supporto che azzera un array

void azzeraArray(struct data * array, int n) {
    int i;
    for (i = 0; i < n; i++) {
        struct data aux = {i, 0};
        array[i] = aux;
    }
}

//funzione che ritorna la data sottoforma di stringa

char* getTime() {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return asctime(timeinfo);
}