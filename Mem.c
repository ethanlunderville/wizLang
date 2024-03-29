#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "Keywords.h"
#include "Mem.h"

void* reallocate(void * pointer, long newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }
    return realloc(pointer, newSize);
}

char * copyStr(char * str) {
    int len = strlen(str);
    char * copy = malloc(len + 1);
    copy[len] = '\0';
    strncpy(copy, str, len);
    return copy;
}

char * genStr(int size) {
    char * copy = malloc(size+1);
    memset(copy,'#',size-1);
    copy[size] = '\0';
    return copy;
}