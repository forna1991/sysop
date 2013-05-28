#include "equals.h" //contiene tutti gli include e le firme dei metodi

FILE * logger;

int main(int argc, char **argv) {
    int retval;
    
    openLog(); //apre il log
    fprintf(logger, "(II) Nuova esecuzione del programma al tempo: %s", getTime());

    // stampa il comando inserito nel file di log
    int i;
    fprintf(logger, "(II) Comando inserito : ");
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
        printf("File/directory %s non trovato \n", argv[1]);
        return 1;
    }
    fclose(file);
    if (!(file = fopen(argv[2], "r"))) {
        fprintf(logger, "File/directory %s non trovato \n", argv[2]);
        printf("File/directory %s non trovato \n", argv[2]);
        return 1;
    }
    fclose(file);


    // se i due file/directory coincidono perfettamente allora stampo video e nel log
    // che coincidono altrimenti solo nel log che non coincidono
    char pt1[1000], pt2[1000];
    realpath(argv[1], pt1);
    realpath(argv[2], pt2);

    retval = !equals(pt1, pt2);
    fclose(logger);
    return retval;

}

/**
 * confronta i due path dati in input per decidere se sono equivalenti o meno
 * @param patha primo path da confrontare con il secondo
 * @param pathb secondo path da confrontare con il primo
 * @return 1 se i due path sono un file identico anche per nome o una cartella
 * con dentro file uguali tra loro 0 altrimenti
 */
int equals(char * patha, char * pathb) {
    fprintf(logger, "(!!) confronto tra %s %s", patha, pathb);
    int retval = 1; //TRUE
    struct stat a, b;

    if (stat(patha, &a) == 0 && stat(pathb, &b) == 0) {
        if (a.st_mode & S_IFREG) {
            if (b.st_mode & S_IFREG) {
                //sono entrambi file li confronto tra di loro
                if (confrontafile(patha, pathb)) {
                    return 1;
                } else {
                    return 0;
                }
            } else if (b.st_mode & S_IFDIR) {
                printf("%s è un file mentre %s è una cartella\n", patha, pathb);
                return 0;
            }
        } else if (a.st_mode & S_IFDIR) {
            if (b.st_mode & S_IFREG) {
                printf("%s è un file mentre %s è una cartella\n", pathb, patha);
                return 0;
            } else if (b.st_mode & S_IFDIR) {
                //sono entrambe directory, lancio la funzione per confrontarle
                return recursiveDirectory(pathb, patha);
            }
        }
    }
    return retval;
}

/**
 * confronta due directory per vedere se hanno gli stessi file (solo come nome)
 * su tutti i file delle due cartelle che hanno lo stesso nome rilancia la 
 * equals che confronta i 2 file per vedere se sono uguali
 * @param patha path della prima cartella
 * @param pathb path della seconda cartella
 * @return 1 se le cartelle hanno gli stessi file (solo come nome) 0 altrimenti
 */
int recursiveDirectory(char * patha, char * pathb) {
    fprintf(logger, "(!!) controllo delle due cartelle %s %s", patha, pathb);
    struct dirent **drnt1;
    struct dirent **drnt2;
    int retval = 1;
    int n1 = 0, n2 = 0;
    int i, j;
    char tmp1[256], tmp2[256];

    //restituisce un array ordinato di dirent
    n1 = scandir(patha, &drnt1, 0, alphasort);
    n2 = scandir(pathb, &drnt2, 0, alphasort);

    j = 2; //i contatori partono da 2 per saltare le cartelle "." e ".."
    for (i = 2; i < n1 || j < n2;) { //finche non ho finito di scorrere entrambe le cartelle
        int cmp;
        //controllo se uno dei due contatori e' uscito dal vettore
        if (i < n1 && j < n2) {
            strcpy(tmp1, drnt1[i]->d_name);
            strcpy(tmp2, drnt2[j]->d_name);
            cmp = strcmp(tmp1, tmp2);
            sprintf(tmp2, "%s/%s", pathb, drnt2[j]->d_name);
            sprintf(tmp1, "%s/%s", patha, drnt1[i]->d_name);
        } else if (i == n1) {
            sprintf(tmp2, "%s/%s", pathb, drnt2[j]->d_name);
            cmp = 1;
        } else {
            sprintf(tmp1, "%s/%s", patha, drnt1[i]->d_name);
            cmp = -1;
        }

        //in base al risultato del cmp tra i 2 path decido quale puntatore portare
        //avanti, se lo strcmp non da 0 porto avanti il risultato piu' piccolo
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

/**
 * confronta i due file per decidere se sono uguali o no
 * @param file1 path del primo file
 * @param file2 path del secondo file
 * @return 1 se i file hanno uguale nome e contenuto, 0 altrimenti
 */
int confrontafile(char * file1, char *file2) {
    char ch1, ch2;
    FILE *f1, *f2;
    f1 = fopen(file1, "r");
    f2 = fopen(file2, "r");
    ch1 = getc(f1);
    ch2 = getc(f2);
    fprintf(logger, "(!!) confronto dei due file %s %s", file1, file2);

    if (ch1 != ch2)
        return 0;
    while (ch1 != EOF || ch2 != EOF) {
        if (ch1 != ch2) {
            //se i caratteri differiscono chiude i file ed esce ritornando 0
            fclose(f1);
            fclose(f2);
            return 0;
        }
        ch2 = getc(f1);
        ch1 = getc(f2);
    }
    //se arriva in fondo al ciclo vuol dire che i caratteri sono sempre stati 
    //uguali quindi ritorna 1
    fclose(f1);
    fclose(f2);
    return 1;
}

/**
 * funzione per stampare la data in formato leggibile da un umano
 * @return una stringa con l'ora corrente
 */
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
    if (logger = fopen("/var/log/utility/equals", "r")) {
        fclose(logger);
        retval = 1;
    } else
        retval = 0;
    logger = fopen("/var/log/utility/equals", "a");

    // se non esiste lo crea e ci scrive 2 righe di informazione
    if (retval == 0) {
        printf("%s : Nuovo file di log creato\n", getTime());
        fprintf(logger, "Markers: (!!) debug, (II) informational, (WW) warning, (EE) error\n");
        fprintf(logger, "(II) Creazione log %s", getTime());
    }
    fprintf(logger, "(II) Nuova esecuzione del programma al tempo: %s", getTime());
    return retval;
}