#include "kernel/types.h"
#include "kernel/syscall.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int n = 0;
  if(argc < 2){
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  n = atoi(argv[1]);

  sleep(n);

  return 0;
}
