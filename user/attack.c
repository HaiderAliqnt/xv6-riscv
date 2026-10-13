#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  char *end = sbrk(PGSIZE * 32);   // same allocation secret.c made
  end = end + 9 * PGSIZE;          // same page offset
  printf("%s\n", end + 32);        // secret was written at +32, print it alone
  exit(1);
}
