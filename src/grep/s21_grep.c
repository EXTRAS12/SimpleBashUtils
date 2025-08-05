#include "s21_grep.h"

int main(int argc, char** argv) {
  flags f = {0};
  char* fileArray[SIZE] = {0};
  char* patternArray[SIZE] = {0};
  bool usedArgs[SIZE] = {false};
  int patternCount = 0;
  int fileCount = 0;

  checkFlagsAndPatterns(argc, argv, &f, patternArray, usedArgs, &patternCount);
  collectFiles(argc, argv, usedArgs, patternArray, fileArray, &fileCount,
               &patternCount, &f);

  grep(&f, fileCount, patternCount, fileArray, patternArray);

  for (int i = 0; i < patternCount; i++) {
    if (patternArray[i]) {
      free(patternArray[i]);
      patternArray[i] = NULL;
    }
  }
  return 0;
}

int checkFlagsAndPatterns(int argc, char** argv, flags* f,
                          char* patternArray[SIZE], bool usedArgs[SIZE],
                          int* patternCount) {
  int status = 0;
  bool exit = false;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      exit = false;
      for (int j = 1; j < (int)strlen(argv[i]) && !exit; j++) {
        if (argv[i][j] == 'e') {
          f->e = true;
          if (j < (int)strlen(argv[i]) - 1) {
            char* pattern = argv[i] + j + 1;
            patternArray[*patternCount] = my_strdup(pattern);
            if (!patternArray[*patternCount]) {
              fprintf(stderr, "Ошибка выделения памяти\n");
              status = 1;
              exit = true;
            } else {
              (*patternCount)++;
            }
          } else if (i + 1 < argc) {
            patternArray[*patternCount] = my_strdup(argv[i + 1]);
            if (!patternArray[*patternCount]) {
              fprintf(stderr, "Ошибка выделения памяти\n");
              status = 1;
              exit = true;
            } else {
              (*patternCount)++;
              usedArgs[i + 1] = true;
              i++;
            }
          }
          exit = true;
        } else if (argv[i][j] == 'f') {
          bonusTask(argc, argv, i, &f, patternArray, usedArgs, patternCount);
          i++;
          exit = true;
        } else {
          if (!setFlag(argv[i][j], f)) {
            fprintf(stderr, "Неизвестный флаг: -%c\n", argv[i][j]);
            status = 1;
          }
        }
      }
      usedArgs[i] = true;
    }
  }

  return status;
}

int bonusTask(int argc, char** argv, int i, flags** f, char* patternArray[SIZE],
              bool usedArgs[SIZE], int* patternCount) {
  int status = 0;
  FILE* file = NULL;
  if (i + 1 >= argc) {
    fprintf(stderr, "grep: Не указан файл после -f\n");
    status = 1;
  }

  if (status == 0) {
    file = fopen(argv[i + 1], "r");
    if (!file) {
      fprintf(stderr, "grep: Не удается открыть файл: %s\n", argv[i + 1]);
      status = 1;
    }
  }

  if (status == 0) {
    char line[1024];
    while (fgets(line, sizeof(line), file) && status == 0) {
      size_t len = strlen(line);
      if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

      if (*patternCount >= SIZE) {
        fprintf(stderr, "Достигнуто максимальное количество паттернов.\n");
        status = 1;
      }

      if (status == 0) {
        patternArray[*patternCount] = my_strdup(line);
        if (!patternArray[*patternCount]) {
          fprintf(stderr, "Ошибка выделения памяти\n");
          status = 1;
        } else {
          (*patternCount)++;
        }
      }
    }
  }

  if (file) {
    fclose(file);
  }

  if (status == 0) {
    usedArgs[i + 1] = true;
    (*f)->f = true;
  }

  return status;
}

int collectFiles(int argc, char** argv, bool usedArgs[SIZE],
                 char* patternArray[SIZE], char* fileArray[SIZE],
                 int* fileCount, int* patternCount, flags* f) {
  int status = 0;
  int i = 1;

  while (i < argc && status == 0) {
    if (!usedArgs[i] && argv[i][0] != '-') {
      if (*patternCount == 0 && !f->e && !f->f) {
        patternArray[*patternCount] = my_strdup(argv[i]);

        if (patternArray[*patternCount] == NULL) {
          fprintf(stderr, "Ошибка выделения памяти\n");
          status = 1;
        } else {
          (*patternCount)++;
          usedArgs[i] = true;
        }
      } else {
        fileArray[(*fileCount)++] = argv[i];
      }
    }
    i++;
  }

  return status;
}

char* my_strdup(const char* s) {
  if (!s) return NULL;
  char* p = malloc(strlen(s) + 1);
  if (p) strcpy(p, s);
  return p;
}

