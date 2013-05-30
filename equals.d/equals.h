/* 
 * File:   equals.h
 * anno accademico 2012/2013
 * Progetto #1
 * Giulio Fornasaro     151991
 * Francesco Boschini   151840
 * Luca Zamboni         151759
 */

#ifndef EQUALS_H
#define	EQUALS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <syslog.h>

char* getTime();
int equals(char *, char *);
int confrontafile(char * file1,char *file2);
int recursiveDirectory(char * patha, char * pathb);
int openLog();

#endif	/* EQUALS_H */

