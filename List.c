#include <stdlib.h>
#include "DataStructures.h"
#include "Keywords.h"

struct wizList * initList(int size) {
    struct wizList * list = (struct wizList *)malloc(sizeof(struct wizList)); 
    list->size = 0;
    list->capacity = size * 2;
    list->wizV.type = LIST;
    list->wizV.referenceCount = 0;
    list->wizV.value.listVal = malloc(list->capacity * sizeof(struct wizObject*));
    return list;
}


void appendToWizList(struct wizList * list, struct wizObject* element) {
    if (list->size >= list->capacity - 1) {
        list->capacity *= 2;
        list->wizV.value.listVal = realloc(list, list->capacity * sizeof(struct wizObject));
    }
    list->wizV.value.listVal[list->size] = element;
    if (element->referenceCount != -1)
        element->referenceCount++;
    list->size++;
}
