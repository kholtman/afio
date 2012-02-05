
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "patchlevel.h"

#include "afio.h"


struct extnode { char *ext; struct extnode *next; };

/* merge in auto-generated list of default extensions mentioned in the manpage */
#include "exten_default.h"

/* Read file extensions of files that are not to be compressed
 * from compextsfile.
 * Extenstions in the file are seperated by whitespace.
 * a # begins a comment that lasts till the end of the line.
 */
int readcompexts(char *compextsfile)
{
 FILE *infile;
 char ex[81];
 int c;
 struct extnode *tmp;

 if(compextsfile[0]=='+') compextsfile++; else compexts=NULL;

 infile=fopen(compextsfile,"r");
 if(infile==0)
   {
     fprintf (stderr,
	      "Can't read configuration file %s\n",
	      compextsfile);     
     return 0;
   }

 while(fscanf(infile,"%80s",ex)!=EOF)
   {
     if(ex[0]=='#')
       { /* throw away comment. */
        do{
           c=fgetc(infile);
           if(c==EOF)  { fclose(infile); return 1; }
          }while(c!='\n');
        continue;
       }

     tmp=(struct extnode *)malloc(sizeof(struct extnode));
     if(tmp==NULL) break;
     if((tmp->ext=strdup(ex))==NULL) break;
     tmp->next=compexts;
     compexts=tmp;
   }

 fclose(infile);
 return 1;
}

int matchcompext(char *s)
{
 struct extnode *p;
 size_t sl;

 p=compexts;
 sl=strlen(s);

 while(p!=NULL)
   {
    if(sl >= strlen(p->ext))
      {
	if(extcasesens)
	  {
            if(strcmp(s+sl-strlen(p->ext),p->ext)==0) return 1;
	  }
	else
	  {
            if(strcasecmp(s+sl-strlen(p->ext),p->ext)==0) return 1;
	  }
      }
     p=p->next;
   }

 if(namecmp_ext(s)==0) return 1;

 return 0;
}



