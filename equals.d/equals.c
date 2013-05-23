/* 
 * File:   equals.c
 * Author: forna
 *
 * Created on 23 maggio 2013, 10.55
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


int equals(char *, char *);

int main() {

}

int equals(char * patha, char * pathb) {
	//controllo sul pathing
	//se e' una cartella rilanciare su tutti gli elementi
	//se e' un file vedere se nell'altro path c'e' un file e nel caso confrontarli tra loro
	int retval = 1; //TRUE
	char * tmpa, tmpb;
	struct stat a,b;
	tmpa = malloc(snprintf(NULL, 0, "%s",tmpa) + 1);
	sprintf(tmpa, "%s",patha);
	tmpb = malloc(snprintf(NULL, 0, "%s",tmpb) + 1);
	sprintf(tmpb, "%s",pathb);

	if( stat(tmpa,&a) == 0 && stat(tmpb,&b)) {	
		if( a.st_mode & S_IFREG ) {
			if( s.st_mode & S_IFREG ) {
				//scorrere tutti e due i file e se differiscono stampare i path
				//e scrivere che sono differenti
			} else if( s.st_mode & S_IFDIR ) {
				//stampare che uno dei due e' una cartella
			}	
		} else if( a.st_mode & S_IFDIR ) {
			if( s.st_mode & S_IFREG ) {
				//stampare che uno dei due file e' una cartella
			} else if( s.st_mode & S_IFDIR ) {
				//lanciare ricorsivamente su tutti i sottofile delle due cartelle
			}
		}
	}
}