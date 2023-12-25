#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "user/user.h"
#include "stdbool.h"

#define ARG_PATH 1
#define ARG_NAME 2
#define ERROR -1
#define BUFFER_MAX 512

void concat_path(char *path, const char *tail) {
  //fprintf(1, "path: %s tail: %s\n", path, tail);
  path[strlen(path)] = '/';
  strcpy(&path[strlen(path)], tail);
}

int get_file_type(int fd) {
  struct stat st;
  if(fstat(fd, &st) < 0){
    return ERROR;
  }

  return st.type;
}

bool name_matched(char *path, char *name) {
  int last_slash = strlen(path)-1;
  while(last_slash >= 0) {
    if (path[last_slash] == '/') {
      break;
    }
    last_slash--;
  }

  return strcmp(&path[last_slash+1], name) == 0;
}

int find(char *path, char *name) {

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return ERROR;
  }

  if (name_matched(path, name)) {
    fprintf(1, "%s\n", path);
  }

  int file_type = get_file_type(fd);
  if (file_type == ERROR) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return ERROR;
  }

  if (file_type != T_DIR) {
    close(fd);
    return 0;
  }

  struct dirent de;
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if (strcmp(de.name, ".") == 0 || 
      strcmp(de.name, "..") == 0 ||
      strlen(de.name) == 0 )  {
      continue;
    }

    char child_path[BUFFER_MAX] = {0};
    strcpy(child_path, path);
    concat_path(child_path, de.name);

    if (find(child_path, name) == ERROR){
      close(fd);
      return ERROR;
    }
  }

  close(fd);
  return 0;
}

int main(int argc, char *argv[]) {

  if(argc != 3){
    fprintf(2, "usage: find <path> <name>\n");
    return -1;
  }

  return find(argv[ARG_PATH], argv[ARG_NAME]);
}
