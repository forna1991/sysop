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
pthread_t mthread,mainthread;

int main (int argc, char **argv) {
	int c;
	int error = -1;
	int i;
	int nflag;
	
	openLog();

	// stampa il comando inserito nel file di log
	fprintf(logger, "(II) impostazioni inserite : "); 
	for (i = 0; i < argc; i += 1) {
		fprintf(logger, "%s ", argv[i]);
	}
	fprintf(logger, "\n");

	// controlla se è stata inserita l'opzione n e se è statsa inserita un opzione non valida
	// termina l'esecuzione del programma
	while ((c = getopt (argc, argv, "n:")) != -1){
		switch (c) {
			case 'n':
		 		num_vis = atoi(optarg);
		 		break;
	   		default:
	   			fprintf(logger, "(EE) opzione non riconosciuta\n");
		 		abort ();
	   }
	}

	// crea e starta i due thread
	pthread_create(&mainthread, NULL, &controllocaratteri, NULL);
	pthread_create(&mthread, NULL, &printProc, NULL);
	pthread_join(mthread, NULL);
	pthread_join(mainthread, NULL);

	return 0;
}


//funzione che controlla gli input da tastiera durante l'esecuzione del programma
void* controllocaratteri(void *arg){
    while(!ext){
		char w;
		w = getchar();

		if(w == 'q'){
			fprintf(logger, "(II) Premuto q -- Uscita dal programma al tempo: %s", getTime());
			ext = 1;
		}else if(w < 58 && w >=48){
			fprintf(logger, "(II) Premuto %c -- Aggiornamento ogni %c secondi: %s",w,w, getTime());
			dormi = w-48;
		}else{
			fprintf(logger, "(WW) Nessuna opzione per %c\n",w);
		}
	}
    return NULL;
}

//funzione che stampa i processi
void* printProc(void *arg){
	int i;
	int index = 0;

	struct data totalTimeCpu[LUNGHEZZA_ARRAY];
	struct data totalTimeCpuBefore[LUNGHEZZA_ARRAY];

	struct data user_util[1000];

	float time_total_after,time_total_before;
	
	// inizializzo l'array dei processi
	getArrayUserTime(totalTimeCpuBefore);

	//inizializzo il tempo di cpu totale
	time_total_before = getUserCpu();
	
	while(!ext){

		// prendo il nuovo valore
		getArrayUserTime(totalTimeCpu);

		time_total_after = getUserCpu();
		azzeraArray(user_util,NUMERO_MAX_PROC);
		index =0;

		// con questo for calcola per ogni processo il tempo di cpu
		for (i=0;i<LUNGHEZZA_ARRAY;i++){
			if(totalTimeCpu[i].userTime !=0){
			    //user_util[index].userTime = 100 * (totalTimeCpu[i].userTime - totalTimeCpuBefore[i].userTime) / (time_total_after - time_total_before);
			    user_util[index].userTime = 100 * (totalTimeCpu[i].userTime - totalTimeCpuBefore[i].userTime) / (100 * dormi);
			    user_util[index].pid = i;
			    index++;
			}
		}

		//faccio il sort dell'array in modo da da avere quelli che usano piu cpu in cima all'array
		sortArray(user_util);

		//faccio il clear screen
		printf("\x1B[2J\x1B[1;1H");
		//system("clear");
		int result;

		//Stampo a video i vari dati
		printf("Pid: \tCpu: \tPpid: \tName:\n\n");
		for (i=0;i<num_vis;i++){
			   printf("%d \t%d ", user_util[i].pid,user_util[i].userTime);
			   printInfo(user_util[i].pid);
			   printf("\n");
		}

		copiaArray(totalTimeCpu,totalTimeCpuBefore);
	    time_total_before = time_total_after;

		sleep(dormi);
	}
}

//stampa a video il parent pid e il nome del processo
void printInfo(int pid){
	char* target;
	char *stringa;
	char *ppid;
	ppid = malloc(sizeof(char) * 256);
	stringa = malloc(sizeof(char) * 256);

	target = malloc(snprintf(NULL, 0, "/proc/%d/status",pid) + 1);
	sprintf(target,"/proc/%d/status",pid);

	int i=0;
	FILE *ifile;
	if(ifile = fopen(target, "r")){
		while(fscanf(ifile,"%s",stringa)!=EOF) {
			if(i==1){
				snprintf(ppid,sizeof(char) * 256,"%s",stringa);
				//printf("\t%s ", stringa);
			}
			if(i==10){
				printf("\t%s", stringa);
				fclose(ifile);
			}
		    i++;
		}
	}
	printf("\t%s", ppid);
	free(ppid);
	free(stringa);
	free(target);
	//
}

