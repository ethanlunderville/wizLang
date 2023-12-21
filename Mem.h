/*

    Filename: Mem.h

    Description:

    Code for various random memory related things

*/

#ifndef MEM_H
#define MEM_H

#include "Keywords.h"

// General code for growing arrays since C does not have dynaic arrays.

void* reallocate(void ** pointer, long newSize) {
    if (*pointer == NULL){
        *pointer = malloc(newSize);
        return *pointer;
    }
    if (newSize == 0) {
        free(*pointer);
        return NULL;
    }
    return realloc(*pointer, newSize);
}

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)
#define GROW_ARRAY(type, pointer, newCount) (type*)reallocate(pointer, sizeof(type) * (newCount))

#endif