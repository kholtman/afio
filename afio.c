/*
 * afio.c
 *
 * Manipulate archives and files.
 *
 * This software was written by Mark Brukhartz at Lachman Associates,
 * Inc..  Additional code was written by a large cast of people.  
 *
 * Licensing and (re)distribution
 * ------------------------------
 *
 * THE SUMMARY INFORMATION BELOW WAS WRITTEN FOR THE BENEFIT OF 
 * SOFTWARE DISTRIBUTORS
 *
 * Because of historical reasons, different parts of this software
 * package are covered by different licenses.  As of 2012, my (Koen
 * Holtman's) intepretation of the license status is as follows.
 *
 * - The main authors and maintainers all intend afio to be free and
 *   freely distributable.  It has been distributed widely and for
 *   free since at least 1987, when it was posted to the
 *   comp.sources.linux newsgroup.  
 *
 * - The legal risks to re-distributers, coming from the licence, are
 *   effectively zero.
 *
 * - The afio license is not a standard OSI/FSF approved free software
 *   license, it predates these license texts.  Unfortunately, the
 *   afio license includes wording that is considered to be
 *   problematic by several of todays open source licensing legal
 *   experts, because the wording leaves too much room for
 *   interpretation.  It is impossible to upgrade this problematic
 *   license text.
 *
 * - Therefore, if your software redistribution or package labeling
 *   policy implies a rejection of non-standard OSI/FSF approved free
 *   software licenses, you need to be careful about handling afio.
 *
 *   See the file afio_license_issues_v5.txt for more legal
 *   discussion.
 *
 * END OF SUMMARY INFORMATION
 *
 * ------------------------------------------------------------------ 
 *
 * License notice 1, covering part of this software package.
 *
 * [Covers the original 1985 afio code]
 * 
 * Copyright (c) 1985 Lachman Associates, Inc..
 *
 * This software was written by Mark Brukhartz at Lachman Associates,
 * Inc.. It may be distributed within the following restrictions:
 *	(1) It may not be sold at a profit.
 *	(2) This credit and notice must remain intact.
 * This software may be distributed with other software by a commercial
 * vendor, provided that it is included at no additional charge.
 *
 *
 * [Note: it is believed that condition 5 of the Perl "Artistic
 * License" implies the intent of restriction (1) above.]
 *
 * --------
 *
 * ** License notice 2, covering part of this software package.
 *
 * [Covers the tempnam function]
 *
 * Copyright:	Copyright (c) 1989 by Monty Walls.
 *		Not derived from licensed software.
 *
 *		Permission to copy and/or distribute granted under the
 *		following conditions:
 *
 *		1). This notice must remain intact.
 *		2). The author is not responsible for the consequences of use
 *			this software, no matter how awful, even if they
 *			arise from defects in it.
 *		3). Altered version must not be represented as being the
 *			original software.
 *
 * --------
 *
 * ** License notice 3, covering part of this software package.
 *
 * [Covers the contents of the gnu.fnmatch.tar.gz file]
 *
 *  Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *  
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *  
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; see the file COPYING.LIB.  If
 *  not, write to the Free Software Foundation, Inc., 675 Mass Ave,
 *  Cambridge, MA 02139, USA.  
 *
 * --------
 *
 * ** License notice 4, covering part of this software package.
 *
 * [Covers the remainder of this software package]
 *
 * Additional code was written by a large cast of people.
 *
 * All additional code may be redistributed under the conditions of
 * license notice 1.
 *
 * Note that the authors of the additional code retain the right to
 * allow for the re-distribution of their code under weaker (and less
 * exotic) conditions.
 *
 * --------
 *
 * ** WARRANTY NOTICE
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * [End of licensing and redistribution section] 
 *
 * ---------------------------------------------------------------------
 *
 * Makefile options:
 *  o Define INDEX to use index() in place of strchr() (v7, BSD).
 *  o Define MEMCPY when an efficient memcpy() exists (SysV).
 *  o Define MKDIR when a mkdir() system call is present (4.2BSD, SysVr3).
 *  o Define NOVOID if your compiler doesn't like void casts.
 *  o Define SYSTIME to use <sys/time.h> rather than <time.h> (4.2BSD).
 *  o Define VOIDFIX to allow pointers to functions returning void (non-PCC).
 *  o Define CTC3B2 to support AT&T 3B2 streaming cartridge tape.
 *  o Define HAVEFCNTL if you have <fcntl.h>
 *  o Define MYTEMPNAM if you don't have tempnam()
 *  o Define UNIXPC if you are on a 3b1, 7300, etc.
 *  o Define HAVEMEMCMP if you have memcmp otherwise assumes bcmp
 *  o Define DEFFMTCMD to being how to format the media you use the most.
 *  o Define LONGZFILE if you want .Z to be tagged on the end of a 14 char
 *
 * BUGS:
 *	See the manpage.
 *
 * Added by Jeff Buhrt:
 *	Floppy Verify/format/restart output in the middle of a set,
 *	compress files on output, extended error messages and logging
 *
 * Added by Dave Gymer:
 *	Lotsa bugfixes, Linux support, recognition of .Z files in an archive
 *	that were compressed already (and shouldn't be uncompressed).
 *	Displays compression ratios.
 *
 * See the HISTORY file for more revision info.  */

#ifdef LINT
static char *ident = "$Header: /u/buhrt/src/afio/RCS/afio.c,v 2.3 1991/09/25 20:08:33 buhrt Exp $";
#endif

#include <stdio.h>
#include <errno.h>

#ifdef sun
#include <sys/types.h>
#include <utime.h>
#include <signal.h>
#include <strings.h>
#include <sys/wait.h>
#define linux_tstamp 1
#if 0
/* fix SunOS errno.h not declaring what the manpage says it declares 
   bogosity. */
 extern int sys_nerr;
 extern char *sys_errlist[];
#endif
#endif

#ifdef hpux
#if 0
 /* Fix that HPUX dosent have sys_nerr or sys_errlist 
    Added by Daniel Andersson, daniel.andersson@sto.sema.se
  */
extern int sys_nerr;
extern char *sys_errlist[];
#endif
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#ifdef __CYGWIN32__
#include <signal.h>
#else
#include <sys/signal.h>
#endif
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include "patchlevel.h"

#ifdef linux

#define linux_tstamp 1

#include <utime.h>
#include <sys/wait.h>

/* for flushing floppy cache */
#include <linux/fd.h>
#endif

/* compatibility fixes for IRIX native c compiler. */
#ifdef irix_cc_compatibility
#define linux_tstamp 1
#include <strings.h>
#endif

#ifndef	major
#ifdef sun
#include <sys/mkdev.h>
#else
#include <sys/sysmacros.h>
#endif
#endif /* major */

#include "afio.h"


/* define 1 to enable file descriptor leak debugging code */
#define FDDEBUG 0

     /*
      * Static variables.
      */
     STATIC short Fflag;	/*
				 * floppy flag (write when buf full)
				 * set -sdisk_size as well
				 */
     STATIC short Zflag;	/* compress the files that we can */
     STATIC short verifyflag;	/* Verify (floppy) flag */
     STATIC short verifycnt;
#ifdef	CTC3B2
     STATIC short Cflag;	/* Enable 3B2 CTC streaming (kludge) */
#endif /* CTC3B2 */
     STATIC short aflag;	/* Preserve atime (while munging ctime) */
     STATIC short dflag;	/* Don't create missing directories */
     STATIC short fflag;	/* Fork before writing to archive */
     STATIC short gflag;	/* Change to input file directories */
            short hflag;	/* Follow symbolic links */
     STATIC short jflag;	/* Don't generate sparse filesystem blocks */
     STATIC short kflag;	/* Skip initial junk to find a header */
            short lflag;	/* Link rather than copying (when possible) */
     STATIC short mflag;	/* Ignore archived timestamps */
     STATIC short nflag;	/* Keep newer existing files */
     STATIC short uflag;	/* Report files with unseen links */
     STATIC short vflag;	/* Verbose */
     STATIC short xflag;	/* Retain file ownership */
     STATIC short zflag;	/* Print final statistics */
     STATIC short flag0;	/* Input names terminated with '\0' */
     STATIC short Jflag=0;	/* Keep going after archive write error */
     STATIC short hidequit;	/* show the quit option? */
     STATIC short abspaths;	/* allow absolute path names? */
     STATIC uint arbsize = BLOCK;	/* Archive block size */
     STATIC short areof;	/* End of input volume reached */
     STATIC int arfd = -1;	/* Archive file descriptor */
     STATIC ulonglong arleft;	/* Space remaining within current volume */
     STATIC char *arname;	/* Expanded archive name */
     STATIC uint arpad;		/* Final archive block padding boundary */
     STATIC char arspec[PATHSIZE];	/* Specified archive name */
     STATIC ulonglong aruntil;	/* Volume size limit */
     STATIC int roundaruntil=1; /* Round aruntil to multiple of arbsize? */
     STATIC ulonglong maxsizetocompress=200L*1024L*1024L; /* ==0, then no max */
     STATIC int askfornext=0;	/* Ask for next disk on input eof? */
     STATIC uint arvolume = 1;	/* Volume number */
     STATIC off_t buflen;	/* Archive buffer length */
     STATIC char *buffer;	/* Archive buffer */
     STATIC char *bufidx;	/* Archive buffer index */
     STATIC char *bufend;	/* End of data within archive buffer */
     STATIC Child *children;	/* Child processes */
     STATIC char *formatcmd = DEFFMTCMD;	/* how to format */
     STATIC gid_t gid;		/* Group ID */
     STATIC Link *linkbase[1024];/* Unresolved link information */
     STATIC unsigned char ino16bitused[256*256/8];  /* bitmap of 16 bit inode numbers */
     STATIC ino_t freshino = 0; /* counter to make fresh inos */
     STATIC FILE *logfile = NULL;	/* log same errors as stderr would */
     STATIC ushort mask;	/* File creation mask */
     STATIC char *myname;	/* Arg0 */
     extern char *optarg;	/* Option argument */
     extern int optind;		/* Command line index */
     STATIC int outpid;		/* Process ID of outstanding outflush() */
     STATIC char pwd[PATHSIZE];	/* Working directory (with "-g") */
     STATIC int pipepid;	/* Pipeline process ID */
     STATIC time_t timenow;	/* Current time */
     STATIC time_t timewait;	/* Time spent awaiting new media */
     STATIC ulonglong total;	/* Total number of bytes transferred */
     STATIC int ttyf = -1;	/* For interactive queries (yuk) */
     STATIC uid_t uid;		/* User ID */
     int uncompressrun = 0;	/* is uncompress running? its pid if so */
     char uncompto[PATHSIZE];	/* name we uncompressed to */
     STATIC int anycorrupt = 0; /* if any data is corrupted */
     STATIC int warnings = 0; /* if any data is corrupted */
     int printbytepos = 0;      /* print position of each file in archive */
     ulonglong bytepos;            /* position of first byte of current file */
     STATIC char *controlscript=NULL;  /* script to pipe control files to */
     STATIC char *promptscript=NULL; /* name of disk-change script */
     STATIC ushort extfmt = 0;  /* Use extended ASCII format */
     STATIC ushort cpiocompat = 0;  /* Never use large ASCII format headers */
     STATIC ushort cpiowarned = 0;  /* warning given? */
     STATIC char *email=NULL;        /* email address to notify of volume change requests */
     STATIC int rewindfd = -1;  /* file descriptor to rewind before
                                   (un)compress invocations */
     STATIC char *ignorewarnings="mc"; /* data for -1 option */
     STATIC char *aruntil_string; /* -s option string given by user */
     STATIC int extcasesens=0; /* Case sensitive matching in -E option? */
     STATIC Dir *DirP=NULL; /* list of directories with their saved timestamps */
     STATIC char firstfilename[PATHSIZE]=""; /* for temp storage during -o */
     STATIC int useoutmodetoc=0; /* used in tocentry() */
     STATIC short noglob=0; /* disable globbing */

