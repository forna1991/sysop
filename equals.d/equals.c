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
                if (confrontafile(patha, pathb)) {
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
                int r1 = 1;
                return recursiveDirectory(pathb, patha);
            }
        }
    }
    return retval;
}

int recursiveDirectory(char * patha, char * pathb) {
    struct dirent **drnt1;
    struct dirent **drnt2;
    int retval = 1;
    int n1 = 0, n2 = 0;
    int i, j;
    int check1 = 1, check2 = 1;
    char tmp1[256], tmp2[256];

    n1 = scandir(patha, &drnt1, 0, alphasort);
    n2 = scandir(pathb, &drnt2, 0, alphasort);

    j = 2;
    for (i = 2; i < n1 || j < n2;) {
        int cmp;
        //printf("debug %d %d %d %d\n", i, j, n1, n2);
        if (i < n1 && j < n2) {
            strcpy(tmp1, drnt1[i]->d_name);
            strcpy(tmp2, drnt2[j]->d_name);
            cmp = strcmp(tmp1, tmp2);
            sprintf(tmp2, "%s/%s", pathb, drnt2[j]->d_name);
            sprintf(tmp1, "%s/%s", patha, drnt1[i]->d_name);
        } else if (i==n1) {
            sprintf(tmp2, "%s/%s", pathb, drnt2[j]->d_name);
            cmp = 1;
        } else {
            sprintf(tmp1, "%s/%s", patha, drnt1[i]->d_name);
            cmp = -1;
        }

        if (cmp == 0) {
            i++;
            j++;
            equals(tmp1, tmp2);
        } else if (cmp > 0) {
            j++;
            retval = 0;
            printf("l'elemento %s non e\' presente in %s\n", tmp2, patha);
        } else {
            i++;
            retval = 0;
            printf("l'elemento %s non e\' presente in %s\n", tmp1, pathb);
        }
    }
    return retval;
}

int confrontafile(char * file1, char *file2) {
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