//Funzione che mette nell'array le informazioni di ogni processo
void getArrayUserTime(struct data * array){
	struct dirent *drnt;
 	struct stat s;
	DIR *dr;
	char* tmp;
	char* target;
	int pid = 0;

	// prima della lettura azzera l'array
	azzeraArray(array,LUNGHEZZA_ARRAY);

	target = malloc(snprintf(NULL, 0, "%s","") + 1);
	tmp = malloc(snprintf(NULL, 0, "%s","") + 1);
	sprintf(target, "%s","/proc/");

	//apre la cartella e guarda tutti i file e cartelle
	dr=opendir(target);
	while(drnt=readdir(dr)) {
		if (strcmp(".", drnt->d_name)==0 || strcmp("..", drnt->d_name)==0)
			continue;

		//le cartelle che hanno come prima lettera un numero vuol dire 
		// che sono le cartelle che contengono le info dei processi
		if(drnt->d_name[0] < 58 && drnt->d_name[0]>=48){
			//creo la stringa contenente il percorso del file stat dento proc/<pid>
			sprintf(tmp, "%s%s%s",target,drnt->d_name,"/stat");
			pid = atoi(drnt->d_name);
			int auux = getProcCpu(tmp);
			struct data aux = {pid,auux};

			// riempio l'array con le info che mi interessan
			array[pid] = aux;
			
		}
	}
	closedir(dr);

	free(target);
	free(tmp);
}

//funzione di suppoto che fa il sort di un array di tipo struct data
void sortArray(struct data * array){
	int i,j;
	struct data temp;
	for(i=0;i<NUMERO_MAX_PROC;i++){
		for(j=i+1;j<NUMERO_MAX_PROC;j++){
			if(array[i].userTime<array[j].userTime){
				temp=array[i];
				array[i]=array[j];
				array[j]=temp;
			}
		}
	}
}

//funzione di supporto che copia l' array1 nell array2
void copiaArray(struct data * array,struct data * array2){
	int i;
	for(i=0;i<LUNGHEZZA_ARRAY;i++){
		array2[i] = array[i];
	}
}

//funzione che ritorna quanto un processo ha occupato la cpu
//per semplicità gli passo il percorso del file dove è contenuta l'informazione
float getProcCpu(char *percorso){
	int i=0;
	int indexProc = 13;
	char stringa[100];
	FILE *ifile;
	ifile = fopen(percorso, "r");
	while(fscanf(ifile,"%s",stringa)!=EOF) {

		//ciclo fino a indexProc che è la posizione del' informazione che mi interessa
		if(i==indexProc){
			fclose(ifile);
			return atof(stringa);
		}
	    i++;
	}
	return 0;
}

//funzione che ritorna un float che è il tempo che è stata utilizzata la cpu in totale
float getUserCpu(){
	char fileProc[100] = "/proc/stat";
	int indexCpu=1;
	char stringa[100];

	// Apro il file /proc/stat che contiene tutte le info della cpu totali
	FILE *sysfile;
	sysfile = fopen(fileProc, "r");

	// due scanf perchè il valore che mi interessa è il secondo dentro il file
	fscanf(sysfile,"%s",stringa);
	fscanf(sysfile,"%s",stringa);
	fclose(sysfile);
	return atof(stringa);
}

//funzione di supporto che azzera un array
void azzeraArray(struct data * array,int n){
	int i;
	for(i=0;i<n;i++){
		struct data aux = {i,0};
		array[i] = aux;
	}
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
    if (logger = fopen("./LogPlive", "r")) {
        fclose(logger);
        retval = 1;
    } else
    	retval = 0;
    logger = fopen("./LogPlive", "a");

    // se non esiste lo crea e ci scrive 2 righe di informazione
    if (retval == 0) {
    	fprintf(logger, "Markers: (!!) notice, (II) informational, (WW) warning, (EE) error\n");
    	fprintf(logger, "(!!) Creazione log %s", getTime());
    }
   	fprintf(logger, "(II) Nuova esecuzione del programma al tempo: %s", getTime()); 
    return retval;
}