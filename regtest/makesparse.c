
/* test large file compile environment and make a large sparse file
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


void fatal(const char *s)
{
  printf("makesparse: %s\n",s);
  exit(1);
}


int main(int argc, char ** argv)
{
  struct stat s;
  int f;
  unsigned long long pos;
  char *bla="bla bla bla\n";

  printf("sizeof st_size = %d\n",sizeof(s.st_size));

  if(sizeof(s.st_size)<8) fatal("Large file compile environment not present!");

  if(argc!=2) fatal("Wrong number of args");

  f=open(argv[1],O_RDWR);

  if(!f) fatal("Open failed");

  pos=(unsigned long long)(4LL*1024LL*1024LL*1024LL);

  if(lseek(f,(off_t)pos,SEEK_SET)==((off_t)-1)) 
    { perror("lseek"); fatal("lseek failed"); }

  if(!write(f,bla,strlen(bla))) { perror("write"); fatal("write failed"); }

  close(f);

  if(lstat(argv[1],&s)) { perror("stat"); fatal("stat failed"); }

  if(s.st_size!=pos+strlen(bla)) 
    fatal("created file does not have the expected length");

  return 0;
}