int main (int ac, char **av)
{
  reg int c;
  reg uint group = 1;
  VOIDFN (*fn)(char **) = NULL;
  time_t timedone;
  auto char remote[PATHSIZE];
  char *exitmsg;
  int status;
  Stat tmpstat;

#if 0
  printf("sizeof off_t = %d\n",sizeof(off_t));
  printf("sizeof size_t = %d\n",sizeof(size_t));
#endif

  if ((myname = strrchr (*av, '/')))
    ++myname;
  else
    myname = *av;
  mask = umask (0);
  uid = getuid ();
  gid = getgid ();
  if (uid == 0)
    xflag = 1;

  /* ignore SIGPIPE to deal with gzip -d exiting prematurely */
  VOID signal (SIGPIPE, SIG_IGN);

  /* All letters have been used as options, now eating into the numbers....
   */

  while ((c = options (ac, av, 
         "aioprtIOVCb:c:de:fghjklmns:uvxXy:Y:zFKZL:R:qAE:G:M:w:W:T:SBD:P:Q:U4JH:0@:N:3:1:92:56:7"))
        )
    {
      switch (c)
	{
	case 'r':
	  if (fn)
	    usage ();
	  fn = readcheck;
	  break;
	case 'i':
	  if (fn)
	    usage ();
	  fn = in;
	  break;
	case 'o':
	  if (fn)
	    usage ();
	  fn = out;
	  useoutmodetoc=1;
	  break;
	case 'p':
	  if (fn)
	    usage ();
	  fn = pass;
	  break;
	case 't':
	  if (fn)
	    usage ();
	  fn = toc;
	  break;
	case 'I':
	  if (fn)
	    usage ();
	  fn = copyin;
	  break;
	case 'O':
	  if (fn)
	    usage ();
	  fn = copyout;
	  break;
	case 'V':
	  VOID printf ("%s: Version %s dated %s\n",
		       myname, VERSION, DATE);
	  exit (0);
#ifdef	CTC3B2
	case 'C':
	  ++Cflag;
	  arbsize = 31 * 512;
	  group = 10;
	  aruntil = 1469 * 31 * 512;
	  break;
#endif /* CTC3B2 */
	case 'a':
	  ++aflag;
	  break;
	case 'b':
	  if ((arbsize = (uint) optsize (optarg)) == 0)
	    fatal (optarg, "Bad block size");
	  break;
	case 'c':
	  if ((group = (uint) optsize (optarg)) == 0)
	    fatal (optarg, "Bad buffer count");
	  break;
	case 'd':
	  ++dflag;
	  break;
	case 'e':
	  arpad = (uint) optsize (optarg);
	  break;
	case 'f':
	  ++fflag;
	  break;
	case 'g':
	  ++gflag;
	  break;
	case 'h':
	  ++hflag;
	  break;
	case 'j':
	  ++jflag;
	  break;
	case 'k':
	  ++kflag;
	  break;
	case 'l':
	  ++lflag;
	  break;
	case 'm':
	  ++mflag;
	  break;
	case 'n':
	  ++nflag;
	  break;
	case 's':
	  /* Do a 'dry run' to check all values for syntax errors */
	  aruntil_string = strdup(optarg);
          while(aruntil_string) update_aruntil();
	  /* Now the real initialisation */
          aruntil_string = optarg;
          update_aruntil();
	  if (aruntil == 0)  askfornext = 1;
	  break;
	case 'F':
	  ++Fflag;
	  break;
	case 'Z':
	  ++Zflag;
	  break;
	case 'K':
	  ++verifyflag;
	  break;
	case 'u':
	  ++uflag;
	  break;
	case 'v':
	  ++vflag;
	  break;
	case 'x':
	  xflag = 1;
	  break;
	case 'X':
	  xflag = 0;
	  break;
	case 'y':
	  nameadd (optarg, PATTYPE_MATCH);
	  break;
	case 'Y':
	  nameadd (optarg, PATTYPE_NOMATCH);
	  break;
	case 'z':
	  ++zflag;
	  break;
	case 'L':
	  if ((logfile = fopen (optarg, "a")) == (FILE *) 0)
	    {
	      fprintf (stderr,
		       "Can't open %s to append, get help\n",
		       optarg);
	      exit (1);
	    }
	  break;
	case 'R':
	  formatcmd = optarg;
	  break;
	case 'q':
	  hidequit = TRUE;
	  break;
	case 'A':
	  abspaths = TRUE;
	  break;
        case 'E':
	  if(strcmp(optarg,"CS")==0) extcasesens=1;
	  else if(strcmp(optarg,"CI")==0) extcasesens=0;
          else if(!readcompexts(optarg))
	    {
	      exit (1);
	    }
          break;
        case 'G':
          gzipfactor=(uint) optsize(optarg);
          if((gzipfactor <1) || (gzipfactor >9))
	    {
	      fprintf (stderr,
		       "%s: Illegal gzip speed factor (Must be 1--9)\n",
		       optarg);
	      exit (1);
	    }
          break;
        case 'M':
          maxmem=(off_t) optsize(optarg);
          break;
        case 'T':
          compthreshold=(uint) optsize(optarg);
          break;
        case 'w':
          if(!nameaddfile(optarg,PATTYPE_MATCH,flag0))
	    {
	      fprintf (stderr,
		       "Can't read configuration file %s\n",
		       optarg);
	      exit (1);
	    }
          break;
        case 'W':
          if(!nameaddfile(optarg,PATTYPE_NOMATCH,flag0))
	    {
	      fprintf (stderr,
		       "Can't read configuration file %s\n",
		       optarg);
	      exit (1);
	    }
          break;
        case '6':
          if(!nameaddfile(optarg,PATTYPE_EXT,0))
	    {
	      fprintf (stderr,
		       "Can't read configuration file %s\n",
		       optarg);
	      exit (1);
	    }
          break;
        case 'S':
          ignoreslash=0;
          break;
        case 'B':
          printbytepos=1;
          break;
        case 'D':
          controlscript=optarg;
          break;
  	case 'Q':
	  compressargs=1;
  	  add_arg(optarg);
  	  break;
  	case 'P':
  	  compressprog=optarg;
  	  break;
  	case 'U':	/* compress All files */
  	  forceZflag=1;
	  lflag=1; /* Due to internal limitations we need to set this */
   	  break;
 	case '0':
 	  flag0 = 1-flag0;
 	  break;
 	case '4':       /* Use extended ASCII format */
 	  extfmt = 1;
 	  break;
 	case '5':
 	  cpiocompat = 1;
 	  break;
 	case 'J':
 	  Jflag = 1;
 	  break;
       case 'H':
         promptscript=optarg;  /* name or definition of the promptscript */
         break;
       case '@':
         email=optarg;
         break;
       case '3':
         rewindfd=(int)optsize(optarg);
         break;
       case '1':
         ignorewarnings=optarg;
         break;
       case '9':
	 roundaruntil=0;
         break;
       case '2':
	 maxsizetocompress=optsize(optarg);
         break;
       case '7':
	 noglob = 1-noglob;
	 break;

	default:
	  usage ();
	}
    }
  if (fn == NULL || av[optind] == NULL)
    usage ();

 if (extfmt && cpiocompat) 
   {
     warn(av[0], "Options -4 and -5 cannot be specified at the same time."); 
     usage ();
   }

  if(fflag && aruntil==0)
  {
      fprintf (stderr,"Fatal: must supply nonzero -s [volsize] to use -f\n");
      usage();
  }

  if(compressprog && (rewindfd==-1))
      if(strstr(compressprog,"pgp") || strstr(compressprog,"gpg"))
      {
	  fatal(compressprog,"Must use -3 flag if -P program matches 'pgp' or 'gpg', see the afio manual page.");
      }

  if(!compressprog) compressprog = PRG_COMPRESS;
  compress_arg_list[0] = compressprog;

  if (Fflag)
    {      
      if ((buflen = (off_t) aruntil) == 0)
	usage ();
    }
  else
    buflen = arbsize * group;

  if( roundaruntil )
    {
      /* round aruntil down to a multiple of arbsize: some devices
         (like ftape) puke on a smaller-than-blocksize last write to
         the volume */
      aruntil = ( aruntil / (ulonglong) arbsize );
      aruntil = aruntil * arbsize;
    }

  if (aruntil && (aruntil < arbsize))
    {
#ifdef hpux 
      /* HPUX gcc dosent like the (ulong) fixed by Daniel Andersson
         daniel.andersson@sto.sema.se  */
      fprintf (stderr, "Media size %ld is less than block size %d\n",
               aruntil, arbsize);    
#else
      fprintf (stderr, "Media size %ld is less than block size %d\n",
	       (unsigned long) aruntil, arbsize);
#endif
   usage ();
    }
  if (arpad == 0)
    arpad = arbsize;
  if (fn != pass)
    {
      reg char *colon;
      reg char *perc;
      reg char *equal;
      char  *host;
      reg int isoutput = (fn == out || fn == copyout);

      arname = strcpy (arspec, av[optind++]);
      if ((colon = strchr (arspec, ':')))
	{
	  *colon++ = '\0';
	  if ((perc = strchr (arspec, '%')))
	    *perc++ = '\0';
	  if ((equal = strchr ((perc ? perc : arspec), '=')))
	    *equal++ = '\0';
	  if ((host=strchr(arspec,'@')))
	      *host++ = 0;  
	  VOID sprintf (arname = remote,
			"!%s %s%s %s '%s -%c -b %u -c %u %s'",
			perc ? perc : "rsh",
			host ? "-l ":"",
			host ? arspec : "",
			host ? host : arspec,
			equal ? equal : myname,
			isoutput ? 'O' : 'I', arbsize,
			group, colon);
          if( host )
            *--host = '@';			
	  if (equal)
	    *--equal = '=';
	  if (perc)
	    *--perc = '%';
	  *--colon = ':';
	}
      if (gflag && *arname != '/' && *arname != '!')
	fatal (arspec, "Relative pathname");
      VOID signal (SIGINT, goodbye);
      /*
       * +BLOCK is added to make sure we don't overrun buffer on a
       * read (internal read(1) length is thus met)
       */

      if ((buffer = bufidx = bufend = malloc ((size_t)buflen + BLOCK)) == NULL)
	fatal (arspec, "Cannot allocate enough memory for I/O buffer");

      /* if in -o mode, do a sanity check on the input now.  This
	 should prevent deletion of the archive file contents in most
	 cases when people who want to do afio -i accidentally type
         afio -o.  If not reading from a terminal, ignore check -- this
         allows backup scripts to create empty archives.
       */
      if((fn == out)&&isatty(STDIN))
	{
	  while(1)
	    {
	      if (lineget (stdin, firstfilename, PATHSIZE) < 0)
		{
		  fatal("-o (write archive) mode", "could not read any file names from stdin");
		}
	      if(strncmp(firstfilename,"//--",4)==0) break;
	      if (STAT (firstfilename, &tmpstat) < 0)
		{
		  VOID warn (firstfilename, syserr ());
		  VOID warn (firstfilename, "-o (write archive) mode needs file names on stdin");
		}
	      else break;
	    }
          /* OK, if there was a good file name, then
             now firstfilename[] has a non-NULL string, this indicates
	     to the openin function that it needs to use that string, not
	     read from stdin */
	}
	
      /*
       * open the archive file
       *
       * open a floppy at the last moment (if output), otherwise now
       * note we set arleft prematurely so we don't have to open the
       * disk now
       */
      if (!Fflag || !isoutput)
	{
	  if (nextopen (isoutput ? O_WRONLY : O_RDONLY) < 0)
	    goodbye (1);
	}
      else
	arleft = aruntil;

    }
  timenow = time ((time_t *) NULL);
  (*fn) (av + optind);
  timedone = time ((time_t *) NULL);
  if (uflag)
    linkleft ();
  if (vflag || (fn == toc))
    fflush(stdout);

  if(cpiowarned)
    {
      /* repeat warning message at end */
     VOID warn_nocount(arspec,"Warning: Created archive is not fully compatible with cpio or afio versions 2.4.7 and lower.");
     VOID warn_nocount(arspec,"See the ARCHIVE PORTABILITY section of the manpage.");
    }

  exitmsg="The operation was successful.";
  if(warnings) 
  {
      exitmsg=malloc(80);
      if(exitmsg==NULL) 
	  exitmsg="The operation HAD WARNINGS ABOUT ERRORS.";
      else
	  sprintf(exitmsg,"The operation HAD %d WARNING%s ABOUT ERRORS.",warnings,warnings==1?"":"S");
  }
  if(anycorrupt) exitmsg="The operation FAILED.";


  if (zflag)
    {
      reg FILE *stream;

      stream = fn == toc || fn == copyin || arfd == STDOUT ? stderr : stdout;
      VOID fprintf (stream, "%s: ", myname);
      prsize (stream, total);
      VOID fprintf (stream, " bytes %s in %lu seconds. %s\n",
                    fn == pass
                    ? "transferred"
                    : fn == out || fn == copyout
                    ? "written"
                    : "read",
                    timedone - timenow - timewait,
                    exitmsg);     
    }
  if (logfile != (FILE *) 0)
    {
      VOID fprintf (logfile, "%s: Final count: ", myname);
      prsize (logfile, total);
      VOID fprintf (logfile,
		    " bytes %s in %lu seconds (+waited %lu seconds for disk swapping (%u disks)) finished at %s",
		    (fn == pass ? "transferred" : (fn == out
						   || fn == copyout ? "written" : "read")),
		    timedone - timenow - timewait,
		    timewait, arvolume, ctime (&timedone));
      VOID fprintf (logfile,"%s\n",exitmsg);
    }
  nextclos ();

  /* Added to lower chance of race condition (?) in rsh processing
     when reading remote files */
  if(fn==copyin) { fflush(stdout);  fclose(stdout); sleep(2);  } 
  if (email) mail(email,-1,arspec);

  status=0;
  if(anycorrupt) status=1;
  if(warnings) if(index(ignorewarnings,(int)'a')==NULL) status=1;

  goodbye (status);
  /* NOTREACHED */
  return 0;	/* silence gcc -Wall */
}



/*
 * update_aruntil()
 *
 * Sets the next aruntil-value out of the options-list
 */
void update_aruntil()
{    
    char *next_aruntil_string = aruntil_string;

    if(aruntil_string)
      {
	aruntil_string = strchr (aruntil_string, ',');
	if(aruntil_string) *aruntil_string++='\0';

	aruntil = optsize (next_aruntil_string);
      }
}

/*
 * mail()
 *
 * Mail tape change message
 */
void mail(char *who,int vol,char *archive)
{
	FILE *fp;
	char cmd[1024];
	char hostname[256];
	gethostname(hostname,sizeof(hostname));
	sprintf(cmd,"sendmail %s",who);
	fp = popen(cmd,"w");
	if (fp == NULL ) {
		perror(cmd);
		return;
	}
	fprintf(fp,"From: Afio archiver\n");
	fprintf(fp,"Subject: %s %s: %s\n\n",hostname,archive,vol<0?"operation complete":"volume change needed");
	fprintf(fp,"Hostname: %s\n",hostname);
	fprintf(fp,"Archive : %s\n\n",archive);
	if(vol>=0) 
	    fprintf(fp,"Need change to volume #%d.\n\n",vol);
	else
	    fprintf(fp,"Operation complete.\n\n");
	pclose(fp);
}


/*
 * copyin()
 *
 * Copy directly from the archive to the standard output.
 */
STATIC VOIDFN
copyin (char **av)
{
  reg int got;
  reg uint have;

  if (*av)
    fatal (*av, "Extraneous argument");
  while (!areof || askfornext)
  { 
      VOID infill ();
      while ((have = bufend - bufidx))
	  if ((got = writeall (STDOUT, bufidx, have)) < 0)
	      fatal ("<stdout>", syserr ());
	  else 
	  {
	      total+=have;
	      if (got > 0)
		  bufidx += got;
	      else
	        return;
	  }
  }
}

/*
 * copyout()
 *
 * Copy directly from the standard input to the archive.
 */
STATIC VOIDFN
copyout (char **av)
{
  reg int got;
  reg uint want;

  if (*av)
    fatal (*av, "Extraneous argument");
  for (;;)
    { 
      while ((want = bufend - bufidx) == 0)
	outflush (NOTDONE);
      if ((got = read (STDIN, bufidx, want)) < 0)
	fatal ("<stdin>", syserr ());
      else if (got == 0)
	break;
      else
      {
	  bufidx += got;
	  total += got; /* actually a bit too early for bytes written count */
      } 
    }
  outflush (DONE);
  if (fflag)
    outwait ();
}

/*
 * dirchg()
 *
 * Change to the directory containing a given file.
 */
STATIC int
dirchg (char *name, char *local)
{
  reg char *last;
  reg int len;
  auto char dir[PATHSIZE];

  if (*name != '/')
    return (warn (name, "Relative pathname"));
  for (last = name + strlen (name); last[-1] != '/'; --last)
    ;
  len = last - name;
  strncpy (dir, name, (size_t)len)[len] = '\0';
  VOID strcpy (local, *last ? last : ".");
  if (strcmp (dir, pwd) == 0)
    return (0);
  if (chdir (dir) < 0)
    return (warn (name, syserr ()));
  VOID strcpy (pwd, dir);
  return (0);
}

/*
 * dirmake()
 *
 * Make a directory. Returns zero if successful, -1 otherwise.
 */
STATIC int
dirmake (char *name, Stat *asb) 
{
  if (mkdir (name, asb->sb_mode & S_IPOPN) < 0)
    return (-1);
/* First do the chown, then the chmod, because the chown may clear
   the suid/sgid bits we want to set.
*/
  if (xflag)
    VOID chown (name,
		(uid == 0 ? asb->sb_uid : uid),
		asb->sb_gid);
  if (asb->sb_mode & S_IPEXE)
    VOID chmod (name, asb->sb_mode & S_IPERM);
  return (0);
}

/*
 * dirneed()
 *
 * Recursively create missing directories (with the same permissions
 * as their first existing parent). Temporarily modifies the 'name'
 * argument string. Returns zero if successful, -1 otherwise.
 */
STATIC int
dirneed (char *name)
{
  reg char *cp;
  reg char *last;
  reg int ok;
  static Stat sb;

  last = NULL;
  for (cp = name; *cp;)
    if (*cp++ == '/')
      last = cp;
  if (last == NULL)
    return (STAT (".", &sb));
  *--last = '\0';
  ok = STAT (*name ? name : "/", &sb) == 0
    ? ((sb.sb_mode & S_IFMT) == S_IFDIR)
    : (!dflag && dirneed (name) == 0 && dirmake (name, &sb) == 0);
  *last = '/';
  return (ok ? 0 : -1);
}

/*
 * fatal()
 *
 * Print fatal message and exit.
 */
STATIC void
fatal (char *what, char *why) 
{
  /* print position in archive if some data was transferred */
  if(total>0)
    VOID warnarch ("Fatal error:",(off_t)0);
  VOID warn (what, why);
  goodbye (1);
}



/*
 * writeall()
 *
 * Write all bytes in buf or return -1.  Used to fix invalud assumptions
 * about write() elsewhere.
 */
STATIC
int writeall(int fd, const char *buf, unsigned int count)
{
 ssize_t put;
 unsigned int totalput;

 totalput=0;
 while(totalput<count)
 {
     put=write(fd,buf+totalput,count-totalput);

     if(put<0) return put;
     totalput+=put;
 }

 return count;
}

/*
 * readall()
 *
 * Read, completely filling buf if we can, or return short size or -1.  
 * Used to fix invalud assumptions
 * about read() elsewhere.
 */
STATIC
int readall(int fd, char *buf, unsigned int count)
{
 ssize_t got;
 unsigned int totalgot;

 totalgot=0;
 while(totalgot<count)
 {
     got=read(fd,buf+totalgot,count-totalgot);     
     /* if(got!=count) printf("got on %d = %d\n",fd,got); */
     if(got<=0) 
       {
	 if(got==0) return totalgot;
	 if(got<0) return got;
       }
     totalgot+=got;
 }

 return count;
}


/*
 * mayberewind()
 *
 * implement -3 option
 */
STATIC
void mayberewind()
{
  if(rewindfd>=1) 
  {
    if(lseek(rewindfd,(off_t)0,SEEK_SET)<0)
    {
	fatal("-3 option",syserr());
    }
  }
}


 /*
 * savedirstamp()
 *
 * Remember necessary timestamps for a directory,
 * so they can be restored afterwards.
 */
STATIC void
savedirstamp (char *name, time_t mtime)
{
  Dir *dirp;

  if( (dirp=(Dir *)memget(sizeof(Dir))) != NULL && (dirp->d_name=memstr(name)) != NULL )
  {
    dirp->d_mtime=mtime;
    dirp->d_forw=DirP;
    DirP=dirp;
  }
}

/*
 * restoredirstamps()
 *
 * Restore timestamps for the saved list of directories.
 */
STATIC void
restoredirstamps (void)
{
#ifdef linux_tstamp
  auto struct utimbuf tstamp;
#else
  auto time_t tstamp[2];
#endif
  Dir *DirP_forw;
  while(DirP!=NULL)
  {
#ifdef linux_tstamp
    tstamp.actime = DirP->d_mtime;
    tstamp.modtime = DirP->d_mtime;
    /* no error code checking on purpose */
    VOID utime (DirP->d_name, &tstamp);
#else
    tstamp[0] = DirP->d_mtime;
    tstamp[1] = DirP->d_mtime;
    /* no error code checking on purpose */
    VOID utime (DirP->d_name, tstamp);
#endif
    /* We don't call free because we are about to exit anyway.
       Not calling free should make things a bit more robust if the
       memory pool is corrupted due to a bug */
    /* free(DirP->d_name); */
    DirP_forw=DirP->d_forw;
    /* free(DirP); */
    DirP=DirP_forw;
  }
}


/*
 * in()
 *
 * Read an archive.
 */
STATIC VOIDFN
in (av)
     reg char **av;
{
  auto Stat sb;
  auto char name[PATHSIZE];
  int sel,sel2,res;

  if (*av)
    fatal (*av, "Extraneous argument");
  name[0] = '\0';
  while (inhead (name, &sb) == 0)
    {
      sel = (namecmp (name,&sb));
      if (sel)
	{
	  if (inskip (sb.sb_size) < 0)
	    VOID warn (name, "Skipped file data is corrupt");
	}
      else 
	{
	  if((sel2 = inentry (name, &sb)) != 0)
	    VOID warn (name, "unpacking error");
	  else
	    if (vflag)
	      {
		  /* we cast to double and print as floating point because
		     %Lu printing is buggy above 4G (at least with my C library). */
		  if(printbytepos) fprintf(stderr,"%.0f ",(double)bytepos);
		  
		  if (*uncompto)
		    res = fprintf (stderr, "%s -- uncompressed\n", uncompto);
		  else
		    res = fprintf (stderr, "%s -- okay\n", name);
		  
		  /* check for broken pipe on stderr */
		  if(res<0) {
		    if(errno == EPIPE)
		      fatal("<stderr>", syserr());
		  }
	      }
	}
    }
  restoredirstamps();
}


/*
 * readcheck()
 *
 * Read an archive and check contents against existing files
 */

Stat atime_sb;       /* set in openincheck */
int atime_sb_valid;  /* set in openincheck */

STATIC VOIDFN
readcheck (av)
     reg char **av;
{
  auto Stat sb;
  auto char name[PATHSIZE];
  auto char local[PATHSIZE];
  int sel, res;
#ifdef linux_tstamp
  auto struct utimbuf tstamp;
#else
  auto time_t tstamp[2];
#endif

  if (*av)
    fatal (*av, "Extraneous argument");
  name[0] = '\0';
  while (inhead (name, &sb) == 0)
    {
      if ((sel = namecmp (name,&sb)) < 0) {
	if (inskip (sb.sb_size) < 0)
	  VOID warn (name, "Skipped file data is corrupt");
	continue;
      }
      if (vflag) {
	strcpy(local, name);
	tocentry (local, &sb);
      }

      atime_sb_valid=0;
      if ((res = incheckentry(name, &sb)) < 0)
	inskip (sb.sb_size);

      if(aflag && atime_sb_valid && ((sb.sb_mode & S_IFMT)==S_IFREG))
      {
	  /* reset access time, this distroys the ctime btw. */
#ifdef linux_tstamp
	  tstamp.actime = atime_sb.sb_atime;
	  tstamp.modtime = atime_sb.sb_mtime;
	  VOID utime (name, &tstamp);
#else
	  tstamp[0] = atime_sb.sb_atime;
	  tstamp[1] = atime_sb.sb_mtime;
	  VOID utime (name, tstamp);
#endif
	}


    }
}

/*
 * inalloc()
 *
 * Allocate input buffer space (which was previously indexed
 * by inavail()).
 */
STATIC void
inalloc (uint len)
{
  bufidx += len;
  total += len;
}

/*
 * inascii()
 *
 * Read an ASCII header. Returns zero if successful;
 * -1 otherwise. Assumes that the entire magic number
 * has been read.
 */

