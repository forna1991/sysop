#ifndef PLIVE_H
#define PLIVE_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

/**
La struttura data serve per memorizzarmi il pid di un singolo proecsso e il tempo che 
il processo ha occupato il provcessore
*/
struct data
{
	int pid;
	int userTime;
};

int openLog();
char* getTime();
void* printProc();
float getUserCpu();
float getProcCpu(char *pid);
void azzeraArray(struct data * array,int n);
void getArrayUserTime(struct data * array);
void copiaArray(struct data * array,struct data * array2);
void sortArray(struct data * array);
void printInfo(int pid);
void* controllocaratteri(void *arg);

#endif /* PLIVE_H */