#include "s21_cat.h"

int main(int argc, char* argv[]) {
  flags f = {0};
  char* files[MAX_FILES];
  int fileCount = 0;

  if (!checkFlags(argc, argv, &f, files, &fileCount)) return 0;
  cat(files, fileCount, &f);
  return 0;
}

bool checkLongFlag(const char* s, flags* f) {
  if (strcmp(s, "number-nonblank") == 0)
    f->b = true;
  else if (strcmp(s, "number") == 0)
    f->n = true;
  else if (strcmp(s, "squeeze-blank") == 0)
    f->s = true;
  else
    return false;
  return true;
}

bool setFlag(char flag, flags* f) {
  switch (flag) {
    case 'b':
      f->b = true;
      f->n = false;
      break;
    case 'e':
      f->e = true;
      f->v = true;
      break;
    case 'v':
      f->v = true;
      break;
    case 'E':
      f->e = true;
      f->v = false;
      break;
    case 'n':
      if (!f->b) f->n = true;
      break;
    case 's':
      f->s = true;
      break;
    case 't':
      f->t = true;
      f->v = true;
      break;
    case 'T':
      f->t = true;
      f->v = false;
      break;
    default:
      return false;
  }
  return true;
}

bool checkFlags(int argc, char** argv, flags* f, char* files[],
                int* fileCount) {
  bool result = true;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == '-') {
        if (!checkLongFlag(&argv[i][2], f)) {
          fprintf(stderr, "Некорректный флаг: %s\n", argv[i]);
          result = false;
        }
      } else {
        for (char* ptr = &argv[i][1]; *ptr; ptr++) {
          if (!setFlag(*ptr, f)) {
            fprintf(stderr, "Некорректный флаг: -%c\n", *ptr);
            result = false;
          }
        }
      }
    } else {
      if (*fileCount < MAX_FILES) {
        files[*fileCount] = argv[i];
        (*fileCount)++;
      }
    }
  }
  return result;
}

void processLine(char* buff, int* currentLine, flags* f, bool* previousLineVoid,
                 int* nonEmptyLineCount) {
  bool voidLine = (buff[0] == '\n' || buff[0] == '\0');
  bool suppressLine = false;

  if (f->s && *previousLineVoid && voidLine) {
    suppressLine = true;
  }
  if (!suppressLine) {
    if (f->b && !voidLine) {
      (*nonEmptyLineCount)++;
      printf("%6d\t", *nonEmptyLineCount);
    } else if (f->n && !f->b) {
      printf("%6d\t", *currentLine);
    }
  }

  if (!suppressLine) {
    size_t len = strlen(buff);
    bool hasNewline = (len > 0 && buff[len - 1] == '\n');
    size_t end;
    if (hasNewline) {
      end = len - 1;
    } else {
      end = len;
    }

    for (size_t i = 0; i < end; i++) {
      unsigned char c = (unsigned char)buff[i];

      if (c == '\t') {
        if (f->t) {
          printf("^I");
        } else {
          putchar(c);
        }
      } else if (f->v && (c < 32 || c == 127)) {
        if (c == 0x00) {
          printf("^@");
        } else {
          printf("^%c", (c < 32) ? c + 64 : '?');
        }

      } else {
        putchar(c);
      }
    }
    if (hasNewline) {
      if (f->e) printf("$");
      putchar('\n');
    }
  }
  if (!suppressLine) {
    (*currentLine)++;
    *previousLineVoid = voidLine;
  } else {
    *previousLineVoid = true;
  }
}

void processFile(const char* path, flags* f) {
  FILE* file = fopen(path, "r");
  if (!file) {
    fprintf(stderr, "Ошибка открытия файла: %s\n", path);
    return;
  }

  char buff[2048];
  int currentLine = 1;
  int nonEmptyLineCount = 0;
  bool previousLineVoid = false;

  while (fgets(buff, sizeof(buff), file)) {
    processLine(buff, &currentLine, f, &previousLineVoid, &nonEmptyLineCount);
  }

  fclose(file);
}

void cat(char* files[], int fileCount, flags* f) {
  for (int i = 0; i < fileCount; i++) {
    processFile(files[i], f);
  }
}