STATIC int
inascii (magic, name, asb)
     reg char *magic;
     reg char *name;
     reg Stat *asb;
{
  auto uint namelen;
  auto char header[H_STRLEN + 1];
  PStat pasb;

  if (strncmp (magic, M_ASCII, M_STRLEN) != 0)
    return (-1);

  if (inread (header, H_STRLEN) < 0)
    return (warnarch ("Corrupt ASCII header", (off_t) H_STRLEN));
  header[H_STRLEN] = '\0';

  if (sscanf (header, PH_SCAN, &pasb.PSt_dev,
             &pasb.PSt_ino, &pasb.PSt_mode, &pasb.PSt_uid,
             &pasb.PSt_gid, &pasb.PSt_nlink, &pasb.PSt_rdev,
             &pasb.PSt_mtime, &namelen, &pasb.PSt_size) != H_COUNT)
    return (warnarch ("Bad ASCII header", (off_t) H_STRLEN));
  /* now, we let the compiler cast the info to the right types (field sizes) */
  asb->sb_dev = pasb.PSt_dev;
  asb->sb_ino = pasb.PSt_ino;
  asb->sb_mode = pasb.PSt_mode;
  asb->sb_uid = pasb.PSt_uid;
  asb->sb_gid = pasb.PSt_gid;
  asb->sb_nlink = pasb.PSt_nlink;
  asb->sb_rdev = pasb.PSt_rdev;
  asb->sb_mtime = pasb.PSt_mtime;
  asb->sb_size = pasb.PSt_size;
  if (namelen == 0 || namelen >= PATHSIZE)
    return (warnarch ("Bad ASCII pathname length", (off_t) H_STRLEN));
  if (inread (name, namelen) < 0)
    return (warnarch ("Corrupt ASCII pathname", (off_t) namelen));
  if (name[namelen - 1] != '\0')
    return (warnarch ("Bad ASCII pathname", (off_t) namelen));
  return (0);
}



/*
 * inascii2()
 *
 * Read an ASCII header (new format). Returns zero if successful;
 * -1 otherwise. Assumes that the entire magic number
 * has been read.
 */
 
STATIC int
inascii2 (char *magic, char *name, Stat *asb)
{
  auto uint namelen;
  auto char header[H_STRLEN2 + 1];
  PStat pasb;

  if (strncmp (magic, M_ASCII2, M_STRLEN) != 0)
    return (-1);
  if (inread (header, H_STRLEN2) < 0)
    return (warnarch ("Corrupt extended ASCII header", (off_t) H_STRLEN2));
  header[H_STRLEN2] = '\0';
  if (sscanf (header, PH_SCAN2, &pasb.PSt_dev,
             &pasb.PSt_ino, &pasb.PSt_mode, &pasb.PSt_uid,
             &pasb.PSt_gid, &pasb.PSt_nlink, &pasb.PSt_rdev,
             &pasb.PSt_mtime, &namelen, &pasb.PSt_size) != H_COUNT)
    return (warnarch ("Bad extended ASCII header", (off_t) H_STRLEN));
  /* now, we let the compiler cast the info to the right types (field sizes) */
  asb->sb_dev = pasb.PSt_dev;
  asb->sb_ino = pasb.PSt_ino;
  asb->sb_mode = pasb.PSt_mode;
  asb->sb_uid = pasb.PSt_uid;
  asb->sb_gid = pasb.PSt_gid;
  asb->sb_nlink = pasb.PSt_nlink;
  asb->sb_rdev = pasb.PSt_rdev;
  asb->sb_mtime = pasb.PSt_mtime;
  asb->sb_size = pasb.PSt_size;

  if (namelen == 0 || namelen >= PATHSIZE)
    return (warnarch ("Bad ASCII pathname length", (off_t) H_STRLEN));
  if (inread (name, namelen) < 0)
    return (warnarch ("Corrupt ASCII pathname", (off_t) namelen));
  if (name[namelen - 1] != '\0')
    return (warnarch ("Bad ASCII pathname", (off_t) namelen));
  return (0);
}
 

/*
 * inascii3()
 *
 * Read an ASCII header (large format). Returns zero if successful;
 * -1 otherwise. Assumes that the entire magic number
 * has been read.
 */
 
STATIC int
inascii3 (magic, name, asb)
     reg char *magic;
     reg char *name;
     reg Stat *asb;
{
  auto uint namelen;
  auto char header[H_STRLEN3 + 1];
  PHStat pasb;
  uint specialflags;
  uint extraheaderlen;

  if (strncmp (magic, M_ASCII3, M_STRLEN) != 0)
    return (-1);
  if (inread (header, H_STRLEN3) < 0)
    return (warnarch ("Corrupt extended ASCII3 header", (off_t) H_STRLEN3));
  header[H_STRLEN3] = '\0';
  if (sscanf (header, PH_SCAN3, &pasb.PSt_dev,
             &pasb.PSt_ino, &pasb.PSt_mode, &pasb.PSt_uid,
             &pasb.PSt_gid, &pasb.PSt_nlink, &pasb.PSt_rdev,
             &pasb.PSt_mtime, &namelen, &specialflags, &extraheaderlen, &pasb.PSt_size) != H_COUNT3)
    return (warnarch ("Bad extended ASCII3 header", (off_t) H_STRLEN));
  /* now, we let the compiler cast the info to the right types (field sizes) */
  asb->sb_dev = pasb.PSt_dev;
  asb->sb_ino = pasb.PSt_ino;
  asb->sb_mode = pasb.PSt_mode;
  asb->sb_uid = pasb.PSt_uid;
  asb->sb_gid = pasb.PSt_gid;
  asb->sb_nlink = pasb.PSt_nlink;
  asb->sb_rdev = pasb.PSt_rdev;
  asb->sb_mtime = pasb.PSt_mtime;
  asb->sb_size = pasb.PSt_size;

  if (namelen == 0 || namelen >= PATHSIZE)
    return (warnarch ("Bad ASCII pathname length", (off_t) H_STRLEN));
  if (inread (name, namelen) < 0)
    return (warnarch ("Corrupt ASCII pathname", (off_t) namelen));
  if (name[namelen - 1] != '\0')
    return (warnarch ("Bad ASCII pathname", (off_t) namelen));
  if (inskip ((off_t)extraheaderlen) < 0)
    return (warnarch ("Corrupt header", (off_t) 0));
  return (0);
}
 

/*
 * inavail()
 *
 * Index availible input data within the buffer. Stores a pointer
 * to the data and its length in given locations. Returns zero with
 * valid data, -1 if unreadable portions were replaced with nulls.
 */
STATIC int
inavail (bufp, lenp)
     reg char **bufp;
     uint *lenp;
{
  reg uint have;
  reg int corrupt = 0;

  while ((have = bufend - bufidx) == 0)
    corrupt |= infill ();
  *bufp = bufidx;
  *lenp = have;
  return (corrupt);
}

/*
 * inbinary()
 *
 * Read a binary header. Returns the number of trailing alignment
 * bytes to skip; -1 if unsuccessful.
 */
STATIC int
inbinary (magic, name, asb)
     reg char *magic;
     reg char *name;
     reg Stat *asb;
{
  reg uint namefull;
  auto Binary binary;

  if ((int) *((ushort *) magic) != M_BINARY)
    return (-1);
  memcpy ((char *) &binary,
	  magic + sizeof (ushort),
	  M_STRLEN - sizeof (ushort));
  if (inread ((char *) &binary + M_STRLEN - sizeof (ushort),
	      sizeof (binary) - (M_STRLEN - sizeof (ushort))) < 0)
    return (warnarch ("Corrupt binary header",
		   (off_t) sizeof (binary) - (M_STRLEN - sizeof (ushort))));
  asb->sb_dev = binary.b_dev;
  asb->sb_ino = binary.b_ino;
  asb->sb_mode = binary.b_mode;
  asb->sb_uid = binary.b_uid;
  asb->sb_gid = binary.b_gid;
  asb->sb_nlink = binary.b_nlink;
  asb->sb_rdev = binary.b_rdev;
  asb->sb_mtime = binary.b_mtime[0] << 16 | binary.b_mtime[1];
  asb->sb_size = binary.b_size[0] << 16 | binary.b_size[1];
  if (binary.b_name == 0 || binary.b_name >= PATHSIZE)
    return (warnarch ("Bad binary pathname length",
		   (off_t) sizeof (binary) - (M_STRLEN - sizeof (ushort))));
  if (inread (name, namefull = binary.b_name + binary.b_name % 2) < 0)
    return (warnarch ("Corrupt binary pathname", (off_t) namefull));
  if (name[binary.b_name - 1] != '\0')
    return (warnarch ("Bad binary pathname", (off_t) namefull));
  return (asb->sb_size % 2);
}

/*
 * indata()
 *
 * Install data from an archive. Returns given file descriptor.
 */
STATIC int
indata (fd, size, name)
     int fd;
     reg off_t size;
     char *name;
{
  reg uint chunk;
  reg char *oops;
  reg int sparse;
  reg int corrupt;
  auto char *buf;
  auto uint avail;

  corrupt = sparse = 0;
  oops = NULL;
  while (size)
    {
      corrupt |= inavail (&buf, &avail);
      size -= (chunk = size < (off_t)avail ? (uint) size : avail);
      if (oops == NULL && (sparse = fswrite (fd, buf, chunk)) < 0)
	oops = syserr ();
      inalloc (chunk);
    }
  if (corrupt)
    VOID warn (name, "Corrupt archive data");
  if (oops)
    VOID warn (name, oops);
  else if (sparse > 0
	   && (lseek (fd, (off_t) - 1, 1) < 0
	       || write (fd, "", 1) != 1))
    VOID warn (name, syserr ());
  return (fd);
}

/*
 * incheckdata()
 *
 * Check data from an archive. Returns given file descriptor.
 */
STATIC int
incheckdata (int fd, off_t size, char *name, Stat *asb, int comp) 
{
  reg uint chunk;
  reg char *oops;
  reg int sparse;
  reg int corrupt, warned = 0;
  auto char *buf;
  auto uint avail;
  auto int pfd[2], pfdc[2];
  auto int pid, comppid;

  corrupt = sparse = 0;
  oops = NULL;
  if (comp) {
      /* if compressed, we process data with the following setup:
         - child 1 is the comparator
         - child 2 is the gunzip (gzip -c -d)
         - the main process reads data from the archive and writes it
           to child 2 to be uncompressed
         - child 2 writes the uncompressed data to child 1
         - child 1 reads from the file in the filesystem and from child 2,
           compares, and exits nonzero if is a discrepancy.          
      */

    if (pipe(pfd) < 0) {
      perror("pipe"); exit(1);
    }
    switch ((pid = xfork("incheckdata, decompressing", NODIE))) {
    case -1:
      perror("fork");
      exit(1);
    /* child 1 start ======================== */
    case 0: /* child */
      VOID close (pfd[1]);
      if (arfd != STDIN && arfd != STDOUT) VOID close (arfd);
      if (pipe(pfdc) < 0) {
	perror("pipe"); exit(1);
      }
      switch ((comppid = xfork ("incheckdata, decomp", NODIE))) {
      case -1:
	perror("fork");
	exit(1);
      /* child 2 start ======================== */
      case 0: /* child */
	VOID close (pfdc[0]);
	VOID close (fileno (stdin));
	VOID dup (pfd[0]);
	VOID close (pfd[0]);
	VOID close (fileno (stdout));
	if (dup (pfdc[1]) < 0)
	  exit(1);
	close(pfdc[1]);
	mayberewind();
	if(compressargs)
	    execvp (compressprog, compress_arg_list);
	else
	    execlp (compressprog, compressprog, "-d", "-c", NULL);
	fprintf (stderr, "Could not uncompress, errno %d\n", errno);
	exit(1);
	break;
      /* child 2 end ======================== */
      /* child 1 continued ======================== */
      default:
	{
	  char buff1[40960];
	  char buff2[40960];
	  size_t n;
	  ssize_t n1, n2;

	  close(pfdc[1]);
	  n1 = n2 = 1;
	  while ((n1 > 0) && (n2 > 0)) {
	    n = sizeof(buff1);
	    n1 = readall(fd, buff1, n);
	    n2 = readall(pfdc[0], buff2, n);
	    size -= n1;
	    if (n1 < 0 || n2 < 0 || n1 != n2) {
	      VOID warn_nocount (name, "File in archive has different length than file on filesystem");
	      corrupt = 1;
	      break;
	    }
	    else {
	      if (memcmp(buff1, buff2, (size_t)n1) != 0) {
		if (!warned++)
		  VOID warn_nocount (name, "File in archive has different data than file on filesystem");
		corrupt = 1;
	      }
	    }
	  }
	  if (corrupt)
	    kill(comppid, SIGTERM);
	  close(pfdc[0]);
	  close(fd);
	  if (xwait (comppid, "incheckentry xwait() gunzip", FALSE) != 0) {
	    warn(name, "uncompressing failure");
	    corrupt = 1;
	  }
	  exit(corrupt ? 2 : 0);
       }
      }
    /* child 2 end ======================== */
    default:
    /* main process continue ======================== */
      close(pfd[0]);
      uncompressrun = pid;
      while (asb->sb_size) {
	corrupt |= inavail (&buf, &avail);
	if (corrupt) {
	  break;
	}
	asb->sb_size -= (chunk = asb->sb_size < (off_t)avail ? (uint) asb->sb_size : avail);
	if ((sparse = write (pfd[1], buf, chunk)) < 0)
	  oops = syserr();
	inalloc(chunk);
      }
      close(pfd[1]);
      corrupt |= (xwait (pid, "incheckentry xwait()", FALSE) != 0);
      break;
    }
  }
  else { /* not compressed */
    char buff1[40960];
    ssize_t n1, n2;
    ssize_t n;
    while (asb->sb_size && !corrupt) {
      if (!(corrupt |= inavail (&buf, &avail))) {
	/* cast of asb->sb_size (which can be 8 byte) only happens when
	   the value fits in size_t (which is usually 4 byte) */
	n2 = asb->sb_size < (off_t)avail ? (size_t) asb->sb_size : avail;
	n = (n2 < (ssize_t)sizeof(buff1)) ? n2 : (ssize_t)sizeof(buff1);
	n1 = readall(fd, buff1, n);
	asb->sb_size -= n;
	if (n1 < 0 || n2 < 0 || n1 != n)
	  {
	    if(n1!=n) 
	      VOID warn_nocount (name, "File in archive has different length than file on filesystem");
	    corrupt = 1;
	  }
	else
	  {
	    if( memcmp(buff1, buf, n) != 0 )
	      {
		VOID warn_nocount (name, "File in archive has different data than file on filesystem");
		corrupt = 1;
	      }
	  }
	inalloc((uint)n);
      }
    }
    /* See if the file is _longer_ then our backup. */
    if (read(fd, buff1, 1) > 0) 
      {
	VOID warn_nocount (name, "File in archive is shorter than file on filesystem");
	corrupt = 1;
      }
  }
  close(fd);
  if (corrupt) {
    /* 
        file	: Stat atime_sb
        archive : Stat *asb
    */
    if ( (atime_sb.sb_mtime==asb->sb_mtime) &&
	 (atime_sb.sb_size==asb->sb_size))
    {
      /* file has same mod time and length --> should have verified OK, 
	 so this is very probably a real error in the backup medium.
      */
      VOID warn (name, "File data in archive is corrupt");
      anycorrupt=1;
    }
    else
      {
	VOID warn (name, "Archive headers indicate that file on filesystem was modified during or after archive was made");

	if(index(ignorewarnings,(int)'r')) 
	  {
	    warn_nocount(name, "Not counting this as a verification error");
	    warnings--;
	  }
	else
	  anycorrupt=1;
	
      }

    return -1;
  }
  return 0;
}

/*
 * inentry()
 *
 * Install a single archive entry. Returns zero if successful, -1 otherwise.
 */
STATIC int
inentry (name, asb)
     char *name;
     reg Stat *asb;
{
  reg Link *linkp;
  reg int ifd;
  reg int ofd;
#ifdef linux_tstamp
  auto struct utimbuf tstamp;
#else
  auto time_t tstamp[2];
#endif
  int result;
  result=0;

  if ((ofd = openotty (name, asb, linkp = linkfrom (asb,1), 0, Zflag)) > 0)
    {
      if (asb->sb_size || linkp == NULL || linkp->l_size == 0)
	VOID close (indata (ofd, asb->sb_size, name));
      else if ((ifd = open (linkp->l_path->p_name, O_RDONLY)) < 0)
	VOID warn (linkp->l_path->p_name, syserr ());
      else
	{
	  passdata (linkp->l_path->p_name, ifd, name, ofd);
	  VOID close (ifd);
	  VOID close (ofd);
	}
      /* safety */
      if (uncompressrun)
	{
	  result = xwait (uncompressrun, "inentry xwait()", TRUE);
	  uncompressrun = 0;
          if(result!=0) result=-1;
	}
    }
  else if (ofd < 0)
    return (-1);
  else if (inskip (asb->sb_size) < 0)
    VOID warn (name, "Redundant file data is corrupt");

  /* Cannot set utime on symlink (at least not under Linux) */
  if((asb->sb_mode & S_IFMT) != S_IFLNK)
  {
#ifdef linux_tstamp
      tstamp.actime = tstamp.modtime = mflag ? timenow : asb->sb_mtime;
      VOID utime (name, &tstamp);
#else
      tstamp[0] = tstamp[1] = mflag ? timenow : asb->sb_mtime;
      VOID utime (name, tstamp);
#endif
  }

  return (result);
}

/*
 * incheckentry()
 *
 * Check a single archive entry. Returns zero if successful, -1 otherwise.
 */
STATIC int
incheckentry (name, asb)
     char *name;
     reg Stat *asb;
{
  reg int ifd;
  auto int compression;

  if (ISCONTROL(asb))
  {
      /* process control file */
      if(inentry (name, asb) < 0)
	  if (inskip (asb->sb_size) < 0)
	      VOID warn (name, "Skipped file data is corrupt");

      /* do not verify the control file, return success */
      return(0);
  }
  
  uncompressrun = 0;

  /* The exact logic below here, and inside openincheck and
     incheckdata, is questionable -- it will not catch all faulty
     situations as being a definate error.  It will work in 99.9% of
     the error cases though, so I am not touching it...  -- KH */

  if ((ifd = openincheck (name, asb, &compression, Zflag)) > 0)
    {
      if (asb->sb_size)
	return incheckdata(ifd, asb->sb_size, name, asb, compression);
    }
  else if (ifd < 0) /* some strangeness occured in in archive file */
    {
      anycorrupt = -1;
      return (-1);
    }
  else if (inskip (asb->sb_size) < 0)
    VOID warn (name, "Redundant file data is corrupt");
  if (ifd > 0)
    close(ifd);
  return (0);
}

/*
 * infill()
 *
 * Fill the archive buffer. Remembers mid-buffer read failures and
 * reports them the next time through. Replaces unreadable data with
 * null characters. Returns zero with valid data, -1 otherwise.
 */
