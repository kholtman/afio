
/* compare status of files argv[1] and argv[2],
   testing if afio restored argv[2] right.
   if we have an argv[3], then do not compare uid/gid.
   returns 1 if the test is not OK or other things are not OK.
   prints reason for that to stdout.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>

char ** av;

void fatal(const char *s)
{
  printf("cmpstat %s %s: %s\n",av[1],av[2],s);
  exit(1);
}

void fatalls(const char *s)
{
  char buf[1000];
  printf("cmpstat %s %s: %s\n",av[1],av[2],s);
  sprintf(buf,"ls -ld \"%s\" \"%s\"",av[1],av[2]);
  system(buf);
  exit(1);
}


int main(int argc, char ** argv)
{
  struct stat s1,s2;

  av=argv;  

  if(argc>4) { printf("cmpstat: wrong number of args\n"); return 1; }

  if(lstat(argv[1],&s1)) fatal("file 1 not found");

  if(lstat(argv[2],&s2)) fatal("file 2 not found");

  if(s1.st_nlink != s2.st_nlink) fatalls("nlink difference");

  if(argc!=4)
    {
      if(s1.st_uid != s2.st_uid) fatalls("uid difference");
      if(s1.st_gid != s2.st_gid) fatalls("gid difference");
    }

  if(!(S_ISDIR(s1.st_mode) || S_ISLNK(s1.st_mode)))
    if(s1.st_mtime != s2.st_mtime) fatalls("mtime difference");

  if(S_ISCHR(s1.st_mode) || S_ISBLK(s1.st_mode))
    {
      if(s1.st_rdev != s2.st_rdev) fatalls("device number difference");
    }

  if(s1.st_mode != s2.st_mode) fatalls("mode difference");

  if(!(S_ISDIR(s1.st_mode)))
     if(s1.st_size != s2.st_size) fatalls("size difference");

  return 0;
}
