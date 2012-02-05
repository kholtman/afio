/* afio.h  defines for afio. */

#ifdef	SYSTIME
#include <sys/time.h>
#else /* SYSTIME */
#include <time.h>
#endif /* SYSTIME */

#ifdef	CTC3B2
#include <sys/vtoc.h>
#include <sys/ct.h>
#endif /* CTC3B2 */

#ifdef MYTEMPNAM
#include <sys/file.h>
#endif

#ifdef USESHMEM
#include <sys/ipc.h>
#include <sys/shm.h>

#define	NUMSHKEYS	20
#define	SHMEMSIZE	262144	/* 2^18 (dev3b1) */
#endif

/* done writing to the archive */
#define	FALSE	0
#define	TRUE	1
#define	NOTDONE	0
#define	DONE	1
#define	NODIE	0
#define	DIE	1

/* Note for porters: see the PORTING file if your compiler
   does not support unsigned long long.
*/
typedef  unsigned long long ulonglong;

/* KH */
#define MEMFD 10000
#define ZIPFD 10001
extern int zipfdfd;

/* flags for the st_rdev field of regular files */
 /* file is not compressed, ignore any .z extension */
#define RDEV_NOTCOMPR 1
 /* file is a control file */
#define RDEV_ISCONTROL 2

/*
   An archive entry is a control file if it is a regular file and
   if the ISCONTROL flag is set.  The filename is not important.
*/
#define ISCONTROL(sb) ((((sb)->sb_mode & S_IFMT) == S_IFREG)&&((sb)->sb_rdev & RDEV_ISCONTROL))

/* pseudo filename for control files */
#define CONTROLNAME "CONTROL_FILE"

/* label value if no label given */
#define NOLABEL "no_label"

/*
 * Address link information base.
 */
#define	linkhash(ino)	\
	(linkbase + ((ino) & 0xffff) % nel(linkbase))

/*
 * Mininum value.
 */
#define	min(one, two)	\
	(one < two ? one : two)

/*
 * Number of array elements.
 */
#define	nel(a)		\
	(sizeof(a) / sizeof(*(a)))

/*
 * Remove a file or directory.
 */
#define	afremove(name, asb) \
	(((asb)->sb_mode & S_IFMT) == S_IFDIR ? rmdir(name) : unlink(name))

/*
 * Swap bytes.
 */
#define	swab(n)		\
	((((unsigned short)(n) >> 8) & 0xff) | (((unsigned short)(n) << 8) & 0xff00))

/*
 * Cast and reduce to unsigned short.
 */
#define	ush(n)		\
	(((unsigned short) (n)) & 0177777)

#define ulo(n) 		\
	(((unsigned long) (n)) & 0xFFFFFFFFL)

#define ull(n)		\
	(((unsigned long long) (n)) & 0xFFFFFFFFFFFFFFFFLL)
/*                             1234567890123456 */

/*
 * Definitions.
 */
#define	reg	register	/* Convenience */
#define	uint	unsigned int	/* Not always in types.h */
#define	ushort	unsigned short	/* Not always in types.h */

#define	BLOCK	5120		/* Default archive block size */
#define	FSBUF	(8*1024)	/* Filesystem buffer size */
#define	H_COUNT	10		/* Number of items in ASCII header */

/* binary format */
#define	M_BINARY 070707		/* Binary magic number */
#define	M_STRLEN 6		/* ASCII magic number length */

/* (old) ASCII format */
#define	H_STRLEN 70		/* old ASCII header string length */
#define	M_ASCII "070707"	/* old ASCII magic number */
#define H_PRINT "%06o%06o%06o%06o%06o%06o%06o%011lo%06o%011lo"
/* H_SCAN is obsolete, replaced by PH_SCAN to be more portable. */
#define H_SCAN  "%6ho%6ho%6ho%6ho%6ho%6ho%6ho%11lo%6o%11lo"
#define PH_SCAN  "%6lo%6lo%6lo%6lo%6lo%6lo%6lo%11lo%6o%11lo"

/*
Below is a handy piece of ASCII art which can be used to decode old
ASCII format headers.

|23456|23456|23456|23456|23456|23456|23456|23456|23456789ab|23456|23456789ab|-
|  hdr|  dev|  ino| mode|  uid|  gid|nlink| rdev|     mtime|nmlen|      size|n
*/

