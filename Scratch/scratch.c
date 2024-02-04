#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#define MAX_MATCH_SIZE 1000

int resultsSize = 0;
char * results[MAX_MATCH_SIZE];

void regexMatch(char *string, char *regex) {  
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
        results[resultsSize] = malloc(len+1);
        memset(results[resultsSize],'\0',len+1);
        for (i = begin; i < end; i++) {
            results[resultsSize][j] = string[i];
            j++; 
        }
        printf("M: %s\n", results[resultsSize]);
        resultsSize++;
        offset = end;
        actualOffset += (end - begin);
    }
    regfree(&rgT);
}

struct RegexSizer {
    int low;
    int high;
};

struct RegexSizer regexSpans[MAX_MATCH_SIZE * sizeof(struct RegexSizer)];
int regexSpansSize = 0;

void addRegexSize(int low, int high) {
    regexSpans[regexSpansSize].low = low;
    regexSpans[regexSpansSize].high = high;
    regexSpansSize++;
}

void regexOffset(char *string, char *regex) {
    regexSpansSize = 0;
    memset(regexSpans, '\0', MAX_MATCH_SIZE * sizeof(struct RegexSizer));
    int stringL = strlen(string);
    int i, begin, end, offset = 0, j = 0, actualOffset = 0;                 
    regex_t rgT; 
    regmatch_t match;
    regcomp(&rgT,regex,REG_EXTENDED);
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

int fib(int n) {
    if (n <2) return n;
    else return fib(n-1) + fib(n-2);
}

int main() {
	printf("%i\n", fib(100));
	return 0;
}