
/* print sizeof values of struct stat fields
   useful for compatibility testing
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char ** argv)
{
  struct stat s;

  printf("sizeof st_dev = %d\n",sizeof(s.st_dev));
  printf("sizeof st_ino = %d\n",sizeof(s.st_ino));
  printf("sizeof st_mode = %d\n",sizeof(s.st_mode));
  printf("sizeof st_nlink = %d\n",sizeof(s.st_nlink));
  printf("sizeof st_uid = %d\n",sizeof(s.st_uid));
  printf("sizeof st_gid = %d\n",sizeof(s.st_gid));
  printf("sizeof st_rdev = %d\n",sizeof(s.st_rdev));
  printf("sizeof st_size = %d\n",sizeof(s.st_size));
  printf("sizeof st_blksize = %d\n",sizeof(s.st_blksize));
  printf("sizeof st_blocks = %d\n",sizeof(s.st_blocks));
  printf("sizeof st_atime = %d\n",sizeof(s.st_atime));
  printf("sizeof st_mtime = %d\n",sizeof(s.st_mtime));
  printf("sizeof st_ctime = %d\n",sizeof(s.st_ctime));

  return 0;
}