/* extended ASCII format */
#define	H_STRLEN2 75		/* extended ASCII header string length */
#define	M_ASCII2 "070717"	/* extended ASCII magic number */
#define H_PRINT2  "%06o%011lo%06o%06o%06o%06o%06o%011lo%06o%011lo"
#define PH_SCAN2  "%6lo%11lo%6lo%6lo%6lo%6lo%6lo%11lo%6o%11lo"


/*
Below is a handy piece of ASCII art which can be used to decode large
ASCII format headers.

|23456|2345678|234567890123456m|23456|2345678|2345678|2345678|2345678|234567890123456n|234|234|234s|234567890123456:|-
|  hdr|    dev|            inoM|  mod|    uid|    gid|  nlink|   rdev|          mtimeN|nml|flg|xszS|           size:|n
*/

/* large ASCII format */
#define	H_STRLEN3 110		/* large ASCII string length */
#define	M_ASCII3 "070727"	/* large ASCII magic number */
#define H_COUNT3 12
#define	H_PRINT3  "%08lX%016llXm%06lo%08lX%08lX%08lX%08lX%016llXn%04X%04X%04Xs%016llX:"
#define	PH_SCAN3  "%8lX%16llXm%6lo%8lX%8lX%8lX%8llX%16llXn%4X%4X%4Xs%16llX:"



typedef struct {
long unsigned int PSt_dev;
long unsigned int PSt_ino;
long unsigned int PSt_mode;
long unsigned int PSt_uid;
long unsigned int PSt_gid;
long unsigned int PSt_nlink;
long unsigned int PSt_rdev;
long unsigned int PSt_mtime;
long unsigned int PSt_size;
} PStat;


typedef struct {
long unsigned int PSt_dev;
unsigned long long PSt_ino;
long unsigned int PSt_mode;
long unsigned int PSt_uid;
long unsigned int PSt_gid;
long unsigned int PSt_nlink;
unsigned long long PSt_rdev;
unsigned long long PSt_mtime;
unsigned long long PSt_size;
} PHStat;



#define	NULLDEV	-1		/* Null device code */
#define	NULLINO	0		/* Null inode number */
#define	PATHELEM 256		/* Pathname element count limit */
#define	PATHSIZE 1024		/* Pathname length limit */
#define	S_IFSHF	12		/* File type shift (shb in stat.h) */
#define	S_IPERM	07777		/* File permission bits (shb in stat.h) */
#define	S_IPEXE	07000		/* Special execution bits (shb in stat.h) */
#define	S_IPOPN	0777		/* Open access bits (shb in stat.h) */
#define	STDIN	0		/* Standard input file descriptor */
#define	STDOUT	1		/* Standard output file descriptor */
#define	TTY	"/dev/tty"	/* For volume-change queries */

/* the three flags below are used in the match.c code, they allow us
   to store 3 types of patterns in a single `all patterns' linked
   list.  (the use of a single list is for historical reasons, it is
   not the most ovbious design for the current needs)
 */
#define PATTYPE_MATCH 0 
#define PATTYPE_NOMATCH 1
#define PATTYPE_EXT 2

#ifndef PRG_COMPRESS
#define PRG_COMPRESS "compress"
#endif

/*
 * Some versions of the portable "C" compiler (PCC) can't handle
 * pointers to functions returning void.
 */
#ifdef	VOIDFIX
#define	VOIDFN	void		/* Expect "void (*fnptr)()" to work */
#else /* VOIDFIX */
#define	VOIDFN	int		/* Avoid PCC "void (*fnptr)()" bug */
#endif /* VOIDFIX */

/*
 * Trailer pathnames. All must be of the same length.
 */
#define	TRAILER	"TRAILER!!!"	/* Archive trailer (cpio compatible) */
#define	TRAILZ	11		/* Trailer pathname length (including null) */

/*
 * Open modes; there is no <fcntl.h> with v7 UNIX.
 */
#ifdef HAVEFCNTL
#include <fcntl.h>
#else
#define	O_RDONLY 0		/* Read-only */
#define	O_WRONLY 1		/* Write-only */
#define	O_RDWR	2		/* Read/write */
#endif
/*
 * V7 and BSD UNIX use old-fashioned names for a couple of
 * string functions.
 */
#ifdef	INDEX
#define	strchr	index		/* Forward character search */
#define	strrchr	rindex		/* Reverse character search */
#endif /* INDEX */

/*
 * Some compilers can't handle void casts.
 */
#ifdef	NOVOID
#define	VOID			/* Omit void casts */
#else /* NOVOID */
#define	VOID	(void)		/* Quiet lint about ignored return values */
#endif /* NOVOID */

