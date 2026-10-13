#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

// Return a pointer to the last path component ("a/b/c" -> "c").
char*
basename(char *path)
{
  char *p = path + strlen(path);
  while(p > path && *(p - 1) != '/')
    p--;
  return p;
}

// What to do with a matching path: print it, or exec "cmd ... path".
void
action(char *path, char **cmd)
{
  if(cmd == 0){
    printf("%s\n", path);
    return;
  }
  if(fork() == 0){
    char *argv[MAXARG];
    int i = 0;
    for(; cmd[i] != 0 && i < MAXARG - 2; i++)
      argv[i] = cmd[i];
    argv[i++] = path;   // append the found file
    argv[i] = 0;
    exec(argv[0], argv);
    fprintf(2, "find: exec %s failed\n", argv[0]);
    exit(1);
  }
  wait(0);
}

void
find(char *path, char *name, char **cmd)
{
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512], *p;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // match the current node (files and directories both)
  if(strcmp(basename(path), name) == 0)
    action(path, cmd);

  if(st.type == T_DIR){
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
      fprintf(2, "find: path too long\n");
      close(fd);
      return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, name, cmd);   // recurse
    }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find path name [-exec cmd ...]\n");
    exit(1);
  }

  char *path = argv[1];
  char *name = argv[2];
  char **cmd = 0;

  for(int i = 3; i < argc; i++){
    if(strcmp(argv[i], "-exec") == 0){
      if(i + 1 >= argc){
        fprintf(2, "find: -exec requires a command\n");
        exit(1);
      }
      cmd = &argv[i + 1];   // argv is null-terminated at argc, so this is a valid argv
      break;
    }
  }

  find(path, name, cmd);
  exit(0);
}
