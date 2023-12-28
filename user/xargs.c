#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/syscall.h"
#include "user/user.h"
#include "stdbool.h"

#define NULL 0
#define ERROR -1
#define BUFFER_MAX 512

int exec_child(char buf_raw[], int buf_len, char *argv[], int argv_start) {

  char buf[BUFFER_MAX] = {0};
  memcpy(buf, buf_raw, buf_len);
  char *buf_ptr = &buf[0];

  int argv_idx = argv_start;
  while(*buf_ptr != '\0' && *buf_ptr != ' ') {
    char *space = buf_ptr; 
    while(*space != ' ' && *space != '\0') {
      space++;
    }

    int len = space - buf_ptr;
    argv[argv_idx] = malloc(len + 1);
    if (argv[argv_idx] == NULL) {
      fprintf(2, "malloc failed");
      return ERROR;
    }

    memcpy(argv[argv_idx], buf_ptr, len);
    argv[argv_idx][len] = '\0';

    argv_idx++;
    buf_ptr = space+1;
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork failed");
    return ERROR;
  }
  if (pid) {
    for(int i = argv_start; i < argv_idx; i++)  {
      free(argv[i]);
    }
    int ret = wait(0);
    return ret;
  }

  int ret = exec(argv[0], argv);
  for(int i = argv_start; i < argv_idx; i++)  {
    free(argv[i]);
  }

  return ret;
}

int xargs(int argc, char *argv[]) {

  char *child_argv[MAXARG] = {NULL};
  for(int i = 1; i < argc; i++){
    child_argv[i-1] = argv[i];
    memcpy(&child_argv[i-1], &argv[i], strlen(argv[i]));
  }

  char buf[BUFFER_MAX] = {0};
  int buf_idx = 0;
  while(read(0, &buf[buf_idx++], 1) != 0) {
  }

  char *buf_ptr = &buf[0];
  while(*buf_ptr != '\0') {
    char *newline = buf_ptr;
    while(*newline != '\n' && *newline!= '\0') {
      newline++;
    }
    int len = newline - buf_ptr;
    if (exec_child(buf_ptr, len, child_argv, argc-1) < 0) {
      return ERROR;
    }
    buf_ptr = ++newline;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(2, "usage: xargs <command>");
    return ERROR;
  }
  return xargs(argc, argv);
}