STATIC int
infill ()
{
  reg int got;
  static int failed;
  ulonglong ullreadsize;
  size_t readsize;

  bufend = bufidx = buffer;
  if (!failed)
    {
      if (areof) 
	{
	  if (total == 0) 
	    {
	      fatal (arspec, "No input");	 
	    } 
	  else
	    {
	      if((aruntil!=0) || askfornext) 
		{
		  next (O_RDONLY, "Input EOF");
		} 
	      else 
		{
		  fatal (arspec, "Premature input EOF");	    
		}
	    }
	}
#if 0
    fprintf(stderr,"aruntil=%lld arleft=%lld arbsize=%d\n",aruntil,arleft,arbsize);
#endif
      if (aruntil && (arleft == 0))
	next (O_RDONLY, "Input limit reached");
      
      if(aruntil) ullreadsize=arleft; else ullreadsize=buffer + buflen - bufend;
      if(ullreadsize>arbsize) ullreadsize=arbsize;

      readsize=(size_t)ullreadsize;


      while (!failed
	     && !areof
             && (aruntil == 0 || arleft >= (ulonglong)readsize)
	     && (off_t)(buffer - bufend) + buflen >= (off_t)readsize)
	{
	  if ((got = read (arfd, bufend, readsize)) > 0)
	    {
	      bufend += got;
	      arleft -= got;
	    }
	  else if (got < 0)
	    failed = warnarch (syserr (),
			       (off_t) 0 - (bufend - bufidx));
	  else
	    ++areof;
	}
    }
  if (failed && bufend == buffer)
    {
      failed = 0;
      for (got = 0; (uint)got < arbsize; ++got)
	*bufend++ = '\0';
      return (-1);
    }
  return (0);
}

/*
 * inhead()
 *
 * Read a header. Quietly translates old-fashioned binary cpio headers
 * (and arranges to skip the possible alignment byte). Returns zero if
 * successful, -1 upon archive trailer.
 */
STATIC int
inhead (name, asb)
     reg char *name;
     reg Stat *asb;
{
  reg off_t skipped;
  auto char magic[M_STRLEN];
  static int align;

#if FDDEBUG
/* debug code added by KH.  Are we leaking file descriptors? */
	  { int i; i=dup(0); close(i); fprintf(stderr,"%d",i); }
#endif

  if (align > 0)
    VOID inskip ((off_t) align);
  align = 0;

  bytepos=total; /* position of current file */

  for (;;)
    {
      VOID inread (magic, M_STRLEN);
      skipped = 0;

      while (   ((align = inascii (magic, name, asb)) < 0)
 	     && ((align = inascii2 (magic, name, asb)) < 0)
	     && ((align = inascii3 (magic, name, asb)) < 0)
	     && ((align = inbinary (magic, name, asb)) < 0)
	     && ((align = inswab (magic, name, asb)) < 0))
	{
	  if (++skipped == 1)
	    {
	      if (!kflag && total - sizeof (magic) == 0)
		{
		  warn(arspec,"Input does not start with valid archive header.");
		  warn(arspec,"Use the -k option to start reading in the middle of an archive.");
		  fatal (arspec, "Unrecognizable archive");
		  
		}
	      VOID warnarch ("Bad magic number",
			       (off_t) sizeof (magic));
	      if (name[0])
		VOID warn (name, "May be corrupt");
	    }
	  memcpy (magic, magic + 1, sizeof (magic) - 1);
	  VOID inread (magic + sizeof (magic) - 1, 1);
	}
      if (skipped)
	{
	  VOID warnarch ("Apparently resynchronized",
			   (off_t) sizeof (magic));
	  VOID warn (name, "Continuing");
	}
      if (strcmp (name, TRAILER) == 0)
	return (-1);
      if (nameopt (name) >= 0)
	break;
      VOID inskip (asb->sb_size + align);
    }
#ifdef	S_IFLNK
  if ((asb->sb_mode & S_IFMT) == S_IFLNK)
    {
      if (inread (asb->sb_link, (uint) asb->sb_size) < 0)
	{
	  VOID warn (name, "Corrupt symbolic link");
	  return (inhead (name, asb));
	}
      asb->sb_link[asb->sb_size] = '\0';
      asb->sb_size = 0;
    }
#endif /* S_IFLNK */
  if ((name[0] == '/') && !abspaths)
    {
      if (name[1])
	{
	  while ((name[0] = name[1]))
	    ++name;
	} 
      else
	{
	  name[0] = '.';
	}
    }
  
  asb->sb_atime = asb->sb_ctime = asb->sb_mtime;
  return (0);
}

/*
 * inread()
 *
 * Read a given number of characters from the input archive. Returns
 * zero with valid data, -1 if unreadable portions were replaced by
 * null characters.
 */
STATIC int
inread (dst, len)
     reg char *dst;
     uint len;
{
  reg uint have;
  reg uint want;
  reg int corrupt = 0;
  char *endx = dst + len;

  while ((want = endx - dst))
    {
      while ((have = bufend - bufidx) == 0)
	corrupt |= infill ();
      if (have > want)
	have = want;
      memcpy (dst, bufidx, have);
      bufidx += have;
      dst += have;
      total += have;
    }
  return (corrupt);
}

/*
 * inskip()
 *
 * Skip input archive data. Returns zero under normal circumstances,
 * -1 if unreadable data is encountered.
 */
STATIC int
inskip (len)
     reg off_t len;
{
  reg uint chunk;
  reg int corrupt = 0;

  while (len)
    {
      while ((chunk = bufend - bufidx) == 0)
	corrupt |= infill ();
      if ((off_t)chunk > len)
	chunk = len;
      bufidx += chunk;
      len -= chunk;
      total += chunk;
    }
  return (corrupt);
}

/*
 * inswab()
 *
 * Read a reversed byte order binary header. Returns the number
 * of trailing alignment bytes to skip; -1 if unsuccessful.
 */
STATIC int
inswab (magic, name, asb)
     reg char *magic;
     reg char *name;
     reg Stat *asb;
{
  reg ushort namesize;
  reg uint namefull;
  auto Binary binary;

  if ((int) *((ushort *) magic) != swab (M_BINARY))
    return (-1);
  memcpy ((char *) &binary,
	  magic + sizeof (ushort),
	  M_STRLEN - sizeof (ushort));
  if (inread ((char *) &binary + M_STRLEN - sizeof (ushort),
	      sizeof (binary) - (M_STRLEN - sizeof (ushort))) < 0)
    return (warnarch ("Corrupt swapped header",
		   (off_t) sizeof (binary) - (M_STRLEN - sizeof (ushort))));
  asb->sb_dev = (dev_t) swab (binary.b_dev);
  asb->sb_ino = (ino_t) swab (binary.b_ino);
  asb->sb_mode = swab (binary.b_mode);
  asb->sb_uid = swab (binary.b_uid);
  asb->sb_gid = swab (binary.b_gid);
  asb->sb_nlink = swab (binary.b_nlink);
  asb->sb_rdev = (dev_t) swab (binary.b_rdev);
  asb->sb_mtime = swab (binary.b_mtime[0]) << 16 | swab (binary.b_mtime[1]);
  asb->sb_size = swab (binary.b_size[0]) << 16 | swab (binary.b_size[1]);
  if ((namesize = swab (binary.b_name)) == 0 || namesize >= PATHSIZE)
    return (warnarch ("Bad swapped pathname length",
		   (off_t) sizeof (binary) - (M_STRLEN - sizeof (ushort))));
  if (inread (name, namefull = namesize + namesize % 2) < 0)
    return (warnarch ("Corrupt swapped pathname", (off_t) namefull));
  if (name[namesize - 1] != '\0')
    return (warnarch ("Bad swapped pathname", (off_t) namefull));
  return (asb->sb_size % 2);
}

/*
 * lineget()
 *
 * Get a line from a given stream. Returns 0 if successful, -1 at EOF.
 */
STATIC int
lineget (stream, buf, bufsize)
     reg FILE *stream;
     reg char *buf;
     int bufsize;
{
  reg int c;
  int size;
  char *bufstart;

  size=0;
  bufstart=buf;

  for (;;)
    {
      if ((c = getc (stream)) == EOF)
	return (-1);
      if ((!flag0 && c == '\n') || (flag0 && c == '\0'))
	break;
      if(size<bufsize-1)
	*buf++ = c;	
      size++;

    }
  *buf = '\0';

  if(size>bufsize-1)
    warn(bufstart,"Path name too long, truncated");

  return (0);
}

/*
 * linkalso()
 *
 * Add a destination pathname to an existing chain. Assumes that
 * at least one element is present.
 */
STATIC void
linkalso (linkp, name)
     reg Link *linkp;
     char *name;
{
  reg Path *path;

  if (((path = (Path *) memget (sizeof (Path))) == NULL)
      || ((path->p_name = memstr (name)) == NULL))
    return;
  path->p_forw = NULL;
  path->p_back = linkp->l_path->p_back;
  path->p_back->p_forw = path;
  linkp->l_path->p_back = path;
}

/*
 * linkfrom()
 *
 * Find a file to link from. Returns a pointer to a link
 * structure, or NULL if unsuccessful.
 */
/* In v2.4.4: added some checks which would be redundant if not
   for the fact that the standard archive format only stores the 
   lower 16 bits of the inode number.
   The checks reduce the chance of false matches when, reading archives
   produced by older afio versions and by cpio in the old ascii
   format.
*/
STATIC Link *
linkfrom (asb, installing)
     reg Stat *asb;
     int installing;
{
  reg Link *linkp;
  reg Link *linknext;
  reg Link **abase;

  /* check: could file be hard-linked at all? */
  if(asb->sb_nlink<=1) return NULL;
  /* check: if is a directory, then cannot be hard-linked to something */
  if((asb->sb_mode & S_IFMT) == S_IFDIR) return NULL;
  /* check: sb_size==0 for entry in archive which should produce hardlink */
  if(installing && (asb->sb_size!=0)) return NULL;

  for (linkp = *(abase = linkhash (asb->sb_ino)); linkp; linkp = linknext)
    {
      
      if (linkp->l_ino == asb->sb_ino
	  && linkp->l_dev == asb->sb_dev
	  /* check: do mtimes also match? */
	  && linkp->l_mtime == asb->sb_mtime
	  /* check: do modes also match? */
	  && linkp->l_mode == asb->sb_mode
	  )
	{
	  --linkp->l_nlink; /* reference count for -u option */

	  /* in older versions of afio, if l_nlink drops to 0, then the
	     link entry is removed from the hash table.  
             We don't do this anymore because that would break things
	     if the same hard linked directory entry was included
	     multiple times in the archive.  Also it allows us to call
	     linkfrom as often as we want for each file
	  */

	  return (linkp);
	}

      linknext = linkp->l_forw;
    }

  return (NULL);
}


/*
 * linkleft()
 *
 * Complain about files with unseen links.
 */
STATIC void
linkleft ()
{
  reg Link *lp;
  reg Link **base;

  for (base = linkbase; base < linkbase + nel (linkbase); ++base)
    for (lp = *base; lp; lp = lp->l_forw)
      {
	/* printf("%s ino=%ld nl=%d\n",lp->l_path->p_name,(long int)lp->l_ino,(int)lp->l_nlink); */
      if (lp->l_nlink)
	VOID warn (lp->l_path->p_name, "Unseen link(s)");

      }
}

/*
 * linkto()
 *
 * Remember a file with outstanding links. Returns a
 * pointer to the associated link structure, or NULL
 * when linking is not possible.
 *
 * except in extfmt mode,
 * updates asb->sb_ino to create an (inode number,device number) pair that
 * is unique for each set of hard linked files in the archive,
 * while keeping the inode number <16bits as long as possible.
 * before calling linkto, asb->sb_ino should contain the inode number
 * from the filesystem, as obtained with a stat call.
 */
STATIC Link *
linkto (name, asb)
     char *name;
     reg Stat *asb;
{
  reg Link *linkp;
  reg Path *path;
  reg Link **abase;
  ino_t ino;
  int ino16;

  if ((asb->sb_mode & S_IFMT) == S_IFDIR)
    return (NULL);

  if(!extfmt)
  {
    /* the old ASCII format only stores the least significant 16
       bits of the inode.  We need to avoid duplicate ino numbers
       for different files in the archive or else the restore can
       have all kinds of horrible failure modes */
    
    /* first, try the inode that is the inode from the fs, truncated */
    ino = asb->sb_ino & 0x0ffff;
    
    /* check if that has been used, if so choose other one */
    while(1)
      {
	ino16=ino&0xffff;
	
	/* if the bit for ino16 is 0, then this value has not been used yet
	   in the archive, so we can break out of the loop and use it */
	if((ino16bitused[ino16/8]&((unsigned char)(1)<<(ino16&0x7)))==0) break;
	
	/* choose other one */
	ino=freshino++;

	if(freshino>0xffff) 
	  {
	    /* break out of loop, no need to check if has been used */
	    break;
	  }
      }

  }
  else
  {
    ino = asb->sb_ino;
  }
  
  /* add this number to the table of used 16-bit inode numbers */
  /* this operation is unneccessary in some cases of the control
     flow of the above if statements, but it is nice to put it here */
  ino16=ino&0xffff;      
  ino16bitused[ino16/8]=ino16bitused[ino16/8]|((unsigned char)(1)<<(ino16&0x7));
  /* add this inode to table of hard links (under filesystem inode number) */

  if(    ((linkp = (Link *) memget (sizeof (Link))) == NULL)
      || ((path = (Path *) memget (sizeof (Path))) == NULL)
      || ((path->p_name = memstr (name)) == NULL))
    return (NULL);

  linkp->l_dev = asb->sb_dev;
  linkp->l_ino = asb->sb_ino; /* original ino from fs */
  linkp->l_nlink = asb->sb_nlink - 1;
  linkp->l_size = asb->sb_size;
  linkp->l_mtime = asb->sb_mtime;
  linkp->l_mode = asb->sb_mode;
  linkp->l_path = path;
  path->p_forw = NULL;
  path->p_back = path;

  linkp->l_ino_ar = asb->sb_ino = ino;

  if ((linkp->l_forw = *(abase = linkhash (linkp->l_ino))))
    linkp->l_forw->l_back = linkp;
  linkp->l_back = NULL;

  return (*abase = linkp);
}

#ifndef	MEMCPY

/*
 * memcpy()
 *
 * A simple block move.
 */
STATIC char *
memcpy (to, from, len)
     reg char *to;
     reg char *from;
     uint len;
{
  reg char *toend;

  for (toend = to + len; to < toend; *to++ = *from++)
    ;
  return (to);
}

#endif /* MEMCPY */

/*
 * memget()
 *
 * Allocate memory.
 */
STATIC char *
memget (len)
     uint len;
{
  reg char *mem;
  static short outofmem;

  if ((mem = malloc (len)) == NULL && !outofmem)
    outofmem = warn ("memget()", "Out of memory");
  return (mem);
}

/*
 * memstr()
 *
 * Duplicate a string into dynamic memory.
 */
STATIC char *
memstr (str)
     reg char *str;
{
  reg char *mem;

  if ((mem = memget ((uint) strlen (str) + 1)))
    VOID strcpy (mem, str);
  return (mem);
}

#ifndef	MKDIR

/*
 * mkdir()
 *
 * Make a directory via "/bin/mkdir". Sets errno to a
 * questionably sane value upon failure.
 */
STATIC int
mkdir (name, mode)
     reg char *name;
     reg ushort mode;
{
  reg int pid;

  if ((pid = xfork ("mkdir()", DIE)) == 0)
    {
      VOID close (fileno (stdin));
      VOID close (fileno (stdout));
      VOID close (fileno (stderr));
      VOID open ("/dev/null", O_RDWR);
      VOID dup (fileno (stdin));
      VOID dup (fileno (stdin));
      VOID umask (~mode);
      VOID execl ("/bin/mkdir", "mkdir", name, (char *) NULL);
      exit (1);
    }
  if (xwait (pid, "mkdir()", TRUE) == 0)
    return (0);
  errno = EACCES;
  return (-1);
}

#endif /* MKDIR */


/*
 * nameopt()
 *
 * Optimize a pathname. Confused by "<symlink>/.." twistiness.
 * Returns the number of final pathname elements (zero for "/"
 * or ".") or -1 if unsuccessful.
 */
STATIC int
nameopt (begin)
     char *begin;
{
  reg char *name;
  reg char *item;
  reg int idx;
  int absolute;
  auto char *element[PATHELEM];

  absolute = (*(name = begin) == '/');
  idx = 0;
  for (;;)
    {
      if (idx == PATHELEM)
	return (warn (begin, "Too many elements"));
      while (*name == '/')
	++name;
      if (*name == '\0')
	break;
      element[idx] = item = name;
      while (*name && *name != '/')
	++name;
      if (*name)
	*name++ = '\0';
      if (strcmp (item, "..") == 0)
	if (idx == 0)
	  if (absolute)
	    ;
	  else
	    ++idx;
	else if (strcmp (element[idx - 1], "..") == 0)
	  ++idx;
	else
	  --idx;
      else if (strcmp (item, ".") != 0)
	++idx;
    }
  if (idx == 0)
    element[idx++] = absolute ? "" : ".";
  element[idx] = NULL;
  name = begin;
  if (absolute)
    *name++ = '/';
  for (idx = 0; (item = element[idx]); ++idx, *name++ = '/')
    while (*item)
      *name++ = *item++;
  *--name = '\0';
  return (idx);
}

/*
 * next()
 *
 * Advance to the next archive volume.
 *
 */
