#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "Interpreter.h"

struct wizList * initList(int size);
void appendToWizList(struct wizList * list, struct wizObject* element);

#endif