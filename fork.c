#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  printf("Start Point : %d\n", (int)getpid());
  int rc = fork();

  if (rc < 0) {
    fprintf(stderr, "fork creation failed\n");
  } else if (rc == 0) {
    printf("child process : %d\n", (int)getpid());
  } else {
    int rc_wait = wait(NULL);
    printf("parent of %d (rc_wait:%d) (pid:%d)\n", rc, rc_wait, (int)getpid());
  }
  return 0;
}