STATIC void
next (mode, why)
     reg int mode;
     reg char *why;
{
  reg time_t began;
  char *msg;
  int msgsize;
  char answer[20];
  int ttyfd;
  char *ttystr;

  msgsize = 200 + strlen(myname) * 2 + strlen(arspec);
  if(promptscript) msgsize += strlen(promptscript);
  msg = memget (msgsize);

  began = time ((time_t *) NULL);
  nextclos ();
  if(!index(ignorewarnings,(int)'M'))
  {
    VOID warnarch (why, (off_t) 0);
    warnings--; /* above warnach call is not an error condition */
  }
  if (arfd == STDIN || arfd == STDOUT)
    goodbye (1);
  ++arvolume;			/* change disk # here */

  if (email) mail(email,(int)arvolume,arspec);

  if(promptscript) 
  {
 
      /* run info-script with volume number and archive spec,
         and reason for calling it as arguments.
	 the script should return 0 for ok and 1 for abort, other 
	 return codes will be treated as errors. */

      /* connect prompt script to /dev/tty if it can be opened, else
         connect it to /dev/null (for crontab/at job use) */
      if ((ttyfd = open (TTY, O_RDWR)) < 0)
	{
	  ttystr="/dev/null";
	}
      else
	{
	  close(ttyfd);
	  ttystr=TTY;
	}

      VOID sprintf(msg,"%s %u %s '%s' <%s",promptscript,arvolume,arspec,why,ttystr);
      for (;;)
      {
	  auto int result;
	  	  
	  result=system(msg);
	  if (result==1)
	      fatal(arspec,"Aborted");
	  if (result!=0)
	      fatal(arspec,"Promptscript failed");
	  if (nextopen(mode)==0)
	      break;
      }
      
  }
  else
  {
      if(Fflag)
      {
	  VOID sprintf (msg, "\
%s: Ready for disk %u on %s\n\
%s: \"quit\" to abort,\"f\" to format, anything else to proceed. > \07",
			myname,
			arvolume,
			arspec,
			myname);
      }
      else
      {
	  VOID sprintf (msg, "\
%s: Ready for volume %u on %s\n\
%s: \"quit\" to abort, anything else to proceed. > \07",
			myname,
			arvolume,
			arspec,
			myname);
      }
      
      for (;;)
      {
	  nextask (msg, answer, sizeof (answer));
	  if (strcmp (answer, "quit") == 0)
	      fatal (arspec, "Aborted");
	  else
	  {
	      while (Fflag && strcmp(answer,"f") == 0) {
		  fprintf (stderr, "formating using %s ...\n",formatcmd);
		  if (system (formatcmd) != 0)
		  {
		      strcpy(msg,"\n");
		      strcat(msg,myname);
		      strcat(msg,": Format failed! Try again (y/n)? > ");
		      nextask (msg, answer, sizeof (answer));
		      if (answer[0] == 'y') answer[0] = 'f';
		      else exit(1);
		  }
		  else break;
	      }
	      
	      if (nextopen (mode) == 0)
		  break;
	  }
      }
  }
  if(!index(ignorewarnings,(int)'M'))
    {
      VOID warnarch ("Continuing", (off_t) 0);
      warnings--; /* above warnach call is not an error condition */
    }
  timewait += time ((time_t *) NULL) - began;
  free(msg);
}

/*
 * nextask()
 *
 * Ask a question and get a response. Ignores spaces and tabs.
 */
STATIC void
nextask (msg, answer, limit)
     reg char *msg;
     reg char *answer;
     reg int limit;
{
  reg int idx;
  reg int got;
  auto char c;

  if (ttyf < 0)
    ttyf = openqtty ();
  if (ttyf < 0)
    fatal (TTY, "Unavailable");
  VOID writeall (ttyf, msg, (uint) strlen (msg));
  idx = 0;
  while ((got = read (ttyf, &c, 1)) == 1)
    if (c == '\04' || c == '\n')
      break;
    else if (c == ' ' || c == '\t')
      continue;
    else if (idx < limit - 1)
      answer[idx++] = c;
  if (got < 0)
    fatal (TTY, syserr ());
  answer[idx] = '\0';
}

/*
 * nextclos()
 *
 * Close an archive.
 */
STATIC void
nextclos ()
{
  if (arfd != STDIN && arfd != STDOUT && !fflag)
    {
      VOID close (arfd);
      arfd = -1;
    }
  areof = 0;
  if (arname && *arname == '!')
    pipewait ();

  /* the volume is complete, need to adjust the volume limit for the
   * following volume.
   */
  if (aruntil != 0)
    {
      update_aruntil();
      if (aruntil == 0)  askfornext = 1;

      if( roundaruntil )
	{
	  /* round aruntil down to a multiple of arbsize: some devices
	     (like ftape) puke on a smaller-than-blocksize last write to
	     the volume */
	  aruntil = ( aruntil / (ulonglong) arbsize );
	  aruntil = aruntil * arbsize;
	}

      /* error check... */
      if (aruntil && (aruntil < arbsize))
      {
	  fatal(arspec,"-s option value for this volume smaller than block (-b) size");
      }

  }
}

/*
 * nextopen()
 *
 * Open an archive. Returns 0 if successful, -1 otherwise.
 */
STATIC int
nextopen (mode)
     int mode;
{
#if 1
  if (arfd != -1)
    close (arfd);
#endif
  if (*arname == '!')
    arfd = pipeopen (mode);
  else if (strcmp (arname, "-") == 0)
    arfd = mode ? STDOUT : STDIN;
  else
    {
#ifdef	CTC3B2
      if (Cflag)
	{
	  reg int oops;
	  reg int fd;

	  oops = ((fd = open (arname, O_RDWR | O_CTSPECIAL)) < 0
		  || ioctl (fd, STREAMON) < 0);
	  VOID close (fd);
	  if (oops)
	    return (warnarch (syserr (), (off_t) 0));
	}
#endif /* CTC3B2 */

#ifdef linux
      /* Do O_SYNC writing to the floppy drive */
      if(Fflag && mode) 
        arfd = open (arname, mode | O_CREAT | O_TRUNC | O_SYNC, 0666 & ~mask);
      else
        arfd = mode ? creat (arname, (mode_t)(0666 & ~mask)) : open (arname, mode);
#else
      arfd = mode ? creat (arname, (mode_t)(0666 & ~mask)) : open (arname, mode);
#endif /* linux */

    }
  if (arfd < 0)
    return (warnarch (syserr (), (off_t) 0));
  arleft = aruntil;
  return (0);
}

/*
 * openin()
 *
 * Open the next input file. Returns a file descriptor, 0 if no data
 * exists, or -1 at EOF. This kludge works because standard input is
 * in use, preventing open() from returning zero.
 *
 * if fsname is non-NULL, copy filename on local filesystem into it,
 * if not a control file, for later use by -a option.
 */
STATIC int
openin (name, fsname, asb, cratio)
     char *name,*fsname;
     reg Stat *asb;
     int *cratio;
{
  int fd;
  auto char local[PATHSIZE];
  char *label;

  if(fsname) *fsname='\0';

  if (cratio)
    *cratio = 100;
  for (;;)
    {
      if(firstfilename[0]!=0)
	{
	  /* special case to suport input sanity check code in main() */
	  strcpy(name,firstfilename);
	  firstfilename[0]=0;
	}
      else
	{
	  if (lineget (stdin, name, PATHSIZE) < 0)
	    return (-1);
	}

      /* control file syntax is //--SOURCEFILE[ LABEL] */
      if(strncmp(name,"//--",4)==0)
      {
	  strcpy(local,&name[4]);
	  
	  label=index(local,(int)' ');
	  if(label!=NULL)
	  {
	      *label='\000';	/* separate filename and label */
	      label++;
	  }
	  else
	      label=NOLABEL;
	  if(*label=='\000') label=NOLABEL;

	  sprintf(name,"%s/%s",CONTROLNAME,label);

	  /* Now, local is the filename, label is the label, and name is
             the filename in the archive, with the label embedded.
          */

	  if (STAT (local, asb) < 0)
	  {
	      VOID warn (local, syserr ());
	      continue;
	  }

          /* Sun machines put trash in sb_rdev in stead of setting it to 0
	     for regular files.  Afio wants a 0 */
          if(! (S_ISBLK(asb->sb_mode)||S_ISCHR(asb->sb_mode)) ) asb->sb_rdev=0;

	  if ((asb->sb_mode & S_IFMT) != S_IFREG)
	  {
	      VOID warn (local, "Cannot read control file from this source.");
	      continue;
	  }

	  /* Flag that this is a control file.
	     An archive entry is a control file if it is a regular file and
             if the ISCONTROL flag is set.  The filename is not important.
	  */
	  asb->sb_rdev |= RDEV_ISCONTROL;

	  if (asb->sb_size == 0)
	      return (0);
	  if ((fd = open (local, O_RDONLY)) >= 0)
	      return (fd);
	  VOID warn (local, syserr ());
	  continue;	  
      }

      /* not a control file, make a normal archive entry */

      if (namecmp (name,NULL) < 0)
 	  continue;
      if (nameopt (name) < 0)
	  continue;
      if (!gflag)
	  VOID strcpy (local, name);
      else if (dirchg (name, local) < 0)
	  continue;

      if(fsname) strcpy(fsname,local); /* for use by utime() in -a option */

      if ((hflag ? STAT (local, asb) : LSTAT (local, asb)) < 0)
	{
	  VOID warn (name, syserr ());
	  if(index(ignorewarnings,(int)'m')) warnings--;
#ifdef EOVERFLOW
	  /* try to catch (in a portable way we hope) the case of
             statting a >2GB file with a regular 4-byte off_t stat
             call, this gives an EOVERFLOW on Linux.  We do not want
             to count this case as a missing file, so print extra
             warning so that warnings counter gets increased properly.  */
	  if(errno==EOVERFLOW) 
	      VOID warn (name, "Can't archive file with size >= 2GB");
#endif
	  continue;
	}

      /* Sun machines put trash in sb_rdev in stead of setting it to 0
	 for regular files.  Afio wants a 0 */
      if(! (S_ISBLK(asb->sb_mode)||S_ISCHR(asb->sb_mode)) ) asb->sb_rdev=0;

      switch (asb->sb_mode & S_IFMT)
	{
	case S_IFDIR:
          asb->sb_nlink = 1;
	  asb->sb_size = 0; 
	  return (0);
#ifdef	S_IFLNK
	case S_IFLNK:
	  if ((asb->sb_size = readlink (local,
			      asb->sb_link, sizeof (asb->sb_link) - 1)) < 0)
	    {
	      VOID warn (name, syserr ());
	      continue;
	    }
	  asb->sb_link[asb->sb_size] = '\0';
	  return (0);
#endif /* S_IFLNK */
	case S_IFREG:
	  if (asb->sb_size == 0)
            {
              /* indicate that file is not suitable for compression */
	      asb->sb_rdev |= RDEV_NOTCOMPR; 
    	      return (0);
	    }

	  if (cpiocompat && (asb->sb_size != (asb->sb_size&0x7fffffff)))
	    {
	      /* >=2GB file.  Can't handle this and remain cpio compatible */
	      VOID fatal (name, "Archiving file with size >= 2GB not allowed by -5 option");
	      continue;	      
	    }

	  if ((fd = open (local, O_RDONLY)) >= 0)
	    {
	      if (Zflag)
		compressfile (&fd, name, asb, cratio);

	      return (fd);
	    }
	  VOID warn (name, syserr ());
	  break;
	default:
	  asb->sb_size = 0;
	  return (0);
	}
    }
}

/*
 * openincheck()
 *
 * Open the input file. Returns a file descriptor, 0 if no data
 * exists, or -1 at EOF. This kludge works because standard input is
 * in use, preventing open() from returning zero.
 */
STATIC int
openincheck (name, asb, comp, dozflag)
     char *name;
     reg Stat *asb;
     int *comp;
     int dozflag;
{
  int fd;
  auto char local[PATHSIZE];
  auto char archlink[PATHSIZE];
  char* namedot;

  *uncompto = '\0';
  *comp = 0;
  if (dozflag && ((asb->sb_mode & S_IFMT) == S_IFREG) &&
      ((namedot = strrchr (name, '.')) != NULL) &&
      (asb->sb_rdev & RDEV_NOTCOMPR) == 0 &&
      (asb->sb_size > 0) &&
      (strcmp (namedot, ".z") == 0))
    {
      *namedot = '\0';
      strcpy(uncompto, name);
      *comp = 1;
    }
  else
    namedot = NULL;		/* not uncompressing */
  if (nameopt (name) < 0)
    return -1;
  if (!gflag)
    VOID strcpy (local, name);
  else if (dirchg (name, local) < 0)
    return 0;
  switch (asb->sb_mode & S_IFMT)
    {
    case S_IFDIR:
      asb->sb_nlink = 1;
      asb->sb_size = 0;
      /* do not check if directory exists */
      /* do not check directory permissions and access times */
      return (0);
#ifdef	S_IFLNK
    case S_IFLNK:
      /* if sb_size==0 then it is a hardlink to a symlink earlier
         in the archive, so don't check. */
      if (asb->sb_size == 0)
	return (0);

      strcpy(archlink,asb->sb_link);

      if ((asb->sb_size = 
             readlink (local, asb->sb_link, sizeof (asb->sb_link) - 1)) < 0)
	{
	  VOID warn (name, syserr ());
	  asb->sb_size = 0; 
	  return 0;
	}
      asb->sb_link[asb->sb_size] = '\0';
      
      if(strcmp(archlink,asb->sb_link)!=0)
	  VOID warn (name, "Difference in symlink destination filename");

     asb->sb_size = 0;
     return (0);
#endif /* S_IFLNK */
    case S_IFREG:
      /* does not check file permissions and access times */
      if (asb->sb_size == 0)
	return (0);

      /* get last access time if we want to restore it */
      if(aflag)
      {
	  if (STAT (local, &atime_sb) < 0)
	  {
	      /* silently fail, the open below will report an error */
	  }
	  else
	  {
	      atime_sb_valid=1;
	  }	  
      }

      if ((fd = open (local, O_RDONLY)) >= 0)
	{
	  return (fd);
	}
      VOID warn_nocount (name, syserr ());
      VOID warn (name, "File on filesystem could not be opened for verify");
      if(index(ignorewarnings,(int)'r'))
	{
	  warn_nocount(name, "Not counting this as a verification error");
	  warnings--;
	}

      return 0;
    default:
      asb->sb_size = 0;
      return (0);
    }
}

/*
 * opencontrolscript()
 *
 * Start a control script. Returns the stdin of the script,
 * -1 if unsuccessful.
 * If no control script option given, return file handle of /dev/null.
 */
STATIC int
opencontrolscript (char *name)
{
    auto int pfd[2];
    int comppid;
    char *label;

    *uncompto='\000';  /* for -v output on restore, verify */

    if(controlscript == NULL )
    {
	/* ignore data */
	warnarch("No -D option given, ignoring control file.",(off_t)0);
	return open("/dev/null",O_WRONLY);
    }

    if(strcmp(controlscript,"")==0)
    {
	/* silently ignore data */
	return open("/dev/null",O_WRONLY);
    }

    label=index(name,(int)'/');
    if(label==NULL) label=NOLABEL; else label++;
    
    if (pipe (pfd) < 0) 
    {
	warn("Control script",syserr());
	return -1;
    }

    if ((comppid = xfork ("opencontrolscript(in), running control script", NODIE)) == 0)
    {
        /* Ignoring SIGPIPE may cause strange results in some shell scripts */
	VOID signal (SIGPIPE, SIG_DFL);

	if (arfd != STDIN && arfd != STDOUT) VOID close (arfd);
	VOID close (pfd[1]);	/* child */
	VOID close (fileno (stdin));
	VOID dup (pfd[0]);
	VOID close (pfd[0]);
	
        execlp (controlscript, controlscript, label, NULL);

	warnarch("Problems running control script:",(off_t)0);	       
	warn(controlscript,syserr());
	exit (1);
    }

    /*parent*/
    
    if (comppid < 0)
    {
	warn("Control script",syserr());
	return -1;
    }

    close (pfd[0]);

    /* this enables some safety checks */
    uncompressrun = comppid;

    return pfd[1];
}

/*
 * openotty()
 *
 * Open an output file. Returns the output file descriptor,
 * 0 if no data is required or -1 if unsuccessful. Note that
 * UNIX open() will never return 0 because the standard input
 * is in use.
 */
