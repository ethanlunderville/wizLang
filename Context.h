#ifndef CONTEXT_H
#define CONTEXT_H

struct wizObject;

#define BASE_SCOPE_SIZE 100

struct IdentifierMap {
    char* identifier;
    struct wizObject * value;
};

struct Context {
    struct Context* cPtr;
    struct IdentifierMap map[BASE_SCOPE_SIZE];
    int currentIndex;
};

struct Context* initContext();
void initGlobalContext();
void printContext();
void printIndent(int spaceNum);
int addToContext(char * identifier, struct wizObject * test);
void* pushScope();
void* popScope(struct wizObject* retVal);
struct wizObject ** getObjectRefFromIdentifier(char * ident);
struct wizObject ** getObjectRefFromIdentifierLocal(char * ident);
struct wizObject ** declareSymbol(char * ident);

#endif