/* 
 * File:   mkbkp.h
 * anno accademico 2012/2013
 * Progetto #1
 * Giulio Fornasaro     151991
 * Francesco Boschini   151840
 * Luca Zamboni         151759
 */

#ifndef MKBKP_H
#define	MKBKP_H

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
#include <sys/syslog.h>

int checkInputs();
void work(int, int, char**);
void createBackup(char*, char*);
int openLog();
char* getTime();
void print(FILE *);
void extractBkp(FILE *);
static void recMkdir(const char *);

#endif	/* MKBKP_H */