STATIC int
openotty (name, asb, linkp, ispass, dozflag)
     char *name;
     reg Stat *asb;
     Link *linkp;
     reg int ispass;
     int dozflag;
{
  reg int exists;
  reg int fd;
  reg ushort perm;
  ushort operm=0; /* this one and next two initialised to */
  uid_t ouid=9999; /* suppress compiler warnings */
  gid_t ogid=9999;
  Path *path;
  auto Stat osb;
#ifdef	S_IFLNK
  reg int ssize;
  auto char sname[PATHSIZE];
#endif /* S_IFLNK */
  char *namedot;
  auto int pfd[2];
  int comppid;

  if(ISCONTROL(asb))
      return opencontrolscript(name);
  
  *uncompto = '\0';
  /*
   * -iZ try to uncompress a compress'd regular file
   */
  if (dozflag && !linkp && ((asb->sb_mode & S_IFMT) == S_IFREG) &&
      ((namedot = strrchr (name, '.')) != NULL) &&
      (asb->sb_rdev & RDEV_NOTCOMPR) == 0 &&
      (asb->sb_size > 0) &&
      (strcmp (namedot, ".z") == 0))
    {
      *namedot = '\0';
    }
  else
    namedot = NULL;		/* not uncompressing */
  if ((exists = (LSTAT (name, &osb) == 0))) 
    {
      /* The asb.sb_ino here has not been truncated to 16 bits, so the check is
	 safe and may even add some protection. */
      if (ispass
	  && osb.sb_ino == asb->sb_ino
	  && osb.sb_dev == asb->sb_dev) 
	return (warn (name, "Same file"));
      else if ((osb.sb_mode & S_IFMT) == (asb->sb_mode & S_IFMT))
	operm = osb.sb_mode & (xflag ? S_IPERM : S_IPOPN);
      else if (afremove (name, &osb) < 0)
	return (warn (name, syserr ()));
      else
	exists = 0;
    }
  if (linkp)
    {
      if (exists)
      {
#if 0
 /* Bogus check commented out.
    This check is bogus and dangerous because we only get the
    least significant 16 bits of the ino of the archived file from
    the archive header.  So the file this check would prevent 
    overwriting (replacing with a link) if true will probably be a different
    file altogether, which we wanted overwritten, not preserved.
    Also, the check is bogus anyway when installing the files on a 
    different computer system.  Ho hum.
 */
        printf("asb->sb_ino == osb.sb_ino %d %d\n",asb->sb_ino, osb.sb_ino);
	if (asb->sb_ino == osb.sb_ino
	    && asb->sb_dev == osb.sb_dev)
	  return (0);
	else 
#endif
        if (unlink (name) < 0)
	  return (warn (name, syserr ()));
	else
	  exists = 0;
      }
      for (path = linkp->l_path; path; path = path->p_forw)
	if (link (path->p_name, name) == 0
	    || (errno == ENOENT
		&& dirneed (name) == 0
		&& link (path->p_name, name) == 0))
	  return (0);
	else if (errno != EXDEV)
	  return (warn (name, syserr ()));
      VOID warn (name, "Link broken");
      linkalso (linkp, name);
    }
  perm = asb->sb_mode & (xflag ? S_IPERM : S_IPOPN);
  if (exists)
    {
      ouid = osb.sb_uid;
      ogid = osb.sb_gid;
    }
  switch (asb->sb_mode & S_IFMT)
    {
    case S_IFBLK:
    case S_IFCHR:
      fd = 0;
      /* rdev==0 means that dev_t value does not fit in 16 bits,
	 and is encoded in dev and ino instead
	 see out(). 
      */
      if(asb->sb_rdev==0) asb->sb_rdev=(asb->sb_dev << 16) + asb->sb_ino;

      if (exists) 
	{
	  if (asb->sb_rdev == osb.sb_rdev)
	    if (perm != operm && chmod (name, perm) < 0)
	      return (warn (name, syserr ()));
	    else
	      break;
	  else if (afremove (name, &osb) < 0)
	    return (warn (name, syserr ()));
	  else
	    exists = 0;
	}

      if (mknod (name, asb->sb_mode, asb->sb_rdev) < 0
	  && (errno != ENOENT
	      || dirneed (name) < 0
	      || mknod (name, asb->sb_mode, asb->sb_rdev) < 0))
	return (warn (name, syserr ()));
      break;
    case S_IFDIR:
      if (exists)
	{
	  if (xflag && (asb->sb_uid != ouid || asb->sb_gid != ogid) &&
	      chown (name, asb->sb_uid, asb->sb_gid) < 0)
	    return (warn (name, syserr ()));
	  else if (perm != operm && chmod (name, perm) < 0)
	    return (warn (name, syserr ()));
	  else { ; }
	  if(osb.sb_mtime <= (mflag ? timenow : asb->sb_mtime))
	    savedirstamp(name, mflag ? timenow : asb->sb_mtime);
	}
      else 
	{
	  savedirstamp(name, mflag ? timenow : asb->sb_mtime);
	  if (dirneed (name) < 0 || dirmake (name, asb) < 0)
	    return (warn (name, syserr ()));
	}
     return (0);
#ifdef	S_IFIFO
    case S_IFIFO:
      fd = 0;
      if (exists)
	if (perm != operm && chmod (name, perm) < 0)
	  return (warn (name, syserr ()));
	else { ; }
      else if (mkfifo (name, asb->sb_mode) < 0
	       && (errno != ENOENT
		   || dirneed (name) < 0
		   || mkfifo (name, asb->sb_mode) < 0))
	return (warn (name, syserr ()));
      break;
#endif /* S_IFIFO */
#ifdef	S_IFSOCK
    case S_IFSOCK:
      fd = 0;
      if (exists)
	if (perm != operm && chmod (name, perm) < 0)
	  return (warn (name, syserr ()));
	else { ; }
      else if (mknod (name, asb->sb_mode, (dev_t) 0) < 0
	       && (errno != ENOENT
		   || dirneed (name) < 0
		   || mknod (name, asb->sb_mode, (dev_t) 0) < 0))
	return (warn (name, syserr ()));
      break;
#endif /* S_IFSOCK */
#ifdef	S_IFLNK
    case S_IFLNK:
      fd = 0;
      if (exists) 
	{
	  if ((ssize = readlink (name, sname, sizeof (sname))) < 0)
	    return (warn (name, syserr ()));
	  else if (strncmp (sname, asb->sb_link, (size_t)ssize) == 0)
	    break;
	  else if (afremove (name, &osb) < 0)
	    return (warn (name, syserr ()));
	  else
	    exists = 0;
	}
      if (symlink (asb->sb_link, name) < 0
	  && (errno != ENOENT
	      || dirneed (name) < 0
	      || symlink (asb->sb_link, name) < 0))
	return (warn (name, syserr ()));
      break;
#endif /* S_IFLNK */
    case S_IFREG:
      if (exists) 
	{
	  if (nflag && osb.sb_mtime > asb->sb_mtime)
	    return (warn_nocount (name, "Newer file exists"));
	  else if (unlink (name) < 0)
	    return (warn (name, syserr ()));
	  else
	    exists = 0;
	}

      if ((fd = creat (name, perm)) < 0
	  && (errno != ENOENT
	      || dirneed (name) < 0
	      || (fd = creat (name, perm)) < 0))
	return (warn (name, syserr ()));
      if (dozflag && !linkp && namedot)
	{
	  if (pipe (pfd) >= 0)
	    {
	      if ((comppid = xfork ("openotty(in), compressing", NODIE)) == 0)
		{
		  if (arfd != STDIN && arfd != STDOUT) VOID close (arfd);
		  VOID close (pfd[1]);	/* child */
		  VOID close (fileno (stdin));
		  VOID dup (pfd[0]);
		  VOID close (pfd[0]);

		  VOID close (fileno (stdout));
		  if (dup (fd) < 0)
		    exit (1);
		  VOID close (fd);
		  mayberewind();
		  if(compressargs)
		      execvp (compressprog, compress_arg_list);
		  else
		      execlp (compressprog, compressprog, "-d", "-c", NULL);
		  fprintf (stderr, "Could not uncompress, errno %d\n", errno);
		  exit (1);
	      }
	      else
		  /* life seems ok */
		  if (comppid > 0)
		  {
		      close (fd);
		      fd = pfd[1];
		      close (pfd[0]);
		      uncompressrun = comppid;
		      strcpy (uncompto, name);
#if 0
		      *namedot = '.';
#endif
		      break;
		  }
	  }
	  /* we failed try again, not uncompressing the file */
	  unlink (name);
	  *namedot = '.';
	  return (openotty (name, asb, linkp, ispass, FALSE));
	}

      break;
    default:
      return (warn (name, "Unknown filetype"));
    }

  /* Can't chown()/chmod() a symbolic link, but for all others... */
  if((asb->sb_mode & S_IFMT) != S_IFLNK)
    {
      if (xflag && (!exists || asb->sb_uid != osb.sb_uid
		    || asb->sb_gid != osb.sb_gid)) 
	{
	  if (chown (name, (uid == 0 ? asb->sb_uid : uid),
		     asb->sb_gid))
	    perror (name);
	  else /* chown may have cleared suid/sgid flags; restore them */
	    if(perm&S_IPEXE)
	      if(chmod (name, perm) < 0)
		return (warn (name, syserr ()));
	}
    }
  else
    {
      /* but if we have an lchown call, we _can_ chown a symbolic link.. */
      /* note: the lchown call is like chown, except that lchown does
	 not follow a symlink.  lchown is not present in all unix systems.
	 Therefore, an ifdef is used below, the symbol is defined in the
	 makefile */
#ifdef HAVE_LCHOWN      
      if (xflag && (!exists || asb->sb_uid != osb.sb_uid
		    || asb->sb_gid != osb.sb_gid)) 
	{
	  if (lchown (name, (uid == 0 ? asb->sb_uid : uid),
		     asb->sb_gid))
	    perror (name);
	}
#endif
    }

  if (linkp == NULL && asb->sb_nlink > 1)
    VOID linkto (name, asb);
  return (fd);
}

/*
 * openqtty()
 *
 * Open the terminal for interactive queries (sigh). Assumes that
 * background processes ignore interrupts and that the open() or
 * the isatty() will fail for processes which are not attached to
 * terminals. Returns a file descriptor (-1 if unsuccessful).
 */
int
openqtty (void)
{
  reg VOIDFN (*intr) (int);
  int fd;

  fd = -1;
  if (!Fflag)
    {
      if ((intr = signal (SIGINT, SIG_IGN)) == SIG_IGN)
	return (fd);
      VOID signal (SIGINT, intr);
    }

  if ((fd = open (TTY, O_RDWR)) < 0)
    {
      VOID warn (TTY, syserr ());
    }
  else if (!isatty (fd))
    VOID warn (TTY, "Is not a tty");
  return (fd);
}

/*
 * options()
 *
 * Decode most reasonable forms of UNIX option syntax. Takes main()-
 * style argument indices (argc/argv) and a string of valid option
 * letters. Letters denoting options with arguments must be followed
 * by colons. With valid options, returns the option letter and points
 * "optarg" at the associated argument (if any). Returns '?' for bad
 * options and missing arguments. Returns zero when no options remain,
 * leaving "optind" indexing the first remaining argument.
 */
STATIC int
options (ac, av, proto)
     int ac;
     register char **av;
     char *proto;
{
  register int c;
  register char *idx;
  static int optsub;

  if (optind == 0)
    {
      optind = 1;
      optsub = 0;
    }
  optarg = NULL;
  if (optind >= ac)
    return (0);
  if (optsub == 0 && (av[optind][0] != '-' || av[optind][1] == '\0'))
    return (0);
  switch (c = av[optind][++optsub])
    {
    case '\0':
      ++optind;
      optsub = 0;
      return (options (ac, av, proto));
    case '-':
      ++optind;
      optsub = 0;
      return (0);
    case ':':
      return ('?');
    }
  if ((idx = strchr (proto, c)) == NULL)
    return ('?');
  if (idx[1] != ':')
    return (c);
  optarg = &av[optind][++optsub];
  ++optind;
  optsub = 0;
  if (*optarg)
    return (c);
  if (optind >= ac)
    return ('?');
  optarg = av[optind++];
  return (c);
}

/*
 * optsize()
 *
 * Interpret a "size" argument. Recognizes suffices for blocks
 * (512-byte), kilobytes, megabytes, gigabytes and blocksize. Returns
 * the size in bytes.
 */
STATIC ulonglong
optsize (str)
     char *str;
{
  reg char *idx;
  ulonglong number;
  ulonglong result;

  result = 0;
  idx = str;
  for (;;)
    {
      number = 0;
      while (*idx >= '0' && *idx <= '9')
	number = number * 10 + *idx++ - '0';
      switch (*idx++)
	{
	case 'b':
	  result += number * (ulonglong)512;
	  continue;
	case 'k':
	  result += number * (ulonglong)1024;
	  continue;
	case 'm':
	  result += number * (ulonglong)(1024L * 1024L);
	  continue;
	case 'g':
	  result += number * (ulonglong)(1024L * 1024L * 1024L);
	  continue;
	case 'x':
	  result += number * (ulonglong)arbsize;
	  continue;
	case '+':
	  result += number;
	  continue;
	case '\0':
	  result += number;
	  break;
	default:
	  break;
	}
      break;
    }
  if (*--idx)
    fatal (str, "Unrecognizable value");
  return (result);
}

/*
 * out()
 *
 * Write an archive.
 */
STATIC VOIDFN
out (av)
     char **av;
{
    reg int fd;
    auto Stat sb;
    auto char name[PATHSIZE];
    auto char fsname[PATHSIZE];
    auto int compression;
#ifdef linux_tstamp
    auto struct utimbuf tstamp;
#else
    auto time_t tstamp[2];
#endif
    int wantlarge;
    Link *linkp;
    
    if (*av)
	fatal (*av, "Extraneous argument");
    
    while ((fd = openin (name, fsname, &sb, &compression)) >= 0)
    {
	bytepos=total; /* offset of this file in archive */
	
	sb.ino_orig=sb.sb_ino;
	if (!lflag && sb.sb_nlink > 1)
	{
	    if ((linkp = linkfrom (&sb,0)))
            {
             /* this file is a had link to a file we stored before */
		sb.sb_size = 0;
		sb.sb_ino=linkp->l_ino_ar;
            }
	    else
	    {
	      /* linkto also updates sb.sb_ino to equal l_ino_ar */
	      VOID linkto (name, &sb);
	    }
	}

	/* Some systems have a dev_t larger than 16 bits.  If the
           dev_t value is larger, then encode it in the dev and ino fields
	   of the archive header, and set rdev to 0 to signify special
	   case. */
	/* for symmetry, and to reduce the amount of conditional logic
           needed in afio, this re-encoding measure is also used if we
           produce an outhead3 large ascii header, even though the
           outhead3 header has more room in its rdev field.  */

        if( (S_ISBLK(sb.sb_mode)||S_ISCHR(sb.sb_mode))&&
	   ( ((sb.sb_rdev|0xffff)!=0xffff) || (sb.sb_rdev==0)) )
	  {	    
	    sb.sb_dev=sb.sb_rdev>>16;
	    sb.sb_ino=sb.sb_rdev&0xffff;
	    sb.sb_rdev=(dev_t)0; 
	  }

	/* calculate if we want to use a large ASCII header.
	 * Use them only when the data won't fit into the
	 * default header.
	 */
	wantlarge=(sb.sb_size != (sb.sb_size&0x7FFFFFFF));

	/* also need large archive header to correctly store
	   hard linked files (if inode numbers assigned by linkto()
	   go above 16 bits) */

	if (!lflag && (sb.sb_nlink > 1) && !extfmt)
	  wantlarge = wantlarge || ((sb.sb_ino&0x0ffff)!=sb.sb_ino);

	/* also need it for big uid/gid values */
	wantlarge=wantlarge || ((sb.sb_uid&0x0ffff)!=sb.sb_uid);
	wantlarge=wantlarge || ((sb.sb_gid&0x0ffff)!=sb.sb_gid);

	/* When on a system with sizeof(time_t)>4bytes, and if the time
	   on the file is >2038, also use large archive header
	   (Linux right now has a 4-byte time_t, but we want to be
	   future-proof.)
	*/
	wantlarge=wantlarge || ((sb.sb_mtime&0x7fffffff)!=sb.sb_mtime);

	if(cpiocompat && wantlarge)
	  {
	    /* if this was the >2GB case we already had a fatal elsewhere.. */
	    VOID fatal (name, "Cannot create cpio-compatible file header for this input, aborting because of -5 option");
	  }

	if(wantlarge && !cpiowarned && !index(ignorewarnings,(int)'C'))
	  {
	    VOID warn_nocount(name, "Cannot create cpio-compatible file header for this input");
	    VOID warnarch("Continuing, archive will not be fully compatible with cpio or afio versions 2.4.7 and lower",(off_t)0);
	    if(index(ignorewarnings,(int)'c')) warnings--;
	    cpiowarned=1;
	  }
       
 	if(wantlarge) 
	  {	   
	    outhead3 (name, &sb); 
	  } 
	else
	  {
	    if(extfmt) 
	      outhead2 (name, &sb); 
	    else
	      outhead (name, &sb);
	  }

	if (fd)
	  {
	    if (fd==ZIPFD) 
	      {
		outdatazip(zipfdfd,name,sb.sb_size);
		close(zipfdfd);
	      } 
	    else if (fd==MEMFD) 
	      {
		outdatamem(name,sb.sb_size);
	      }
	    else
	      VOID close (outdata (fd, name, sb.sb_size));
	  }
	
	if ((vflag>1)&&((arfd!=STDOUT)))
 	{
	  /* You can use -vv for full ls-style listings on stdout. */
	  char toc_name[PATHSIZE];
	  Stat toc_sb;
	  /* Copy data structures first, because tocentry modifies them */
	  strncpy(toc_name,name,PATHSIZE);
	  memcpy(&toc_sb,&sb,sizeof(Stat));
	  tocentry (toc_name, &toc_sb);
	} 
	else if (vflag)
	  {
	    if(printbytepos) fprintf(stderr,"%.0f ",(double)bytepos);
	    
	    if ((name[0] == '/') && !abspaths && (name[1]!=0))
		fprintf (stderr, "%s -- ", &name[1]); 
	    else
		fprintf (stderr, "%s -- ", name); 
	    
	    /* We do not check for a broken pipe on stderr, we wouldn't want
	       to stop making the archive if this happens.
	       */

	    if ((fd==ZIPFD)||(fd==MEMFD))
		VOID fprintf (stderr, "(%02d%%)\n", compression);
	    else
		VOID fputs ("okay\n", stderr);
	}
	
	/* ASX check if file changed between the begining 
	   and end of the backup */
	if (*fsname)
	{
	    struct stat st;
	    /* I must check fsname ! 
	       but error must be reported as fsname or name ????? 
	       I chosed to use fsname */
	    if ((hflag ? stat(fsname, &st) : lstat(fsname, &st))<0)
	    {
	        warn (fsname, syserr());
	    }
	    else
	    {
	        if (st.st_mtime!=sb.sb_mtime)
	        {
	            warn (fsname, "File was modified during its backup");
	        }
	    }
	}
	else
	{
	    warn (name, "ASX no fsname for this name ??");
        }
	
	if(aflag && *fsname && ((sb.sb_mode & S_IFMT)==S_IFREG))
	{
	    /* reset access time, this distroys the ctime btw. */
#ifdef linux_tstamp
	    tstamp.actime = sb.sb_atime;
	    tstamp.modtime = sb.sb_mtime;
	    VOID utime (fsname, &tstamp);
#else
	    tstamp[0] = sb.sb_atime;
	    tstamp[1] = sb.sb_mtime;
	    VOID utime (fsname, tstamp);
#endif
	}
    }
    outeof (TRAILER, TRAILZ);
}

/*
 * outalloc()
 *
 * Allocate buffer space previously referenced by outavail().
 */
STATIC void
outalloc (len)
     reg uint len;
{
  bufidx += len;
  total += len;
}

/*
 * outavail()
 *
 * Index buffer space for archive output. Stores a buffer pointer
 * at a given location. Returns the number of bytes available.
 */
STATIC uint
outavail (bufp)
     reg char **bufp;
{
  reg uint have;

  while ((have = bufend - bufidx) == 0)
    { 
      outflush (NOTDONE);
    }

  *bufp = bufidx;
  return (have);
}

/*
 * outdata()
 *
 * Write archive data. Continues after file read errors, padding with
 * null characters if neccessary. Always returns the given input file
 * descriptor.
 */
STATIC int
outdata (fd, name, size)
     int fd;
     char *name;
     reg off_t size;
{
  reg uint chunk;
  reg int got;
  reg int oops;
  reg uint avail;
  auto char *buf;

  oops = got = 0;
  while (size)
    {
      avail = outavail (&buf);
      size -= (chunk = size < (off_t)avail ? (uint) size : avail);
      if (oops == 0 && (got = readall (fd, buf, chunk)) < 0)
	{
	  oops = warn (name, syserr ());	  
	  if(index(ignorewarnings,(int)'n')) warnings--;
	  got = 0;
	}
      if ((uint)got < chunk)
	{
	  if (oops == 0)
	    oops = warn (name, "Early EOF");
	  while ((uint)got < chunk)
	    buf[got++] = '\0';
	}
      outalloc (chunk);
    }
  return (fd);
}

/*
 * outdatazip()
 *
 * Write archive data. Continues after file read errors, padding with
 * null characters if neccessary.
 *
 * Special version for reading from gzip through a pipe.
 */
void
outdatazip (fd, name, size)
     int fd;
     char *name;
     reg off_t size;
{
  reg uint chunk;
  reg int got;
  reg uint avail;
  auto char *buf;
  char localbuf[4096];
  int overflow=0;

  while (size>0)
    {
      avail = outavail (&buf);
      chunk = (size < (off_t)avail ? (uint) size : avail);

      got = read (fd, buf, chunk);
      if(got<=0) break;

      outalloc ((uint)got);
      size-=got;
    }
  
  /* read the end of the stream, if the data changed on the second gzip */
  overflow=0;
  while (read (fd, localbuf, sizeof(localbuf))>0) overflow=1;

  waitforgzip(); /* wait for child to exit */

  if(size>0 || overflow)
        warn (name, "Error zipping file, written damaged copy to archive.");

  /* pad with zeros, if necessary */
  while (size>0)
    {
      avail = outavail (&buf);
      size -= (chunk = size < (off_t)avail ? (uint) size : avail);
      got=0;
      while ((uint)got < chunk)
          buf[got++] = '\0';
      outalloc (chunk);
    }

}

