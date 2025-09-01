#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLEAN_TERMINAL printf("\033[H\033[J");
#define MAX_ARGS 60
#define MAX_INPUT 1024
#define EXIT "exit"
#define CLR "clear"

size_t cmp(char *a, char *b) {
  size_t idx = 0;
  while (a[idx] != '\0' && b[idx] != '\0') {
    if (a[idx] != b[idx]) {
      return 0;
    }
    idx++;
  }
  return (a[idx] == '\0' && b[idx] == '\0');
}

int main() {
  CLEAN_TERMINAL;

  char *input = (char *)malloc(MAX_INPUT * sizeof(char));
  char *args[MAX_ARGS];

  if (input == NULL) {
    fprintf(stderr, "Input memory allocation failed.\n");
    return 1;
  }

  while (1) {
    printf("mini-shell> ");
    fflush(stdout);
    if (fgets(input, MAX_INPUT, stdin) == NULL) {
      fprintf(stderr, "Error is reading input command.\n");
    }

    size_t len = 0;
    while (input[len] != '\0' && input[len] != '\n') {
      len++;
    }
    input[len] = '\0';

    if (cmp(input, EXIT) == 1) {
      break;
    }
    if (cmp(input, CLR) == 1) {
      CLEAN_TERMINAL;
      fflush(stdout);
    }
    size_t arg_count = 0;
    size_t curr_start = 0;
    for (size_t i = 0; i <= len; ++i) {
      if (input[i] == ' ') {
        size_t j = i;
        while (input[i + 1] == ' ' && i < len) {
          i++;
        }
        args[arg_count] = &input[curr_start];
        arg_count++;
        input[j] = '\0';
        curr_start = i + 1;
      } else if (input[i] == '\0') {
        if (curr_start < i) {
          args[arg_count] = &input[curr_start];
          arg_count++;
        }
      }
    }
    args[arg_count] = NULL;
    int rc = fork();
    if (rc < 0) {
      fprintf(stderr, "Forking child process failed.\n");
      return 1;
    } else if (rc == 0) {
      execvp(args[0], args);
      fprintf(stderr, "Command execution failed.\n");
    } else {
      int rc_wait = wait(NULL);
    }
  }

  free(input);
}
