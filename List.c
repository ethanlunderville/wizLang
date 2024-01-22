#include <stdlib.h>
#include "DataStructures.h"
#include "Keywords.h"
#include "Error.h"

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
    incRef(element);
    list->size++;
}

void appendToString(struct wizList * string, struct wizObject* charElement) {
    if (string->size >= string->capacity - 1) {
        string->capacity *= 2;
        string->wizV.value.strValue = realloc(string, string->capacity);
    }
    if (charElement->type != CHARADDRESS)
        FATAL_ERROR(RUNTIME, -1, "Cannot append a non char type to string");
    string->wizV.value.strValue[string->size] = charElement->value.strValue[0];
    string->size++;
}