/*
 * outdatamem()
 *
 * Write archive data.
 *
 * Special version for reading from internal memory buffer.
 */
void
outdatamem (name, size)
     char *name;
     reg off_t size;
{
  reg uint chunk;
  reg uint got;
  reg uint avail;
  auto char *buf;

  /* read from memory */
  memreset();
  
  while (size)
    {
      avail = outavail (&buf);
      chunk = (size < (off_t)avail ? (uint) size : avail);

      got=memread (buf, (int)chunk);
      if (got==0) break;

      outalloc (got);
      size-=got;
    }

  memfree();
}

/*
 * outeof()
 *
 * Write an archive trailer.
 */
STATIC void
outeof (name, namelen)
     char *name;
     reg uint namelen;
{
  ulonglong pad;
  auto char header[M_STRLEN + H_STRLEN + 1];

  if ((pad = (total + M_STRLEN + H_STRLEN + namelen) % arpad))
    pad = arpad - pad;
  VOID strcpy (header, M_ASCII);
  VOID sprintf (header + M_STRLEN, H_PRINT, 0, 0,
		0, 0, 0, 1, 0, ulo(0) , namelen, (long unsigned int)pad);
  outwrite (header, M_STRLEN + H_STRLEN);
  outwrite (name, namelen);
  outpad ((off_t)pad);
  outflush (DONE);
  if (fflag)
    outwait ();
}

/*
 * outflush()
 *
 * Flush the output buffer. Optionally fork()s to allow the
 * parent to refill the buffer while the child waits for the
 * write() to complete.
 */
STATIC void
outflush (done)
     int done;
{
  int wrstat;

  /*
   * in this case we are a floppy and want to write the floppy from one
   * buffer (to have a copy of the data to verify against)
   */
  bufend = buffer + ((aruntil!=0) ? min ((ulonglong)buflen, arleft) : (ulonglong)buflen);
  if (Fflag && (done == NOTDONE) && ((bufend - bufidx) > 0))
    {
      return;
    }

  /*
   * Otherwise open up the next volume once we've run out of space
   */
  if ( !Fflag && aruntil && arleft == 0)
     next (O_WRONLY, "Output limit reached");

  wrstat = writedisk (1);
  bufend = (bufidx = buffer) + ((aruntil!=0) ? min ((ulonglong)buflen, arleft) : (ulonglong)buflen);
}

/*
 * outhead()
 *
 * Write an archive header.
 */
STATIC void
outhead (name, asb)
     reg char *name;
     reg Stat *asb;
{
  reg uint namelen;
  auto char header[M_STRLEN + H_STRLEN + 1];

  if ((name[0] == '/') && !abspaths)
    {
      if (name[1])
	++name;
      else
	name = ".";
    }
  namelen = (uint) strlen (name) + 1;
  VOID strcpy (header, M_ASCII);
  VOID sprintf (header + M_STRLEN, H_PRINT, ush (asb->sb_dev),
		ush (asb->sb_ino), ush (asb->sb_mode), ush (asb->sb_uid),
		ush (asb->sb_gid), ush (asb->sb_nlink), ush (asb->sb_rdev),
		ulo(mflag ? timenow : asb->sb_mtime), namelen, (long unsigned int) asb->sb_size);
  outwrite (header, M_STRLEN + H_STRLEN);
  outwrite (name, namelen);
#ifdef	S_IFLNK
  if ((asb->sb_mode & S_IFMT) == S_IFLNK)
    outwrite (asb->sb_link, (uint) asb->sb_size);
#endif /* S_IFLNK */
}

/*
 * outhead2()
 *
 * Write an archive header.
 */
STATIC void
outhead2 (name, asb)
     reg char *name;
     reg Stat *asb;
{
  reg uint namelen;
  auto char header[M_STRLEN + H_STRLEN2 + 1];

  if ((name[0] == '/') && !abspaths)
    {
      if (name[1])
	++name;
      else
	name = ".";
    }
  namelen = (uint) strlen (name) + 1;
  VOID strcpy (header, M_ASCII2);
  VOID sprintf (header + M_STRLEN, H_PRINT2, ush (asb->sb_dev),
	        (long unsigned int)(asb->sb_ino), ush (asb->sb_mode), ush (asb->sb_uid),
		ush (asb->sb_gid), ush (asb->sb_nlink), ush (asb->sb_rdev),
		ulo(mflag ? timenow : asb->sb_mtime), namelen, (long unsigned int)(asb->sb_size));
  outwrite (header, M_STRLEN + H_STRLEN2);
  outwrite (name, namelen);
#ifdef	S_IFLNK
  if ((asb->sb_mode & S_IFMT) == S_IFLNK)
    outwrite (asb->sb_link, (uint) asb->sb_size);
#endif /* S_IFLNK */
}

/*
 * outhead3()
 *
 * Write a large ASCII archive header.
 */
STATIC void
outhead3 (name, asb)
     reg char *name;
     reg Stat *asb;
{
  reg uint namelen;
  auto char header[M_STRLEN + H_STRLEN3 + 1];

  if ((name[0] == '/') && !abspaths)
    {
      if (name[1])
	++name;
      else
	name = ".";
    }
  namelen = (uint) strlen (name) + 1;
  VOID strcpy (header, M_ASCII3);

  VOID sprintf (header + M_STRLEN, H_PRINT3, ulo (asb->sb_dev),
		ull (asb->sb_ino), ulo (asb->sb_mode), ulo (asb->sb_uid),
		ulo (asb->sb_gid), ulo (asb->sb_nlink), ulo (asb->sb_rdev),
		mflag ? ull (timenow) : ull (asb->sb_mtime), namelen, 0, 0,
		ull (asb->sb_size));
  outwrite (header, M_STRLEN + H_STRLEN3);
  outwrite (name, namelen);
#ifdef	S_IFLNK
  if ((asb->sb_mode & S_IFMT) == S_IFLNK)
    outwrite (asb->sb_link, (uint) asb->sb_size);
#endif /* S_IFLNK */
}


/*
 * outpad()
 *
 * Pad the archive.
 */
STATIC void
outpad (pad)
     reg off_t pad;
{
  reg int idx;
  reg int len;

  while (pad)
    {
      if ((len = bufend - bufidx) > pad)
	len = pad;
      for (idx = 0; idx < len; ++idx)
	*bufidx++ = '\0';
      total += len;
      outflush (NOTDONE);
      pad -= len;
    }
}

/*
 * outwait()
 *
 * Wait for the last background outflush() process (if any). The child
 * exit value is zero if successful, 255 if a write() returned zero or
 * the value of errno if a write() was unsuccessful.
 */
STATIC void
outwait ()
{
  auto int status;

  if (outpid == 0)
    return;
  status = xwait (outpid, "outwait()", TRUE);
  outpid = 0;
  if (status)
    fatal (arspec, "Child error");
}

/*
 * outwrite()
 *
 * Write archive data.
 */
STATIC void
outwrite (idx, len)
     reg char *idx;
     uint len;
{
  reg uint have;
  reg uint want;
  reg char *endx = idx + len;

  while ((want = endx - idx))
    {
      while ((have = bufend - bufidx) == 0)
	outflush (NOTDONE);
      if (have > want)
	have = want;
      memcpy (bufidx, idx, have);
      bufidx += have;
      idx += have;
      total += have;
    }
}

/*
 * pass()
 *
 * Copy within the filesystem.
 */
STATIC VOIDFN
pass (av)
     reg char **av;
{
  reg int fd;
  reg char **avx;
  auto Stat sb;
  auto char name[PATHSIZE];

  for (avx = av; *avx; ++avx)
    {
      if (gflag && **avx != '/')
	fatal (*avx, "Relative pathname");
      if (STAT (*avx, &sb) < 0)
	fatal (*avx, syserr ());
      if ((sb.sb_mode & S_IFMT) != S_IFDIR)
	fatal (*avx, "Not a directory");
    }
  while ((fd = openin (name, NULL, &sb, (int *) 0)) >= 0)
    {
      if (passitem (name, &sb, fd, av))
	VOID close (fd);
      if (vflag)
	{
	  if (*uncompto)
	    VOID fprintf (stderr, "%s -- uncompressed\n", uncompto);
	  else
	    VOID fprintf (stderr, "%s -- okay\n", name);
	}
    }
}

/*
 * passdata()
 *
 * Copy data to one file. Doesn't believe in input file
 * descriptor zero (see description of kludge in openin()
 * comments). Closes the provided output file descriptor.
 */
STATIC void
passdata (from, ifd, to, ofd)
     char *from;
     reg int ifd;
     char *to;
     reg int ofd;
{
  reg int got;
  reg int sparse;
  auto char block[FSBUF];

  if (ifd)
    {
      VOID lseek (ifd, (off_t) 0, 0);
      sparse = 0;
      while ((got = read (ifd, block, sizeof (block))) > 0
	     && (sparse = fswrite (ofd, block, (uint) got)) >= 0)
	total += got;
      if (got)
	VOID warn (got < 0 ? from : to, syserr ());
      else if (sparse > 0
	       && (lseek (ofd, (off_t) - sparse, 1) < 0
		   || writeall (ofd, block, (uint) sparse) != sparse))
	VOID warn (to, syserr ());
    }
  VOID close (ofd);
}

/*
 * passitem()
 *
 * Copy one file. Returns given input file descriptor.
 */
STATIC int
passitem (from, asb, ifd, dir)
     char *from;
     Stat *asb;
     reg int ifd;
     reg char **dir;
{
  reg int ofd;

#ifdef linux_tstamp
  auto struct utimbuf tstamp;
#else
  auto time_t tstamp[2];
#endif
  auto char to[PATHSIZE];

  while (*dir)
    {
      if (nameopt (strcat (strcat (strcpy (to, *dir++), "/"), from)) < 0)
	continue;
      if ((ofd = openotty (to, asb,
		lflag ? linkto (from, asb) : linkfrom (asb,0), 1, Zflag)) < 0)
	continue;
      if (ofd > 0)
	passdata (from, ifd, to, ofd);
#ifdef linux_tstamp
      tstamp.actime = tstamp.modtime = mflag ? timenow : asb->sb_mtime;
      VOID utime (to, &tstamp);
#else
      tstamp[0] = tstamp[1] = mflag ? timenow : asb->sb_mtime;
      VOID utime (to, tstamp);
#endif
      /* safety */
      if (uncompressrun)
	{
	  VOID xwait (uncompressrun, "passitem xwait()", TRUE);
	  uncompressrun = 0;
	}
    }
  return (ifd);
}

/*
 * pipechld()
 *
 * Child portion of pipeline fork.
 */
STATIC int
pipechld (mode, pfd)
     int mode;
     reg int *pfd;
{
  reg char **av;
  auto char *arg[32];

  av = arg;
  if ((*av = getenv ("SHELL")) && **av)
    ++av;
  else
    *av++ = "/bin/sh";
  *av++ = "-c";
  *av++ = arname + 1;
  *av = NULL;
  if (mode)
    {
      VOID close (pfd[1]);
      VOID close (STDIN);
      VOID dup (pfd[0]);
      VOID close (pfd[0]);
      VOID close (STDOUT);
      VOID open ("/dev/null", O_WRONLY);
    }
  else
    {
      VOID close (STDIN);
      VOID open ("/dev/null", O_RDONLY);
      VOID close (pfd[0]);
      VOID close (STDOUT);
      VOID dup (pfd[1]);
      VOID close (pfd[1]);
    }
  if (ttyf >= 0)
    VOID close (ttyf);
  VOID execvp (arg[0], arg);
  VOID warn (arg[0], syserr ());
  _exit (1);
  return 0; /* to avoid compiler warning */
}

/*
 * pipeopen()
 *
 * Open an archive via a pipeline. Returns a file
 * descriptor, or -1 if unsuccessful.
 */
STATIC int
pipeopen (mode)
     reg int mode;
{
  auto int pfd[2];

  if (pipe (pfd) < 0)
    return (-1);
  if ((pipepid = xfork ("pipeopen()", DIE)) == 0)
    pipechld (mode, pfd);
  if (mode)
    {
      VOID close (pfd[0]);
      return (pfd[1]);
    }
  else
    {
      VOID close (pfd[1]);
      return (pfd[0]);
    }
}

/*
 * pipewait()
 *
 * Await a pipeline.
 */
STATIC void
pipewait ()
{
  reg int status;

  if (pipepid == 0)
    return;
  status = xwait (pipepid, "pipewait()", TRUE);
  pipepid = 0;
  if (status)
    fatal (arspec, "Pipeline error");
}

/*
 * prsize()
 *
 * Print a file offset.
 */
STATIC void
prsize (stream, size)
     FILE *stream;
     reg ulonglong size;
{
  reg ulonglong n;

  if ((n = (size / (1024L * 1024L))))
    {
      VOID fprintf (stream, "%lum+", (unsigned long)n);
      size -= n * 1024 * 1024;
    }
  if ((n = (size / 1024)))
    {
      VOID fprintf (stream, "%luk+", (unsigned long)n);
      size -= n * 1024;
    }
  VOID fprintf (stream, "%lu", (unsigned long) size);
}

#ifndef	MKDIR

/*
 * rmdir()
 *
 * Remove a directory via "/bin/rmdir". Sets errno to a
 * questionably sane value upon failure.
 */
STATIC int
rmdir (name)
     reg char *name;
{
  reg int pid;

  if ((pid = xfork ("rmdir()", DIE)) == 0)
    {
      VOID close (fileno (stdin));
      VOID close (fileno (stdout));
      VOID close (fileno (stderr));
      VOID open ("/dev/null", O_RDWR);
      VOID dup (fileno (stdin));
      VOID dup (fileno (stdin));
      VOID execl ("/bin/rmdir", "rmdir", name, (char *) NULL);
      exit (1);
    }
  if (xwait (pid, "rmdir()", TRUE) == 0)
    return (0);
  errno = EACCES;
  return (-1);
}

#endif /* MKDIR */

/*
 * fswrite()
 *
 * Write a filesystem block. Seeks past sparse blocks. Returns
 * 0 if the block was written, the given length for a sparse
 * block or -1 if unsuccessful.
 */
STATIC int
fswrite (fd, buf, len)
     int fd;
     char *buf;
     uint len;
{
  reg char *bidx;
  reg char *bend;

  /*
   * if -j (no sparse blocks) or compressrun (piping to uncompress utility)
   * then do not bother looking for empty buffers, just write the data.
   */
  if (jflag || uncompressrun)
    return (writeall (fd, buf, len) == (int)len ? 0 : -1);
  bend = (bidx = buf) + len;
  while (bidx < bend)
    if (*bidx++)
      return (writeall (fd, buf, len) == (int)len ? 0 : -1);
  return (lseek (fd, (off_t) len, 1) < 0 ? -1 : (int)len);
}

/*
 * syserr()
 *
 * Return pointer to appropriate system error message.
 */
STATIC char *
syserr ()
{
  static char msg[40];
#if 0
  /* older version */
  if (errno > 0 && errno < sys_nerr)
    return ((char *) sys_errlist[errno]);
  VOID sprintf (msg, "Unknown error (errno %d)", errno);
  return (msg);
#else
  /* newer version, should be posix compliant and eliminate
     some compiler warnings 
  */
  char *pTmp = NULL;
  if (! (pTmp = strerror(errno)))
  {
    VOID sprintf (msg, "Unknown error (errno %d)", errno);
    pTmp = msg;
  }
  return (pTmp);
#endif
}

/*
 * toc()
 *
 * Print archive table of contents.
 */
STATIC VOIDFN
toc (av)
     reg char **av;
{
  auto Stat sb;
  auto char name[PATHSIZE];

  if (*av)
    fatal (*av, "Extraneous argument");
  name[0] = '\0';
  while (inhead (name, &sb) == 0)
    {
	if (ISCONTROL(&sb))
	{
	    /* list it */
	    if (namecmp (name,&sb) == 0)
		tocentry (name, &sb);
	    
	    /* process control file */
	    if(inentry (name, &sb) < 0)
		if (inskip (sb.sb_size) < 0)
		    VOID warn (name, "Skipped file data is corrupt");

	    continue;
	}

	if (namecmp (name,&sb) == 0)
	    tocentry (name, &sb);
	if (inskip (sb.sb_size) < 0)
	    VOID warn (name, "File data is corrupt");
    }
}

/*
 * tocentry()
 *
 * Print a single table-of-contents entry.
 */
