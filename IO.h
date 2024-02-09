#ifndef IO_H
#define IO_H

#include <stdio.h>

void initFilePathHolders(char * fPath);
long getFileSize(FILE *file);
void buildPath(char * rPath);
void checkIsDir(char * fileName);
char* fileToBuffer(char * fileName);

#endif