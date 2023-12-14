#include "kernel/types.h"
#include "kernel/syscall.h"
#include "user/user.h"

#define READ_FD 0
#define WRITE_FD 1

int main(int argc, char *argv[]) {
  int pipe_fd[2] = {0};
  pipe(pipe_fd);

  int pid = fork();

  //parent
  if (pid > 0) {
    write(pipe_fd[WRITE_FD], "ping", 4);

    char buf[100] = {0};
    read(pipe_fd[READ_FD], buf, 4);
    fprintf(2, "%d: received pong\n", getpid());

    return 0;
  } 

  //child
  if (pid == 0) {
    char buf[100] = {0};
    read(pipe_fd[READ_FD], buf, 4);
    fprintf(2, "%d: received ping\n", getpid());

    write(pipe_fd[WRITE_FD], "pong", 4);
    return 0;
  } 

  fprintf(2, "fork failed\n");
  return 1;
}
