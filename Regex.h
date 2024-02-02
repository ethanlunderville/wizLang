#ifndef REGEX_H
#define REGEX_H

struct RegexSizer {
    int low;
    int high;
};

struct wizList;

#define MAX_MATCH_SIZE 1000

int addRegexSize(int low, int high);
void regexOffset(char *string, char *regex);
struct wizList * regexMatch(char *string, char *regex);

#endif