int compilePatterns(flags* f, int patternCount, char* patternArray[SIZE],
                    regex_t* regex) {
  int status = 0;
  int iflag = f->i ? REG_ICASE : 0;
  for (int i = 0; i < patternCount; i++) {
    if (regcomp(&regex[i], patternArray[i], iflag) != 0) {
      if (!f->s)
        fprintf(stderr, "Ошибка в регулярном выражении: %s\n", patternArray[i]);
      status = 1;
    }
  }
  return status;
}

void processFile(flags* f, char* fileName, int patternCount, regex_t* regex,
                 int fileCount) {
  FILE* file = fopen(fileName, "r");
  if (!file) {
    if (!f->s) fprintf(stderr, "grep: %s: Нет такого файла\n", fileName);
    return;
  }

  char line[1024];
  int lineNum = 0;
  int matchCount = 0;
  bool fileHasMatch = false;

  while (fgets(line, sizeof(line), file)) {
    lineNum++;
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

    bool matched = matchLine(f, line, patternCount, regex);

    if (matched) {
      matchCount++;
      fileHasMatch = true;

      if (!f->c && !f->l) {
        if (f->o) {
          for (int patIdx = 0; patIdx < patternCount; patIdx++) {
            printMatches(&regex[patIdx], line, f, fileName, lineNum,
                         fileCount > 1);
          }
        } else {
          displayResult(f, fileCount > 1, fileName, line, lineNum);
        }
      }
    }
  }

  if (f->c && !f->l) {
    if (fileCount > 1 && !f->h) printf("%s:", fileName);
    printf("%d\n", f->l ? (fileHasMatch ? 1 : 0) : matchCount);
  }

  if (f->l && fileHasMatch) {
    printf("%s\n", fileName);
  }

  fclose(file);
}

bool matchLine(flags* f, char* line, int patternCount, regex_t* regex) {
  bool matched = false;
  for (int patIdx = 0; patIdx < patternCount; patIdx++) {
    int result = regexec(&regex[patIdx], line, 0, NULL, 0);
    int condition = (result == 0 && !f->v) || (result == REG_NOMATCH && f->v);

    if (condition) {
      matched = true;
      if (f->o) break;
    }
  }
  return matched;
}

int grep(flags* f, int fileCount, int patternCount, char* fileArray[SIZE],
         char* patternArray[SIZE]) {
  int status = 0;

  regex_t* regex = malloc(patternCount * sizeof(regex_t));

  status = compilePatterns(f, patternCount, patternArray, regex);
  if (status != 0) {
    free(regex);
    return status;
  }

  for (int fileIdx = 0; fileIdx < fileCount; fileIdx++) {
    processFile(f, fileArray[fileIdx], patternCount, regex, fileCount);
  }

  for (int i = 0; i < patternCount; i++) {
    regfree(&regex[i]);
  }
  free(regex);

  return status;
}

void displayResult(flags* f, bool multipleFiles, const char* filename,
                   const char* line, int lineNum) {
  if (f->l) return;

  if (multipleFiles && !f->h) printf("%s:", filename);
  if (f->n) printf("%d:", lineNum);
  printf("%s\n", line);
}

void printMatches(regex_t* regex, const char* line, flags* f,
                  const char* filename, int lineNum, bool multipleFiles) {
  regmatch_t pmatch;
  bool match_found = false;

  if (regexec(regex, line, 1, &pmatch, 0) == 0) {
    match_found = true;
  }

  if (f->v) {
    match_found = !match_found;
  }

  if (match_found) {
    if (!f->o) {
      if (multipleFiles && !f->h) printf("%s:", filename);
      if (f->n) printf("%d:", lineNum);
      printf("%s\n", line);
    } else {
      const char* ptr = line;
      while (regexec(regex, ptr, 1, &pmatch, 0) == 0) {
        if (pmatch.rm_so == -1) break;

        if (multipleFiles && !f->h) printf("%s:", filename);
        if (f->n) printf("%d:", lineNum);

        printf("%.*s\n", (int)(pmatch.rm_eo - pmatch.rm_so),
               ptr + pmatch.rm_so);
        ptr += pmatch.rm_eo;
      }
    }
  }
}

bool setFlag(char flag, flags* f) {
  switch (flag) {
    case 'e':
      f->e = true;
      break;
    case 'i':
      f->i = true;
      break;
    case 'v':
      f->v = true;
      break;
    case 'c':
      f->c = true;
      break;
    case 'l':
      f->l = true;
      break;
    case 'n':
      f->n = true;
      break;
    case 'h':
      f->h = true;
      break;
    case 's':
      f->s = true;
      break;
    case 'f':
      f->f = true;
      break;
    case 'o':
      f->o = true;
      break;
    default:
      return false;
  }
  return true;
}
