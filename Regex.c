#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "Regex.h"
#include "DataStructures.h"

struct RegexSizer regexSpans[MAX_MATCH_SIZE * sizeof(struct RegexSizer)];
int regexSpansSize = 0;

int addRegexSize(int low, int high) {
    if (regexSpansSize > MAX_MATCH_SIZE)
        return 0;
    regexSpans[regexSpansSize].low = low;
    regexSpans[regexSpansSize].high = high;
    regexSpansSize++;
    return 1;
}

void regexOffset(char *string, char *regex) {
    int stringL = strlen(string);
    int begin, end, offset = 0, actualOffset = 0;                 
    regex_t rgT; 
    regmatch_t match;
    regcomp(&rgT,regex,REG_EXTENDED);
    regexSpansSize = 0;
    memset(regexSpans, '\0', MAX_MATCH_SIZE * sizeof(struct RegexSizer));
    while (offset < stringL) {
        string = string + offset;
        if ((regexec(&rgT, string, 1, &match, 0)) != 0) break;
        begin = (int) match.rm_so;
        end = (int) match.rm_eo;
        if (addRegexSize(actualOffset + begin, actualOffset + end) == 0) break;
        offset = end;
        actualOffset += end; 
    }
    for (int i = 0 ; i < regexSpansSize ; i++)
        printf("%i : %i\n", regexSpans[i].low ,regexSpans[i].high);
    regfree(&rgT);
}

struct wizList * regexMatch(char *string, char *regex) {  
    struct wizList * list = initList(1);
    char * stringWiz;
    int stringL = strlen(string);
    int i, begin, end, len, offset = 0, j = 0;                 
    regex_t rgT; 
    regmatch_t match;
    regcomp(&rgT,regex,REG_EXTENDED);
    while (offset < stringL) {
        string = string + offset;
        if ((regexec(&rgT, string, 1, &match, 0)) != 0)
            break;
        j = 0;
        begin = (int) match.rm_so;
        end = (int) match.rm_eo;
        len = end - begin;
        stringWiz = malloc(len+1);
        stringWiz[len] = '\0';
        for (i = begin; i < end; i++) {
            stringWiz[j] = string[i];
            j++; 
        }
        appendToWizList(
            list, 
            (struct wizObject*)initWizString(stringWiz)
        );
        offset = end;
    }
    regfree(&rgT);
    return list;
}