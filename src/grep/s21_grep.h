#ifndef S21_GREP_H_
#define S21_GREP_H_

#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024

typedef struct flags {
  bool e, n, s, t, v, i, c, l, f, h, o;
} flags;

bool setFlag(char flag, flags* f);
int grep(flags* f, int fileCount, int patternCount, char* fileArray[SIZE],
         char* patternArray[SIZE]);
void displayResult(flags* f, bool multipleFiles, const char* filename,
                   const char* line, int lineNum);
int checkFlagsAndPatterns(int argc, char** argv, flags* f,
                          char* patternArray[SIZE], bool usedArgs[SIZE],
                          int* patternCount);
int collectFiles(int argc, char** argv, bool usedArgs[SIZE],
                 char* patternArray[SIZE], char* fileArray[SIZE],
                 int* fileCount, int* patternCount, flags* f);
void printMatches(regex_t* regex, const char* line, flags* f,
                  const char* filename, int lineNum, bool multipleFiles);
int bonusTask(int argc, char** argv, int i, flags** f, char* patternArray[SIZE],
              bool usedArgs[SIZE], int* patternCount);
char* my_strdup(const char* s);
int compilePatterns(flags* f, int patternCount, char* patternArray[SIZE],
                    regex_t* regex);
bool matchLine(flags* f, char* line, int patternCount, regex_t* regex);
void processFile(flags* f, char* fileName, int patternCount, regex_t* regex,
                 int fileCount);

#endif  // S21_GREP_H_