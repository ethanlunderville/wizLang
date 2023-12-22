/*

    Filename: Mem.h

    Description:

    Code for various random memory related things

*/

#ifndef MEM_H
#define MEM_H

#include "Keywords.h"

// General code for growing arrays since C does not have dynamic arrays.

void* reallocate(void * pointer, long newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }
    return realloc(pointer, newSize);
}
#define BASE_CAPACITY 8
#define GROW_CAPACITY(capacity) ((capacity) < BASE_CAPACITY ? BASE_CAPACITY : (capacity) * 2)
#define INIT_ARRAY(type) (type*)malloc(sizeof(type) * BASE_CAPACITY)
#define GROW_ARRAY(type, pointer, newCount) (type*)reallocate(pointer, sizeof(type) * (newCount))

#endif