/*
 * Adb is more palatable when static functions and variables are
 * declared as globals. Lint gives more useful information when
 * statics are truly static.
 */
#ifdef	lint
#define	STATIC	static		/* Declare static variables for lint */
#else /* lint */
#define	STATIC			/* Make static variables global for adb */
#endif /* lint */

/*
 * Simple types.
 */
typedef struct group Group;	/* Structure for getgrgid(3) */
typedef struct passwd Passwd;	/* Structure for getpwuid(3) */
typedef struct tm Time;		/* Structure for localtime(3) */

#ifdef	S_IFLNK
/*
 * File status with symbolic links. Kludged to hold symbolic
 * link pathname within structure.
 */
typedef struct
{
  struct stat sb_stat;
  char sb_link[PATHSIZE];
  ino_t ino_orig; /* used in -o mode to support -ovv */
} Stat;

#define	STAT(name, asb)		stat(name, &(asb)->sb_stat)
#define	FSTAT(fd, asb)		fstat(fd, &(asb)->sb_stat)
#define	LSTAT(name, asb)	lstat(name, &(asb)->sb_stat)
#define	sb_dev		sb_stat.st_dev
#define	sb_ino		sb_stat.st_ino
#define	sb_mode		sb_stat.st_mode
#define	sb_nlink	sb_stat.st_nlink
#define	sb_uid		sb_stat.st_uid
#define	sb_gid		sb_stat.st_gid
#define	sb_rdev		sb_stat.st_rdev
#define	sb_size		sb_stat.st_size
#define	sb_atime	sb_stat.st_atime
#define	sb_mtime	sb_stat.st_mtime
#define	sb_ctime	sb_stat.st_ctime
#define	sb_blksize	sb_stat.st_blksize
#define	sb_blocks	sb_stat.st_blocks
#else /* !S_IFLNK */
/*
 * File status without symbolic links.
 */
typedef struct stat Stat;
#define	STAT(name, asb)		stat(name, asb)
#define	FSTAT(fd, asb)		fstat(fd, asb)
#define	LSTAT(name, asb)	stat(name, asb)
#define	sb_dev		st_dev
#define	sb_ino		st_ino
#define	sb_mode		st_mode
#define	sb_nlink	st_nlink
#define	sb_uid		st_uid
#define	sb_gid		st_gid
#define	sb_rdev		st_rdev
#define	sb_size		st_size
#define	sb_atime	st_atime
#define	sb_mtime	st_mtime
#define	sb_ctime	st_ctime
#endif /* !S_IFLNK */

/*
 * Binary archive header (obsolete).
 */
typedef struct
{
  short b_dev;			/* Device code */
  ushort b_ino;			/* Inode number */
  ushort b_mode;		/* Type and permissions */
  ushort b_uid;			/* Owner */
  ushort b_gid;			/* Group */
  short b_nlink;		/* Number of links */
  short b_rdev;			/* Real device */
  ushort b_mtime[2];		/* Modification time (hi/lo) */
  ushort b_name;		/* Length of pathname (with null) */
  ushort b_size[2];		/* Length of data */
} Binary;

/*
 * Child process structure.
 */
typedef struct child
{
  struct child *c_forw;		/* Forward link */
  int c_pid;			/* Process ID */
  int c_flags;			/* Flags (CF_) */
  int c_status;			/* Exit status */
} Child;

/*
 * Child process flags (c_flags).
 */
#define	CF_EXIT	(1<<0)		/* Exited */

/*
 * Hard link sources. One or more are chained from each link
 * structure.
 */
typedef struct name
{
  struct name *p_forw;		/* Forward chain (terminated) */
  struct name *p_back;		/* Backward chain (circular) */
  char *p_name;			/* Pathname to link from */
} Path;

/*
 * File linking information. One entry exists for each unique
 * file with with outstanding hard links.
 */
typedef struct link
{
  struct link *l_forw;		/* Forward chain (terminated) */
  struct link *l_back;		/* Backward chain (terminated) */
  dev_t l_dev;			/* Device */
  ino_t l_ino;			/* Inode */
  ino_t l_ino_ar;		/* Inode nr we will put in the archive */
  ushort l_nlink;		/* Unresolved link count */
  time_t l_mtime;		/* Modification time */
  ushort l_mode;               /* mode */
  off_t l_size;			/* Length */
  Path *l_path;			/* Pathname(s) to link from */
} Link;

