#include <stdlib.h>
#include <string.h>
#include "DataStructures.h"
#include "Keywords.h"
#include "Error.h"

struct wizObject* initWizObject(enum Types type) {
    struct wizObject * obj = (struct wizObject *) malloc(sizeof(struct wizObject));
    obj->type = type;
    obj->referenceCount = 0;
}

struct wizList * initList(int size) {
    struct wizList * list = (struct wizList *)malloc(sizeof(struct wizList)); 
    list->size = 0;
    list->capacity = size * 2;
    list->wizV.type = LIST;
    list->wizV.referenceCount = 0;
    list->wizV.value.listVal = malloc(list->capacity * sizeof(struct wizObject*));
    return list;
}

struct wizDict * initDict(struct wizList * keys, struct wizList * values) {
    struct wizDict * dict = (struct wizDict *)malloc(sizeof(struct wizDict));
    dict->wizV.type = DICTIONARY;
    dict->wizV.referenceCount = 0;
    dict->keys = keys;
    dict->values = values;
    incRef((struct wizObject*)keys);
    incRef((struct wizObject*)values);
    return dict;
}

struct wizList * initWizString(char* str) {
    struct wizList * list = (struct wizList *)malloc(sizeof(struct wizList)); 
    list->size = strlen(str);
    list->capacity = strlen(str);
    list->wizV.type = STRINGTYPE;
    list->wizV.referenceCount = 0;
    ((struct wizObject*)list)->value.strValue = str;
    return list;
}

void appendToWizList(struct wizList * list, struct wizObject* element) {
    if (list->size >= list->capacity - 1) {
        list->capacity *= 2;
        list->wizV.value.listVal = realloc(list->wizV.value.listVal, list->capacity * sizeof(struct wizObject*));
    }
    list->wizV.value.listVal[list->size] = element;
    incRef(element);
    list->size++;
}

void appendToString(struct wizList * string, struct wizObject* charElement) {
    if (charElement->type != CHARADDRESS)
        FATAL_ERROR(RUNTIME, -1, "Cannot append a non char type to string");
    string->size++;
    string->wizV.value.strValue = realloc(string->wizV.value.strValue, string->size);
    string->wizV.value.strValue[string->size - 1] = charElement->value.strValue[0];
    string->wizV.value.strValue[string->size] = '\0';
}