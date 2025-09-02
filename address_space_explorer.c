#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int pid = (int)getpid();
  printf("PID: %d\n", pid);
  printf("Address of Main Code: %p\n", main);
  printf("Address of Main Code: %px\n", main);

  char *hp = (char *)malloc(10);
  printf("Address of heap: %p\n", hp);

  int x = 10;
  printf("Address of stack: %p\n", &x);

  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "Fork has failed.");
    return 1;
  } else if (rc > 0) {
    wait(NULL);
  } else {
    // child process
    char pid_str[16];
    int child_pid = (int)getpid();
    sprintf(pid_str, "%d", pid);
    char *args[] = {"pmap", pid_str, NULL};
    execvp("vmmap", args);
    fprintf(stderr, "Syscall failed.");
    exit(1);
  }
  return 0;
}
