/* 
 * File:   equals.h
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

int openLog();
char* getTime();
int equals(char *, char *);
int confrontafile(char * file1,char *file2);
int recursiveDirectory(char * patha, char * pathb);

#endif	/* EQUALS_H */

