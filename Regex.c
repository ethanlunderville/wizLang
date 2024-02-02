#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "Regex.h"
#include "DataStructures.h"

struct RegexSizer regexSpans[MAX_MATCH_SIZE * sizeof(struct RegexSizer)];
int regexSpansSize = 0;

void addRegexSize(int low, int high) {
    regexSpans[regexSpansSize].low = low;
    regexSpans[regexSpansSize].high = high;
    regexSpansSize++;
}

void regexOffset(char *string, char *regex) {
    int stringL = strlen(string);
    int i, begin, end, offset = 0, j = 0, actualOffset = 0;                 
    regex_t rgT; 
    regmatch_t match;
    regcomp(&rgT,regex,REG_EXTENDED);
    regexSpansSize = 0;
    memset(regexSpans, '\0', MAX_MATCH_SIZE * sizeof(struct RegexSizer));
    while (offset < stringL) {
        string = string + offset;
        if ((regexec(&rgT, string, 1, &match, 0)) != 0)
            break;
        j = 0;
        begin = (int) match.rm_so;
        printf("B: %i ", actualOffset + begin);
        end = (int) match.rm_eo;
        printf("E: %i \n", actualOffset + end);
        addRegexSize(begin, end);
        offset = end;
        actualOffset += (end - begin); 
    }
    regfree(&rgT);
}

struct wizList * regexMatch(char *string, char *regex) {  
    struct wizList * list = initList(1);
    char * stringWiz;
    int stringL = strlen(string);
    int i, begin, end, len, offset = 0, j = 0, actualOffset = 0;                 
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
        appendToWizList(list, (struct wizObject*)initWizString(stringWiz));
        offset = end;
        actualOffset += (end - begin);
    }
    regfree(&rgT);
    return list;
}