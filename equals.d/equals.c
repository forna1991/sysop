#include "equals.h"

FILE *logger;

int main(int argc, char **argv) {

    openLog();

    // stampa il comando inserito nel file di log
    fprintf(logger, "(II) Comando inserito : ");
    int i;
    for (i = 0; i < argc; i += 1) {
        fprintf(logger, "%s ", argv[i]);
    }
    fprintf(logger, "\n");

    // se gli argomenti non sono 3 ciao ./equals patha e pathb allora lo stampo a video
    // lo stampo nel log e poi termino l'esecuzione
    if (argc != 3) {
        fprintf(logger, "(EE) Wrong command -- usage : ./equals <path A> <path B> \n");
        printf("Wrong command -- usage : ./equals <path A> <path B> \n");
        return 1;
    }

    // controllo che i due file/directory esistano sul disco altrimenti lo stampo nel log e a video
    FILE *file;
    if (!(file = fopen(argv[1], "r"))) {
        fprintf(logger, "(EE) File/directory %s non trovato \n", argv[1]);
        printf("(EE) File/directory %s non trovato \n", argv[1]);
        return 1;
    }
    fclose(file);
    if (!(file = fopen(argv[2], "r"))) {
        fprintf(logger, "(EE) File/directory %s non trovato \n", argv[2]);
        printf("(EE) File/directory %s non trovato \n", argv[2]);
        return 1;
    }
    fclose(file);


    // se i due file/directory coincidono perfettamente allora stampo video e nel log
    // che coincidono altrimenti solo nel log che non coincidono
    char pt1[1000], pt2[1000];
    realpath(argv[1], pt1);
    realpath(argv[2], pt2);
    printf("\n\n%d\n\n", /*isSameLevel(pt1, pt2)/**/!equals(pt1, pt2)/**/);

}

int equals(char * patha, char * pathb) {
    //controllo sul pathing
    //se e' una cartella rilanciare su tutti gli elementi
    //se e' un file vedere se nell'altro path c'e' un file e nel caso confrontarli tra loro
    int retval = 1; //TRUE
    struct stat a, b;

    if (stat(patha, &a) == 0 && stat(pathb, &b) == 0) {
        if (a.st_mode & S_IFREG) {
            if (b.st_mode & S_IFREG) {
                //scorrere tutti e due i file e se differiscono stampare i path
                //e scrivere che sono differenti
                if (confontafile(patha, pathb)) {
                    return 1;
                    //printf("%s e %s coincidono perfettamente\n",patha,pathb);
                } else {
                    return 0;
                    //printf("%s e %s sono differrenti\n",patha,pathb );
                }
            } else if (b.st_mode & S_IFDIR) {
                return 0;
            }
        } else if (a.st_mode & S_IFDIR) {
            if (b.st_mode & S_IFREG) {
                return 0;
            } else if (b.st_mode & S_IFDIR) {
                int r1 = 1, r2;
                r1 = recursiveDirectory(pathb, patha);
                r2 = recursiveDirectory(patha, pathb);
                if (r1 == 1 && r2 == 1) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }
    return retval;
}

int recursiveDirectory(char * patha, char * pathb) {

/*
    if (!isSameLevel(patha, pathb)) {
        return 0;
    }
*/

    struct dirent **drnt1;
    struct dirent **drnt2;
    int retval = 1, god = 1;
    int n1 = 0, n2 = 0;
    int check1 = 1, check2 = 1;
    char tmp1[256], tmp2[256];
    
    n1 = scandir(patha, &drnt1, 0, alphasort);
    n2 = scandir(pathb, &drnt2, 0, alphasort);
    
    while (n1--) {
        printf("%s\n", drnt1[n1]->d_name);
    } 
    
    /*
        while (drnt1 = readdir(dr1)) { //riavvio la funzione ricorsivamente sugli elementi della cartella
            if (strcmp(".", drnt1->d_name) == 0 || strcmp("..", drnt1->d_name) == 0)
                continue;

            dr2 = opendir(pathb);
            int ret = 0;
            while (drnt2 = readdir(dr2)) {
                if (!(strcmp(".", drnt2->d_name) == 0 || strcmp("..", drnt2->d_name) == 0)) {
                    sprintf(tmp2, "%s/%s", pathb, drnt2->d_name);
                    sprintf(tmp1, "%s/%s", patha, drnt1->d_name);
                    //printf("%s %s\n",tmp1,tmp2);
                    if (equals(tmp1, tmp2)) {
                        ret = 1;
                        //printf("%s %s\n",tmp1,tmp2);
                    }
                }
            }
            closedir(dr2);
            if (!ret) {
                retval = 0;
                struct stat a;
                if (stat(tmp1, &a) == 0) {
                    if (a.st_mode & S_IFREG) {
                        printf("Non c'è un file in %s identico a %s\n", pathb, tmp1);
                    }
                }
            }

        }
     */

    n1 = numfileinddir(patha);
    n2 = numfileinddir(pathb);

    if (n1 != n2 && retval == 1) {
        printf("%s e %s contengono un nuemro di elementi diverso\n", patha, pathb);
        retval = 0;
    }

    return retval;
}

int isSameLevel(char * pt1, char * pt2) {
    int i, j;
    int n1 = 0, n2 = 0;
    char c1, c2;
    c1 = pt1[0];
    c2 = pt2[0];
    for (i = 0; c1 != '\0'; i++) {
        if (c1 == '/') {
            n1++;
        }
        c1 = pt1[i];
    }
    for (j = 0; c2 != '\0'; j++) {
        if (c2 == '/') {
            n2++;
        }
        c2 = pt2[j];
    }

    if (c1 == '/') c1 = pt1[--i];
    if (c2 == '/') c2 = pt2[--j];

    while (c1 != '/' && c2 != '/') {
        if (c1 != c2) return 0;
        c1 = pt1[--i];
        c2 = pt2[--j];
    }

    return n1 == n2;
}

int numfileinddir(char * path) {
    struct dirent *drnt;
    DIR *dr;
    dr = opendir(path);
    int ret = 0;
    while (drnt = readdir(dr)) {
        if (!(strcmp(".", drnt->d_name) == 0 || strcmp("..", drnt->d_name) == 0)) {
            ret++;
        }
    }
    closedir(dr);
    return ret;
}

int confontafile(char * file1, char *file2) {
    /*if(strcmp(file1,file2)){
        return 0;
    }*/
    //printf("%s %s\n", file1,file2);
    char ch1, ch2;
    FILE *f1, *f2;
    f1 = fopen(file1, "r");
    f2 = fopen(file2, "r");
    ch1 = getc(f1);
    ch2 = getc(f2);
    if (ch1 != ch2)
        return 0;
    while (ch1 != EOF || ch2 != EOF) {
        if (ch1 != ch2) {
            fclose(f1);
            fclose(f2);
            return 0;
        }
        ch2 = getc(f1);
        ch1 = getc(f2);
    }
    fclose(f1);
    fclose(f2);
    return 1;
}

//funzione che ritorna la data sottoforma di stringa

char* getTime() {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
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