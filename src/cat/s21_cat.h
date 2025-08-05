#ifndef S21_CAT_H_
#define S21_CAT_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 100

typedef struct flags {
  bool b, e, n, s, t, v;
} flags;

bool checkLongFlag(const char* s, flags* f);
bool setFlag(char flag, flags* f);
bool checkFlags(int argc, char** argv, flags* f, char* files[], int* fileCount);
void processLine(char* buff, int* currentLine, flags* f, bool* previousLineVoid,
                 int* nonEmptyLineCount);
void processFile(const char* path, flags* f);
void cat(char* files[], int fileCount, flags* f);

#endif