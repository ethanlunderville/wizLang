#ifndef CONTEXT_H
#define CONTEXT_H

struct wizObject;

#define BASE_SCOPE_SIZE 100

enum ContextType { CONDITIONAL , FUNCTION , GLOBAL};

struct IdentifierMap {
    char* identifier;
    struct wizObject * value;
};

struct Context {
    struct Context* cPtr;
    struct IdentifierMap map[BASE_SCOPE_SIZE];
    int currentIndex;
    enum ContextType type;
};

struct Context* initContext(enum ContextType type);
void printContext();
int addToContext(char * identifier, struct wizObject * test);
void* pushConditionalScope();
void* pushFunctionScope();
void* popScope();
struct wizObject ** getObjectRefFromIdentifier(char * ident);
struct wizObject ** getObjectRefFromIdentifierLocal(char * ident);
struct wizObject ** declareSymbol(char * ident);

#endif