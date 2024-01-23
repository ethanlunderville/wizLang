#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "Interpreter.h"

struct wizList * initList(int size);
void appendToWizList(struct wizList * list, struct wizObject* element);
void appendToString(struct wizList * string, struct wizObject* charElement);
struct wizDict * initDict(struct wizList * keys, struct wizList * values);

#endif