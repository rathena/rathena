#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strlib.h"

//-----------------------------------------------
// string lib.
unsigned char* jstrescape (unsigned char* pt) {
	//copy from here
	unsigned char * ptr;
	int i =0, j=0;
	
	//copy string to temporary
	ptr = malloc(J_MAX_MALLOC_SIZE);
	strcpy (ptr,pt);
	
	while (ptr[i] != '\0') {
		switch (ptr[i]) {
			case '\'':
				pt[j++] = '\\';
				pt[j++] = ptr[i++];
				break;
			default:
				pt[j++] = ptr[i++];
		}
	}
	pt[j++] = '\0';
	free (ptr);
	return (unsigned char*) &pt[0];
}

unsigned char* jstrescapecpy (unsigned char* pt,unsigned char* spt) {
	//copy from here
	int i =0, j=0;
	
	while (spt[i] != '\0') {
		switch (spt[i]) {
			case '\'':
				pt[j++] = '\\';
				pt[j++] = spt[i++];
				break;
			default:
				pt[j++] = spt[i++];
		}
	}
	pt[j++] = '\0';
	return (unsigned char*) &pt[0];
}
int jmemescapecpy (unsigned char* pt,unsigned char* spt, int size) {
	//copy from here
	int i =0, j=0;
	
	while (i < size) {
		switch (spt[i]) {
			case '\'':
				pt[j++] = '\\';
				pt[j++] = spt[i++];
				break;
			default:
				pt[j++] = spt[i++];
		}
	}
	// copy size is 0 ~ (j-1)
	return j;
}
