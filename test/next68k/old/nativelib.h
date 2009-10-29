/* nativelib.h */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> /* TODO: Remove this line and change to use sp_*** types */
#include <ctype.h> // For references to _CurrentRuneLocale and _DefaultRuneLocale

#include <config.h>

#ifndef NATIVE_LIB_H
#define NATIVE_LIB_H

typedef void (*NativeLibFuncPtr)(...);

typedef struct {
	char *name;
	NativeLibFuncPtr addr;
} NativeLibEntry;

extern NativeLibEntry NativeLibs[];
extern const unsigned int kNativeLibCount;

typedef struct {
	FILE *file;
	char pad[0x58-sizeof(void*)];
} DarwinFILE;

extern DarwinFILE nativeSF[];
FILE *getFILE(unsigned int);

struct darwin_stat {
	int32_t	  st_dev;		/* inode's device */
	int32_t	  st_ino;		/* inode's number */
	int16_t	  st_mode;		/* inode protection mode */
	int16_t	  st_nlink;		/* number of hard links */
	int32_t	  st_uid;		/* user ID of the file's owner */
	int32_t	  st_gid;		/* group ID of the file's group */
	int32_t	  st_rdev;		/* device type */
//#ifndef _POSIX_SOURCE
//	struct	timespec st_atimespec;	/* time of last access */
//	struct	timespec st_mtimespec;	/* time of last data modification */
//	struct	timespec st_ctimespec;	/* time of last file status change */
//#else
	int32_t	  st_atime;		/* time of last access */
	int32_t	  st_atimensec;		/* nsec of last access */
	int32_t	  st_mtime;		/* time of last data modification */
	int32_t	  st_mtimensec;		/* nsec of last data modification */
	int32_t	  st_ctime;		/* time of last file status change */
	int32_t	  st_ctimensec;		/* nsec of last file status change */
//#endif
	int64_t	  st_size;		/* file size, in bytes */
	int64_t	  st_blocks;		/* blocks allocated for file */
	u_int32_t st_blksize;		/* optimal blocksize for I/O */
	u_int32_t st_flags;		/* user defined flags for file */
	u_int32_t st_gen;		/* file generation number */
	int32_t	  st_lspare;
	int64_t	  st_qspare[2];
};

#ifdef OS_LINUX   /* This may be considered dirty, but rune_t is 4 bytes on every platform */
    typedef unsigned int rune_t; 
#endif

#define Darwin_CACHED_RUNES (256 )

typedef struct {
        rune_t          min;            /* First rune of the range */
        rune_t          max;            /* Last rune (inclusive) of the range */
        rune_t          map;            /* What first maps to in maps */
        unsigned long   *types;         /* Array of types in range */
} Darwin_RuneEntry;

typedef struct {
        int             nranges;        /* Number of ranges stored */
        Darwin_RuneEntry      *ranges;        /* Pointer to the ranges */
} Darwin_RuneRange;


typedef struct {
        char            magic[8];       /* Magic saying what version we are */
        char            encoding[32];   /* ASCII name of this encoding */

        rune_t          (*sgetrune)
           (const char *, size_t, char const **);
        int             (*sputrune)
           (rune_t, char *, size_t, char **);
        rune_t          invalid_rune;

        unsigned long   runetype[Darwin_CACHED_RUNES];
        rune_t          maplower[Darwin_CACHED_RUNES];
        rune_t          mapupper[Darwin_CACHED_RUNES];

        /*
         * The following are to deal with Runes larger than _CACHED_RUNES - 1.
         * Their data is actually contiguous with this structure so as to make
         * it easier to read/write from/to disk.
         */
        Darwin_RuneRange      runetype_ext;
        Darwin_RuneRange      maplower_ext;
        Darwin_RuneRange      mapupper_ext;

        void            *variable;      /* Data which depends on the encoding */
        int             variable_len;   /* how long that data is */
} Darwin_RuneLocale;

extern Darwin_RuneLocale *native_CurrentRuneLocale;

#define Darwin_CTYPE_A        0x00000100L             /* Alpha */
#define Darwin_CTYPE_C        0x00000200L             /* Control */
#define Darwin_CTYPE_D        0x00000400L             /* Digit */
#define Darwin_CTYPE_G        0x00000800L             /* Graph */
#define Darwin_CTYPE_L        0x00001000L             /* Lower */
#define Darwin_CTYPE_P        0x00002000L             /* Punct */
#define Darwin_CTYPE_S        0x00004000L             /* Space */
#define Darwin_CTYPE_U        0x00008000L             /* Upper */
#define Darwin_CTYPE_X        0x00010000L             /* X digit */
#define Darwin_CTYPE_B        0x00020000L             /* Blank */
#define Darwin_CTYPE_R        0x00040000L             /* Print */
#define Darwin_CTYPE_I        0x00080000L             /* Ideogram */
#define Darwin_CTYPE_T        0x00100000L             /* Special */
#define Darwin_CTYPE_Q        0x00200000L             /* Phonogram */
#define Darwin_CTYPE_SW0      0x20000000L             /* 0 width character */
#define Darwin_CTYPE_SW1      0x40000000L             /* 1 width character */
#define Darwin_CTYPE_SW2      0x80000000L             /* 2 width character */
#define Darwin_CTYPE_SW3      0xc0000000L             /* 3 width character */

#define Darwin_RUNE_MAGIC_1 {'R','u','n','e','M','a','g','i'}    /* Indicates version 0 of RuneLocale */

extern char *Darwin_optarg;
extern int Darwin_optind;
extern int Darwin_opterr;
extern int Darwin_optopt;

void nativelib_execute(int nativelib_index);

#endif
