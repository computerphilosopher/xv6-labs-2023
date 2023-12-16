#include "kernel/types.h"
#include "kernel/syscall.h"
#include "user/user.h"
#include "stdbool.h"

#define LIMIT 35
#define BUFFER_MAX 10
#define READ_FD 0
#define WRITE_FD 1
#define ERROR -2

int read_left(int *left_pipe) {
  char buf[BUFFER_MAX] = {0};
  int bytes = read(left_pipe[READ_FD], buf, 1);
  if (bytes <= 0) {
    return ERROR;
  }
  return buf[0];
}

int filter_primes(int *left_pipe) {

  close(left_pipe[WRITE_FD]);
  int right_pipe[2] = {0};
  if (pipe(right_pipe) != 0) {
    close(left_pipe[READ_FD]);
    fprintf(2, "piping right is failed");
    return ERROR;
  }

  const int prime = read_left(left_pipe);
  if (prime == ERROR) {
    close(left_pipe[READ_FD]);
    close(right_pipe[READ_FD]);
    close(right_pipe[WRITE_FD]);
    return ERROR;
  }
  const int EOF= -1;
  bool forked = false;

  fprintf(1, "prime %d\n", prime);
  int ret = 0;
  while (true) {
    int prime_or_not = read_left(left_pipe);
    if (prime_or_not == ERROR) {
      ret = ERROR;
      break;
    }
    if (prime_or_not >= LIMIT) {
      ret = write(right_pipe[WRITE_FD], &EOF, -1);
      break;
    }
    if (prime_or_not == EOF) {
      break;
    }
    if (prime_or_not % prime == 0) {
      continue;
    }
    if (!forked) {
      forked = true;
      int pid = fork();
      if (pid < 0) {
        ret = pid;
        break;
      }
      if (pid == 0) {
        ret = filter_primes(right_pipe);
        break;
      }
      close(right_pipe[READ_FD]);
    }

    int bytes = write(right_pipe[WRITE_FD], &prime_or_not, 1);
    if (bytes < 0) {
      ret = bytes;
      break;
    }
  }

  close(left_pipe[READ_FD]);
  close(right_pipe[READ_FD]);
  close(right_pipe[WRITE_FD]);

  return ret + wait(0);
}

int main(int argc, char *argv[]) {
  int left_pipe[2] = {0};
  if (pipe(left_pipe) != 0) {
    //fprintf(2, "init pipe is failed");
    return ERROR;
  }

  int pid = fork();
  if (pid < 0) {
    return ERROR;
  }
  if (pid > 0) {
    close(left_pipe[READ_FD]);
    for(int i=2; i <= LIMIT; i++) {
      write(left_pipe[WRITE_FD], &i, 1);
    }
    return close(left_pipe[WRITE_FD]) + wait(0);
  }

  return filter_primes(left_pipe);
}
