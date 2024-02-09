#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Error.h"
#include "IO.h"

#ifdef _WIN32
#define FILE_SEPARATOR '\\'
#else
#define FILE_SEPARATOR '/'
#include <sys/stat.h>
#endif

extern char currentPath[PATH_MAX];
extern int basePathSize;

void initFilePathHolders(char * fPath) {
    memset(currentPath,'\0', PATH_MAX);
    basePathSize = strlen(fPath);
    if (basePathSize > PATH_MAX)
        FATAL_ERROR(IO,-1,"File path of provided source file is too large");
    memcpy(currentPath, fPath, basePathSize);
    if (fPath[basePathSize-1] == FILE_SEPARATOR) 
        return;
    for (int i = basePathSize - 1 ; i > -1 ; i--) {
        if (fPath[i] == FILE_SEPARATOR){
            memset(currentPath + i + 1,'\0', basePathSize - ((i + currentPath) - currentPath) - 1);
            basePathSize = i+1;
            return; 
        }
    }
    currentPath[0] = '.';
    currentPath[1] = FILE_SEPARATOR;
    basePathSize = 2;
}

long getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

void buildPath(char * rPath) {
    int rPathSize = strlen(rPath);
    memset(currentPath + basePathSize, '\0' , PATH_MAX - basePathSize);
    if (basePathSize + rPathSize > PATH_MAX)
        FATAL_ERROR(IO, -1, "Internal error file path string was too long when appended to base path");
    memcpy(currentPath + basePathSize, rPath, rPathSize);
}

void checkIsDir(char * fileName) {
    struct stat path_stat;
    if (stat(fileName, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
        FATAL_ERROR(IO, -1, "Attempted to open directory: %s as a file", fileName);
}

