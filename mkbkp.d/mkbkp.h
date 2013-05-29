/* 
 * File:   mkbkp.h
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

int checkInputs();
void work(int, int, char**);
void createBackup(char*, char*);
int openLog();
char* getTime();
void print(FILE *);
void extractBkp(FILE *);
static void recMkdir(const char *);

#endif	/* MKBKP_H */