#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include "patchlevel.h"

#ifdef HAVEFNMATCH

#include <fnmatch.h>

#else

/* A limited fnmatch replacement. Only accepts patterns 
   in the form "ccccc" or "ccccc*". (These were the patterns accepted
   by older afio versions.)

*/

int fnmatch(const char *p, const char *s, int flag)
{
  if(p[strlen(p)-1]=='*')
      return strncmp(p,s,strlen(p)-1);
  else
      return strcmp(p,s);
}

#endif

#ifdef linux
#include <utime.h>
#endif

#include "afio.h"

STATIC int ignoreslash=1;

/*
 * Pathnames to (or not to) be processed.
 */
typedef struct pattern
{
  struct pattern *p_forw;	/* Forward chain */
  char *p_str;			/* String */
  int p_len;			/* Length of string */
  int p_ptype;			/* Pattern type */
                                /* types are: PATTYPE_MATCH, _NOMATCH, _EXT. */
  int p_noglob;                 /* influenced by used of -7 flag */
} Pattern;

STATIC Pattern *pattern;	/* Pathname matching patterns */

/*
 * nameadd()
 *
 * Add a name to the pattern list.
 */
STATIC void
nameadd (name, ptype)
     reg char *name;
     int ptype;
{
  reg Pattern *px;

  px = (Pattern *) memget (sizeof (Pattern));
  px->p_forw = pattern;
  px->p_str = strdup(name);
   if(px->p_str==NULL) fatal(name,"out of memory.");
  px->p_len = strlen (name);
  px->p_ptype = ptype;
  px->p_noglob = noglob;
  pattern = px;

  /* fprintf(stderr,"--%s added\n",name); */
}

/*
 * nameaddfile()
 *
 * Add a file of patterns to the pattern list.
 * Returns 0 on failure, 1 on success.
*/

STATIC int
nameaddfile(fname,ptype,parsewith0)
     char *fname;
     int ptype;
     int parsewith0;
{
 FILE *infile;
 char pat[PATHSIZE+1];
 int c,i;
 
 infile=fopen(fname,"r");
 if(infile==0) return 0;

 if(parsewith0)
 {
     i=0;
     while((c = fgetc(infile))!=EOF)
     {
         if(i > sizeof(pat))
         fatal(pat,"Path name too long");

         pat[i]=c;

         if(c == '\0')
         {
              if(i>0) nameadd(pat,ptype);
              i=0;
              continue;
         }
         i++;
     }
     /* handle case where the last entry in the file does not end in \0 */
     if(i>0 && i<sizeof(pat)) 
       {
	 pat[i]=0;
         nameadd(pat,ptype);
       }
 }
 else
 {
 while(fgets(pat,PATHSIZE,infile)!=NULL)
   {
    /* remove \n */
     pat[strlen(pat)-1]='\0';

     if(strlen(pat)==0) continue; /* skip empty lines in file */

     if(pat[strlen(pat)-1]==' ')
       {
        warn(pat,"warning: trailing space in this pattern.");
       }

     nameadd(pat,ptype);
   }
 }
 
 fclose(infile);
 return 1;
}


/*
 * namecmp()
 *
 * Compare a pathname with the pattern list. Returns 0 for
 * a match, -1 otherwise.
 * 
 * This new version uses the GCC library function fnmatch()
 * to provide real pattern matching.
 *
 * Also, this implementation assures that if a file matches a pattern
 * in the `not to process' list, it is never processed.
 * In the old version, this depended on several implementation details
 * and the names in the `to process' list.
 *
 * Control files always match.
 */
STATIC int
namecmp (name, asb)
     reg char *name;
     Stat *asb;
{
  reg Pattern *px;
  int existpospat; /* there are `to process' patterns in the list */
  char *namedot,*p,*n;
  char namec[PATHSIZE];

  /* fprintf(stderr,"name:\"%s\" rdev:%d\n",name,
     asb->sb_rdev);
  */

  if(asb && ISCONTROL(asb)) return 0;

  strcpy(namec,name);
  namedot = strrchr (namec, '.');
  if (Zflag && asb && (asb->sb_mode & S_IFMT) == S_IFREG
      && asb->sb_rdev == 0
      && namedot && namedot[1] == 'z' && !namedot[2]
     )
      *namedot = '\0';
    else
      namedot = 0;

  n=namec;
  if(ignoreslash && (n[0]=='/')) n++;

  for(px=pattern; px; px=px->p_forw)
   if((px->p_ptype)==PATTYPE_NOMATCH)
     {
       p=px->p_str;
       if(ignoreslash && (p[0]=='/')) p++;

       if (px->p_noglob)
	 {
	   if(strcmp(p,n)==0) return -1;
	 }
       else
	 {
	   if(fnmatch(p,n,0)==0) return -1;
	 }
     }

  existpospat=0;

  for(px=pattern; px; px=px->p_forw)
   if((px->p_ptype)==PATTYPE_MATCH)
     {
       existpospat=1;

       p=px->p_str;
       if(ignoreslash && (p[0]=='/')) p++;

       if (px->p_noglob)
	 {
	   if(strcmp(p,n)==0) return 0;
	 }
       else
	 {
	   if(fnmatch(p,n,0)==0) return 0;
	 }
     }

  if(!existpospat) return 0; else return -1;

}


/*
 * namecmp_ext()
 *
 * Compare a pathname with the extensions list. Returns 0 for
 * a match, -1 otherwise.
 * 
 */
STATIC int
namecmp_ext (name)
     reg char *name;
{
  reg Pattern *px;
  char *p,*n;

  n=name;
  if(ignoreslash && (n[0]=='/')) n++;

  for(px=pattern; px; px=px->p_forw)
   if((px->p_ptype)==PATTYPE_EXT)
     {
       p=px->p_str;
       if(ignoreslash && (p[0]=='/')) p++;

       if(fnmatch(p,n,0)==0) return 0;
     }

  return -1;
}