STATIC void
tocentry (name, asb)
     char *name;
     reg Stat *asb;
{
  reg Time *atm;
  reg Link *from;
  reg Passwd *pwp;
  reg Group *grp;
  static char *month[] =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  int res;
  Link *linkp;
  Link *linknext;
 
  if(printbytepos) printf("%.0f ",(double)bytepos);

  if (vflag)
    {
      tocmode (asb->sb_mode);
      VOID printf (" %2d", (int)asb->sb_nlink);
      atm = localtime (&asb->sb_mtime);
      if ((pwp = getpwuid (asb->sb_uid)))
	VOID printf (" %-8s", pwp->pw_name);
      else
	VOID printf (" %-8u", (unsigned int)(asb->sb_uid));
      if ((grp = getgrgid (asb->sb_gid)))
	VOID printf (" %-8s", grp->gr_name);
      else
	VOID printf (" %-8u", (unsigned int)(asb->sb_gid));
      switch (asb->sb_mode & S_IFMT)
	{
	case S_IFBLK:
	case S_IFCHR:
	    /* rdev==0 means that rdev is encoded in dev and ino,
	       see out().
	       */
	  if(asb->sb_rdev==0) asb->sb_rdev=(asb->sb_dev<<16) + asb->sb_ino;
	  VOID printf (" %3d, %3d",
		       (int)major (asb->sb_rdev), (int)minor (asb->sb_rdev));
	  break;
	case S_IFREG:
	  VOID printf (" %8lld", (long long int)(asb->sb_size));
	    /* VOID printf (" %8ld", (long unsigned int)(asb->sb_size)); */
	  break;
	default:
	  VOID printf ("         ");
	}
      VOID printf (" %3s %2d %02d:%02d:%02d %4d ",
		   month[atm->tm_mon], atm->tm_mday, atm->tm_hour,
		   atm->tm_min, atm->tm_sec, atm->tm_year + 1900);
    }

  {
    char *namedot = strrchr (name, '.');

    if (Zflag && (asb->sb_mode & S_IFMT) == S_IFREG
	&& (asb->sb_rdev & RDEV_NOTCOMPR) == 0
	&& (asb->sb_size > 0) 
	&& namedot && namedot[1] == 'z' && !namedot[2])
      *namedot = '\0';
    else
      namedot = 0;

    if (ISCONTROL(asb))
	res = printf("//--%s",name);
    else
     {
	if (flag0)
		res = printf ("%s%c", name, 0);
	else
		res = printf ("%s", name);
     }
    /* to find out about broken pipe as early as possible */ 
    if(res > 0) res = fflush(stdout);
    /* check for broken pipe on stdout, this ends the listing */
    if(res<0) {
	if(errno == EPIPE)
	    fatal("<stdout>", syserr());
    }


    if (vflag && namedot)
      VOID printf (" -- compressed");
   }

  if (!flag0 && (vflag || lflag))
    {
      from=NULL;
      if (asb->sb_nlink > 1)
	{

	  if(!useoutmodetoc)
	    {
	      if ((from = linkfrom (asb,1)))
		VOID printf (" -> %s", from->l_path->p_name);
	      else
		VOID linkto (name, asb);
	    }
	  else
	    {
	      /* need special logic to resolve hard link info
		 when in -o mode: because
		 of side effects of linkfrom and linkto, which were
		 already called for the file entry, we cannot use
		 them here again */
	      if((asb->sb_mode & S_IFMT) != S_IFDIR) 
		{
		  for (linkp = *(linkhash (asb->ino_orig)); linkp; linkp = linknext)
		    {
		      if (linkp->l_ino == asb->ino_orig
			  && linkp->l_dev == asb->sb_dev
			  && strcmp(linkp->l_path->p_name,name)
			  )
			{
			  VOID printf (" -> %s", linkp->l_path->p_name);
			  break;
			}
		      
		      linknext = linkp->l_forw;
		    }		  
		} 
	    }

	}
	  
#ifdef	S_IFLNK
      if (((asb->sb_mode & S_IFMT) == S_IFLNK)&&(from==NULL))
	VOID printf (" S-> %s", asb->sb_link);
#endif /* S_IFLNK */
    }

  if (!flag0)
    putchar ('\n');
}

/*
 * tocmode()
 *
 * Fancy file mode display.
 */
STATIC void
tocmode (mode)
     reg mode_t mode;
{
  switch (mode & S_IFMT)
    {
    case S_IFREG:
      putchar ('-');
      break;
    case S_IFDIR:
      putchar ('d');
      break;
#ifdef	S_IFLNK
    case S_IFLNK:
      putchar ('l');
      break;
#endif /* S_IFLNK */
#ifdef	S_IFSOCK
    case S_IFSOCK:
      putchar ('s');
      break;
#endif /* S_IFSOCK */
    case S_IFBLK:
      putchar ('b');
      break;
    case S_IFCHR:
      putchar ('c');
      break;
#ifdef	S_IFIFO
    case S_IFIFO:
      putchar ('p');
      break;
#endif /* S_IFIFO */
    default:
      VOID printf ("[%o]", (uint)(mode >> S_IFSHF));
    }
  putchar (mode & 0400 ? 'r' : '-');
  putchar (mode & 0200 ? 'w' : '-');
  putchar (mode & 0100
	   ? mode & 04000 ? 's' : 'x'
	   : mode & 04000 ? 'S' : '-');
  putchar (mode & 0040 ? 'r' : '-');
  putchar (mode & 0020 ? 'w' : '-');
  putchar (mode & 0010
	   ? mode & 02000 ? 's' : 'x'
	   : mode & 02000 ? 'S' : '-');
  putchar (mode & 0004 ? 'r' : '-');
  putchar (mode & 0002 ? 'w' : '-');
  putchar (mode & 0001
	   ? mode & 01000 ? 't' : 'x'
	   : mode & 01000 ? 'T' : '-');
}

/*
 * usage()
 *
 * Print a helpful message and exit.
 */
STATIC void
usage ()
{
  VOID fprintf (stderr, "\n\
Usage:	[filename generator] | %s -o [options] archive  : write archive\n\
        %s -i [options] archive  : install archive\n\
        %s -t [options] archive  : list table-of-contents of archive\n\
        %s -r [options] archive  : verify archive against filesystem\n\
Frequently used options:\n\
 General: -v : verbose\n\
          -Z : with -o: gzip files when writing them to the archive,\n\
               with -i/t/r: handle archive written with -Z option\n\
          -5 : abort instead of creating archive incompatible with cpio\n\
 Tape:    -s [volsize]   : size of volume, can have suffix k or m or g\n\
          -b [blocksize] : block size (default is 5120)\n\
          -c [count]     : buffer count blocks between doing I/O\n\
 Install: -n : protect newer files  -k : skip corrupt data at beginning\n\
 Select:  -y [pattern] : only process files matching pattern\n\
          -Y [pattern] : do not process files matching pattern\n",
		myname, myname, myname, myname);
  VOID fprintf (stderr,"Version %s dated %s\n", VERSION, DATE);
  exit (1);
}

/*
 * warn()
 *
 * Print a warning message. Always returns -1.
 */
STATIC int
warn (what, why)
     char *what;
     char *why;
{
  time_t dietime;
  warnings++;
  dietime = time ((time_t *) NULL);

  VOID fprintf (stderr,
		"%s: \"%s\": %s\n",
		myname, what, why);
  /* ctime() prodives the \n for the fprintf. */
  if (logfile != (FILE *) 0)
    VOID fprintf (logfile, "%s: \"%s\": %s (disk %u) at %s",
		  myname, what, why, arvolume, ctime (&dietime));
  return (-1);
}

STATIC int warn_nocount (what, why)
     char *what;
     char *why;
{
 warnings--;
 return warn(what,why);
}

/*
 * warnarch()
 *
 * Print an archive-related warning message, including
 * an adjusted file offset. Always returns -1.
 */
STATIC int
warnarch (msg, adjust)
     char *msg;
     off_t adjust;
{
  warnings++;
  VOID fprintf (stderr, "%s: \"%s\" [offset ", myname, arspec);
  prsize (stderr, total - adjust);
  VOID fprintf (stderr, "]: %s\n", msg);
  return (-1);
}

/*
 * xfork()
 *
 * Create a child.
 */
STATIC int
xfork (what, die)
     reg char *what;
     int die;
{
  reg int pid;
  reg Child *cp;
  reg int idx;
  static uint delay[] =
  {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};

  /* flush error logfile before fork */
  if(logfile != (FILE *)0) fflush(logfile);
  /* also flush stdout and stderr */
  fflush(stdout);
  fflush(stderr);


#ifdef SIGCHLD
   VOID signal (SIGCHLD, SIG_DFL);	/* SysV mostly... */
#else
   VOID signal (SIGCLD, SIG_DFL);  /* some SysV's... */
#endif
  for (idx = 0; (pid = fork ()) < 0; ++idx)
    {
      if (idx == sizeof (delay)) 
	{
	  if (die)
	    fatal (arspec, syserr ());
	  else
	    return (-1);
	}

      VOID warn_nocount (what, "Trouble forking...");
      if (Fflag && !die)	/* give up and go on... */
	return (-1);
      sleep (delay[idx]);
    }
  if (idx)
    VOID warn_nocount (what, "...successful fork");
  cp = (Child *) memget (sizeof (*cp));
  cp->c_pid = pid;
  cp->c_flags = 0;
  cp->c_status = 0;
  cp->c_forw = children;
  children = cp;
  return (pid);
}

/*
 * xpause()
 *
 * Await a child.
 */
STATIC void
xpause ()
{
  reg Child *cp;
  reg int pid;
  auto int status;

  do
    {
      while ((pid = wait (&status)) < 0)
	;
      for (cp = children; cp && cp->c_pid != pid; cp = cp->c_forw)
	;
    }
  while (cp == NULL);
  cp->c_flags |= CF_EXIT;
  cp->c_status = status;
}

/*
 * xwait()
 *
 * Find the status of a child.
 */
STATIC int
xwait (pid, what, compstat2)
     reg int pid;
     char *what;
     int compstat2;
{
  reg int status;
  reg Child *cp;
  reg Child **acp;
  auto char why[100];

  for (acp = &children; (cp = *acp); acp = &cp->c_forw)
    if (cp->c_pid == pid)
      break;
  if (cp == NULL)
    fatal (what, "Lost child");
  while ((cp->c_flags & CF_EXIT) == 0)
    xpause ();
  status = cp->c_status;
  *acp = cp->c_forw;
  free ((char *) cp);
  if (status == 0)
    return (0);
  if (status & 0377)
    VOID sprintf (why, "Killed by signal %d%s",
		  status & 0177, status & 0200 ? " -- core dumped" : "");
  else
    VOID sprintf (why, "Exit %d", (status >> 8) & 0377);

  if ((!compstat2 && (((status >> 8) & 0377) != 2)) || compstat2)
    return (warn (what, why));
  else
    return ((status >> 8) & 0377);
}


/* right now we verify the whole disk */
void
verify (error)
     int error;
{
  char *verbuf;
  char *buf;
  reg time_t began;
  int got, len;
  uint readamt;
  auto char msg[200];
  auto char answer[20];

  if (*arname == '!')
    {
      VOID warn ("Can't verify a piped command", "");
      return;
    }
  if (!error)
    {
      if ((verbuf = malloc (arbsize)) == NULL)
	fatal (arspec, "Cannot allocate Verify I/O buffer");

      /*
       * close as O_WRONLY and reopen as O_RDONLY to verify (on a
       * disk this is a good thing)
       */
      /* fprintf(stderr,"closing..\n"); */
 
      nextclos ();
      verifycnt++;

      if (nextopen (O_RDONLY) < 0)
	{
	  VOID warn ("re-open for verify failed", "");
	  error = 1;
	}
      else
	{
#ifdef linux
          /* flush the floppy cache. (added by KH) */
          if(Fflag)
	    {   /*   fprintf(stderr,"flushing..\n"); */
              if (ioctl(arfd,FDFLUSH,NULL) < 0) 
                  warn(arname,"can't flush device cache.");
          
            }
#endif
	  fprintf (stderr, "Verifying disk %u...\n", arvolume );
	  for (buf = buffer; (len = bufidx - buf);)
	    {
	      readamt = min ((uint)len, arbsize);
	      if ((uint)(got = read (arfd, verbuf, readamt)) == readamt)
		{
#ifdef HAVEMEMCMP
		  if (memcmp (verbuf, buf, (size_t)got) != 0)
#else
		  if (bcmp (verbuf, buf, got) != 0)
#endif
		    {
		      VOID warn ("Verify failed", "");
		      error = 1;
		      break;
		    }
		  else
		    buf += got;
		}
	      else
		{
		  VOID warn ("Read returned short", "");
		  fprintf (stderr, "Read %d wanted %d bytes\n", got,
			   readamt);
		  error = 1;
		  break;
		}
	    }
	}
      free (verbuf);
    }
  if (error)
    {
      int answernum = 0;
      nextclos ();

      for (;;)
	{
	    began = time ((time_t *) NULL);
	    VOID sprintf (msg, "\
%s: %s of disk %u has FAILED!\07\n\
\tEnter 1 to RETRY this disk\n\
\tEnter 2 to REFORMAT this disk before a RETRY\n\07\n%s",
			  myname,
			  ((error && (verifycnt == 0)) ? "Writing" :
			   "Verify"),
			  arvolume, hidequit ? "" :
			  "\tEnter \"quit\" to ABORT the backup\n\07");
	    nextask (msg, answer, sizeof (answer));
	    timewait += time ((time_t *) NULL) - began;
	    answernum = atoi (answer);

	  if (answernum == 1)	/* note: recursive here... */
	    {
	      /* if we can't open, try again */
	      if (nextopen (O_WRONLY) < 0)
		continue;
	    }
	  else if ((answernum == 2) )
	  {
	      if (system (formatcmd) != 0)
	      {
		  fprintf (stderr, "Format failed!\n");
		  answernum = 0;/* error, don't autowrite */
	      }
	      else 
	      {
		  fprintf (stderr, "Format successful!\n");
		  /* if we can't open, try again */
		  if (nextopen (O_WRONLY) < 0)
		      continue;      
		  return;
	      }	
	  }
	  else if (strcmp (answer, "quit") == 0)
	      fatal (arspec, "Quiting during a verify");
	}
    }
}


int
writedisk (realwrite)
     int realwrite;
{
  reg char *buf;
  reg int got;
  reg uint len;
  int wrstat;
static int firsttime = 1;  
  /*
   * If we're double buffering wait for any pending writes to
   * complete before starting next writing
   */

  if (fflag)
      outwait ();
  
  /*
   * if we are a floppy open the disk at the last moment
   * call verify w/ an error if the disk can't be opened
   */
  if (Fflag)
  {
    /* There's something funny in the program in that writedisk() gets
       called when there is nothing in the buffer to write; it
       apparently gets called twice for each buffer when using the -s
       double buffer option.  I can't figure out the complex and
       undocumented definitions of bufidx and bufend well enough to
       stop this double calling, but the only harm done by the second
       call is if we call for another disk to be loaded.  So we make
       sure we only call for a disk if there's actually data to write
       on it (i.e. bufidx > buffer).  - Bryan Henderson 98.08.12
       */
      if( ! firsttime && bufidx > buffer )
          next( O_WRONLY, "Next disk needed");
       else 
          while( nextopen (O_WRONLY) < 0)
	  {
	      verifycnt = 0;
	      verify (1);
	  }
      firsttime = 0;	  
  }

  /*
   * If we're double buffering spawn a child to do the
   * actual writing, and return immediately
   */

  if( fflag ) 
  {
      outpid = xfork ("outflush()", DIE);
      if( outpid != 0 )
      {
	  /* what does this do? It seems to be needed to let -s -f work */
	  arleft -= bufidx - buffer;

          nextclos(); /* added by KH to make verify work */

	  return(0);
      }
      else
	  VOID nice (-10);
  }

  do 
  {
      wrstat = 0 ;

      for (buf = buffer; (len = bufidx - buf);)
      {
	  if ((got = write (arfd, buf,
			    *arname == '!' ? len : min (len, arbsize))) > 0)
	     {
		 buf += got;
		 if (realwrite)
		     arleft -= got;
	     }
	  else if (fflag)
	     {
		 VOID warn (arspec, got < 0
			    ? syserr ()
			    : "Apparently full -- archive will be incomplete");
		 _exit (1);
	     }
	  else if (got < 0)
	     {
		 if(errno==EPIPE)
		     fatal(arspec, syserr());
 
                 if(errno==ENOSPC) {    /* For DAT/DDS -- RAM, 1998/05/19 */
                     next (O_WRONLY, "No space left on device");
                     continue;          /* Redo failed write on new volume */
                 }
  
		 if(!Jflag)
		   VOID fatal (arspec, syserr ());		 
		 else
		   { 
		       VOID warn (arspec, syserr ());
		       anycorrupt=1;
		   }
                 wrstat = 1;
                 break;
	     }
	  else
	     {
		 if (Fflag && verifyflag)
                 {
		     /* can't recover from this one, break out of loop
                        and let verify detect the mess */
		     VOID warn (arspec,
				"Apparently full already");
		     wrstat = 1;
		     break;
		 }

		 next (O_WRONLY, "Apparently full");
                 /* added continue, this creates correct behavior
                    (no unwritten data) when changing to next volume on
                    write()=0 -- KH */
		 continue;
	     }
         }
      if (Fflag && verifyflag)
	   {
	       verifycnt = 0;
	       verify ( wrstat );
	   }

      /* Added KH: bug fix for double prompting bug if no -K and -f given */
      /* It took me far too long to find this.  IMO it was a big mistake to
         use arleft, which usually denotes the space left in the buffer, to
         denote the space left on the device in this routine.
         Talking about spaghetti variables.  I'd rather not think about what
         will happen if the write/verify error handling stuff gets used.
      */
      if (Fflag && !verifyflag) arleft = aruntil; 

	       
  } while( wrstat && Fflag && verifyflag );
  
  if( fflag )
    _exit( wrstat );
  else 	      
    return ( wrstat );

  return 0; /* to avoid compiler warning */
}

void
goodbye (status)
     int status;
{
  /* log that we died */
  if (status && (logfile != (FILE *) 0))
    {
      time_t dietime;
      dietime = time ((time_t *) NULL);

      VOID fprintf (logfile, "%s: the backup has failed (status %d), died at %s",
		    myname, status, ctime (&dietime));

    }
  exit (status);
}

#ifdef MYTEMPNAM
/* author:	Monty Walls
 * written:	4/17/89
 * Copyright:	Copyright (c) 1989 by Monty Walls.
 *		Not derived from licensed software.
 *
 *		Permission to copy and/or distribute granted under the
 *		following conditions:
 *
 *		1). This notice must remain intact.
 *		2). The author is not responsible for the consequences of use
 *			this software, no matter how awful, even if they
 *			arise from defects in it.
 *		3). Altered version must not be represented as being the
 *			original software.
 */

#define MAXPREFIX	5
#define TMPNAME		"tmp"
#ifndef P_tmpdir
#define P_tmpdir	"/tmp"
#define L_tmpnam	14
#endif

extern char *mktemp ();
extern char *strcat ();
extern char *strcpy ();
extern char *getenv ();

char *
tempnam (dir, name)
     char *dir;
     char *name;
{
  char *buf, *tmpdir;

  /*
   * This is kind of like the chicken & the egg.
   * Do we use the users preference or the programmers?
   */
#ifdef USE_ENV_VAR
  if ((tmpdir = getenv ("TMPDIR")) == (char *) NULL)
    {
      if ((tmpdir = dir) == (char *) NULL)
	tmpdir = P_tmpdir;
    }
#else
  if ((tmpdir = dir) == (char *) NULL)
    {
      if ((tmpdir = getenv ("TMPDIR")) == (char *) NULL)
	tmpdir = P_tmpdir;
    }
#endif
  /* now lets check and see if we can work there */
  if (access (tmpdir, R_OK + W_OK + X_OK) < 0)
    return ((char *) NULL);

  if (name == (char *) NULL)
    name = TMPNAME;
  else if (strlen (name) > MAXPREFIX)
    name[5] = '\0';		/* this is according to SYS5 */

  /* the magic value 2 is for '\0' & '/' */
  if ((buf = (char *) malloc (L_tmpnam + strlen (tmpdir) + 2)) == (char *) NULL)
    return ((char *) NULL);

  strcpy (buf, tmpdir);
  strcat (buf, "/");
  strcat (buf, name);
  strcat (buf, ".XXXXXX");

  /* pass our completed pattern to mktemp */
  return (mktemp (buf));
}

#endif /* MYTEMPNAM */