/*
 * Directory information. One entry exists for each directory
 * in order to update it with the correct timestamps.
 */
typedef struct dir
{
  struct dir *d_forw;		/* Forward chain (terminated) */
  time_t d_mtime;		/* Modification time */
  char *d_name;			/* Pathname of directory */
} Dir;
 


/*
 * Internal functions.
 */
VOIDFN copyin (char **);
VOIDFN copyout (char **);
void compressfile (int *, char *, Stat *, int *);
int dirchg (char *, char *);
int dirmake (char *, Stat *);
int dirneed (char *);
void fatal (char *,char *);
void goodbye (int);
VOIDFN in (char **);
void inalloc (uint);
int inascii (char *, char *, Stat *);
int inascii2 (char *, char *, Stat *);
int inascii3 (char *, char *, Stat *);
int inavail (char **, uint*);
int inbinary (char *, char *, Stat *);
int indata (int, off_t, char*);
int inentry (char *, Stat *);
int infill (void);
int inhead (char *, Stat *);
int inread (char *, uint);
int inskip (off_t);
int inswab (char *, char *, Stat *);
int lineget (FILE *, char *, int);
void linkalso (Link *, char * );
Link *linkfrom (Stat *, int);
void linkleft (void);
Link *linkto (char *, Stat *);
#ifndef MEMCPY
char *memcpy (char *, char *, uint);
#endif
char *memget (uint);
char *memstr (char *);
#ifndef MKDIR
int mkdir (char *, ushort);
#endif
void nameadd (char *, int);
int namecmp (char *, Stat *);
int namecmp_ext (char *);
int nameopt (char *);
void next (int, char *);
void nextask (char *, char *, int);
void nextclos (void );
int nextopen (int);
int openin (char *, char *, Stat *, int *);
int openotty (char *, Stat *, Link *, int, int);
int openqtty (void);
int options (int, char **, char *);
VOIDFN out (char **);
void outalloc (uint);
uint outavail (char **);
int outdata (int, char*, off_t);

void outdatazip (int, char*, off_t); /* added KH */
void waitforgzip(void);     /* added KH */
void outdatamem (char *, off_t); /* added KH */
void memreset(void); /* added KH */
int memread(char *buf,int count); /* added KH */
void memfree(void); /* added KH */
int nameaddfile(char *, int, int); /* added KH */

void outeof (char *, uint);
void outflush (int);
void outhead (char *, Stat *);
void outhead2 (char *, Stat *);
void outhead3 (char *, Stat *);
void outpad (off_t);
void outwait (void);
void outwrite (char *,uint);
VOIDFN pass (char **);
void passdata (char *, int, char*, int);
int passitem (char *, Stat *,int, char**);
int pipechld (int, int *);
int pipeopen (int );
void pipewait (void);
void prsize (FILE *, ulonglong);
VOIDFN readcheck (char **);
#ifndef MKDIR
int rmdir (char *);
#endif
#if !defined (linux) && !defined(__FreeBSD__) && !defined(sun) && !defined(__CYGWIN32__)
VOIDFN (*signal ())();
#endif
     int fswrite (int, char*, uint);
     char *syserr (void);
     VOIDFN toc (char **);
     void tocentry (char *, Stat *);
     void tocmode (mode_t);
     void usage (void);
     void verify (int);
     int warn (char *, char *);
     int warn_nocount (char*, char *);
     int warnarch (char *, off_t);
     int writedisk (int);
     int xfork (char *, int);
     void xpause (void);
     int xwait (int, char *, int);
     void mail(char *who,int vol,char *archive);

int writeall(int, const char*, unsigned int);
int incheckentry(char *, Stat *);
int incheckdata (int fd, off_t size, char *name, Stat *asb, int comp);
Link * linkinode16 (dev_t dev, ino_t ino);
int opencontrolscript (char *name);

int openincheck(char *, Stat *, int *,int);
void mayberewind(void);

extern  void add_arg(char *arg);
extern  char *compress_arg_list[];

extern short lflag;
extern short hflag;

extern int gzipfactor;
extern off_t maxmem;
extern long compthreshold;
extern int ignoreslash;
extern short Zflag;
extern int arfd;
extern int extcasesens;

extern int forceZflag;
extern char *compressprog;
extern int compressargs;
extern int rewindfd;
extern char *ignorewarnings;

extern int readcompexts(char*);

ulonglong optsize (char *);
void update_aruntil(void);
extern ulonglong maxsizetocompress;
extern short noglob;
extern short flag0;
