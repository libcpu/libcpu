/* nativelib.c */

/*
   some caveats:
   * off_t is 64 bit
   * ...
*/

#include "nativelib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <dirent.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#ifdef OS_DARWIN
	#include <vis.h>
#endif

#include <utime.h>
#include <time.h>
#include <ctype.h>
#include <pwd.h>
#include <locale.h>
#include <libgen.h>

#include <config.h>

#include "cpu/cpu_generic/ppc_cpu.h"
#include "cpu/cpu_generic/ppc_mmu.h"

#include "spmalloc.h"
#include "endian.h"

void dump(unsigned char* a, int size) {
	int i;
	for (i=0; i<size; i++) {
		printf("%02x ", a[i]);
		if (!((i+1) & 15)) printf("\n");
	}
	printf("\n");
}

inline u_int64_t gpr64(int r1, int r2) {
	return ((u_int64_t)(gCPU.gpr[r1]))<<32 | gCPU.gpr[r2];
}

inline void set_gpr64(int r1, int r2, u_int64_t a) {
	gCPU.gpr[r1] = a >> 32;
	gCPU.gpr[r2] = a & 0xFFFFFFFF;
}


/**********************************************************************************/
/* structure conversion functions                                                 */
/**********************************************************************************/

/* *** Darwin "struct stat" to host "struct stat" conversion ***
   Darwin and FreeBSD share the same structure, only endianness has
   to be converted.
   Linux has an incompatible "struct stat", which is shorter
   (88 bytes instead of Darwin's 96 bytes). We call the host's
   stat() function and convert the structure afterwards.
 */
void convert_stat(struct stat *mystat) {
#ifdef OS_LINUX
	struct darwin_stat newstat;
	newstat.st_dev = Host_to_BE16(mystat->st_dev);
	newstat.st_ino = Host_to_BE16(mystat->st_ino);
	newstat.st_mode = Host_to_BE16(mystat->st_mode);
	newstat.st_nlink = Host_to_BE16(mystat->st_nlink);
	newstat.st_uid = Host_to_BE16(mystat->st_uid);
	newstat.st_gid = Host_to_BE16(mystat->st_gid);
	newstat.st_rdev = Host_to_BE16(mystat->st_rdev);
	newstat.st_size = Host_to_BE64(mystat->st_size);
	newstat.st_blksize = Host_to_BE32(mystat->st_blksize);
	newstat.st_blocks = Host_to_BE64(mystat->st_blocks);
//	newstat.st_atime = Host_to_BE32(mystat->st_atime);
//	newstat.st_atimensec = Host_to_BE32(mystat->st_atime_nsec);
//	newstat.st_mtime = Host_to_BE32(mystat->st_mtime);
//	newstat.st_mtimensec = Host_to_BE32(mystat->st_mtime_nsec);
//	newstat.st_ctime = Host_to_BE32(mystat->st_ctime);
//	newstat.st_ctimensec = Host_to_BE32(mystat->st_ctime_nsec);
	*(struct darwin_stat*)mystat = newstat;
	
//	printf(" %i\n", sizeof());
#else /* Darwin or BSD */
	/* no struct conversion necessary - but maybe endianness conversion */
	mystat->st_dev = Host_to_BE16(mystat->st_dev);
	mystat->st_ino = Host_to_BE16(mystat->st_ino);
	mystat->st_mode = Host_to_BE16(mystat->st_mode);
	mystat->st_nlink = Host_to_BE16(mystat->st_nlink);
	mystat->st_uid = Host_to_BE16(mystat->st_uid);
	mystat->st_gid = Host_to_BE16(mystat->st_gid);
	mystat->st_rdev = Host_to_BE16(mystat->st_rdev);
	mystat->st_size = Host_to_BE64(mystat->st_size);
	mystat->st_blksize = Host_to_BE32(mystat->st_blksize);
	mystat->st_blocks = Host_to_BE64(mystat->st_blocks);
	/* TODO: Convert st_*time members */
#endif
}

int convert_open_flags(int flags) {
#ifdef OS_LINUX
	int hostflags = (flags & 3); /* bits 0 and 1 are identical on Darwin and Linux */
	if (flags & 0x0004) hostflags |= O_NONBLOCK;
	if (flags & 0x0008) hostflags |= O_APPEND;
/*	if (flags & 0x0010) hostflags |= O_SHLOCK;
	if (flags & 0x0020) hostflags |= O_EXLOCK; */
	if (flags & 0x0040) hostflags |= O_ASYNC;
	if (flags & 0x0080) hostflags |= O_FSYNC;
	if (flags & 0x0100) hostflags |= O_NOFOLLOW;
	if (flags & 0x0200) hostflags |= O_CREAT;
	if (flags & 0x0400) hostflags |= O_TRUNC;
	if (flags & 0x0800) hostflags |= O_EXCL;
/*	if (flags & 0x1000) hostflags |= FMARK;
	if (flags & 0x2000) hostflags |= FDEFER;
	if (flags & 0x4000) hostflags |= FHASLOCK;
	if (flags & 0x8000) hostflags |= O_EVTONLY; */
/* Linux doesn't have the flags commented out above - we just ignore them */
	return hostflags;
#else /* Darwin and BSD */
	return flags; /*just do nothing, as flags are compatible */
#endif
}

void convert_tm(struct tm* mytm) {
	/* struct tm consists of ten 32 bit values and a char* - so we can treat
	   it as int tm[10] */
	int i;
	for (i=0; i<10; i++) {
		((int*)mytm)[i] = BE32_toHost(((int*)mytm)[i]);
	}
}

/* caller assumes this to be symmetric */
void convert_utimbuf(struct utimbuf* myutimbuf) {
	myutimbuf->actime = BE32_toHost(myutimbuf->actime);
	myutimbuf->modtime = BE32_toHost(myutimbuf->modtime);
}

FILE *getFILE(unsigned int reg) {
	/* Check if reg points to nativeSF */
	if((DarwinFILE*)reg == &nativeSF[0] || (DarwinFILE*)reg == &nativeSF[1] ||(DarwinFILE*)reg == &nativeSF[2])
		return ((DarwinFILE*)reg)->file;
	return (FILE*)reg;
}

/**********************************************************************************/

/*
int puts(const char *str);
*/
void my_puts() {
	gCPU.gpr[3] = puts((const char*)(gCPU.gpr[3]));
}

/*
int putc(int c, FILE *stream);
*/
void my_putc() {
	gCPU.gpr[3] = putc((int)gCPU.gpr[3], getFILE(gCPU.gpr[4]));
}

/*
int putchar(int c);
*/
void my_putchar() {
	gCPU.gpr[3] = putchar((int)(gCPU.gpr[3]));
}

/*
int fputc(int c, FILE *stream);
*/
void my_fputc() {
	gCPU.gpr[3] = fputc((int)gCPU.gpr[3], getFILE(gCPU.gpr[4]));
}

/*
int fputs(const char *str, FILE *stream);
*/
void my_fputs() {
	gCPU.gpr[3] = fputs((const char *)gCPU.gpr[3], getFILE(gCPU.gpr[4]));
}

/*
int fsync(int fd);
*/
void my_fsync() {
	gCPU.gpr[3] = fsync((int)gCPU.gpr[3]);
}

/*
void setbuf(FILE * restrict stream, char * restrict buf);
*/
void my_setbuf() {
	setbuf(getFILE(gCPU.gpr[3]), (char *)gCPU.gpr[4]);
}

/*
int system(const char *string);
*/
void my_system() {
	gCPU.gpr[3] = system((const char*)(gCPU.gpr[3]));
}

/*
long sysconf(int name);
*/
void my_sysconf() {
	gCPU.gpr[3] = sysconf((int)(gCPU.gpr[3]));
}

/*
int printf(const char * restrict format, ...);
*/
void my_printf() {
	/* TODO: handle variadic */
	gCPU.gpr[3] = printf((const char*)gCPU.gpr[3],gCPU.gpr[4],gCPU.gpr[5],gCPU.gpr[6],gCPU.gpr[7],gCPU.gpr[8],gCPU.gpr[9],gCPU.gpr[10]);
}

/*
int sprintf(char * restrict str, const char * restrict format, ...);
*/
void my_sprintf() {
	/* TODO: handle variadic */
	gCPU.gpr[3] = sprintf((char*)gCPU.gpr[3],(const char*)gCPU.gpr[4],gCPU.gpr[5],gCPU.gpr[6],gCPU.gpr[7],gCPU.gpr[8],gCPU.gpr[9],gCPU.gpr[10]);
}

/*
FILE *fopen(const char * restrict path, const char * restrict mode);
*/
void my_fopen() {
	 /* TODO: endianness in FILE */
	gCPU.gpr[3] = (unsigned int)fopen((const char *)gCPU.gpr[3], (const char *)gCPU.gpr[4]);
}

/*
size_t fwrite(const void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
*/
void my_fwrite() {
	/* TODO: endianness in FILE */
	gCPU.gpr[3] = (unsigned int)fwrite((const void *)gCPU.gpr[3], (size_t)gCPU.gpr[4], (size_t)gCPU.gpr[5], getFILE(gCPU.gpr[6]));
}

/*
char *fgets(char * restrict str, int size, FILE * restrict stream);
*/
void my_fgets() {
	/* TODO: endianness in FILE */
	gCPU.gpr[3] = (unsigned int)fgets((char *)gCPU.gpr[3], (int)gCPU.gpr[4], getFILE(gCPU.gpr[5]));
}

/*
int fflush(FILE *stream);
*/
void my_fflush() {
	/* TODO: endianness in FILE */
	gCPU.gpr[3] = (unsigned int)fflush(getFILE(gCPU.gpr[3]));
}

/*
void exit(int status);
*/
void my_exit() {
	exit((int)gCPU.gpr[3]);
}

/*
void _exit(int status);
*/
void my__exit() {
	_exit((int)gCPU.gpr[3]);
}

/*
char *strchr(const char *s, int c);
*/
void my_strchr() {
	gCPU.gpr[3] = (unsigned int)strchr((const char *)gCPU.gpr[3], (int)gCPU.gpr[4]);
}

/*
char *strrchr(const char *s, int c);
*/
void my_strrchr() {
	gCPU.gpr[3] = (unsigned int)strrchr((const char *)gCPU.gpr[3], (int)gCPU.gpr[4]);
}

/*
size_t strlen(const char *s);
*/
void my_strlen() {
	gCPU.gpr[3] = (unsigned int)strlen((const char *)gCPU.gpr[3]);
}

/*
int strncmp(const char *s1, const char *s2, size_t len);
*/
void my_strncmp() {
	gCPU.gpr[3] = (unsigned int)strncmp((const char *)gCPU.gpr[3], (const char *)gCPU.gpr[4], (size_t)gCPU.gpr[5]);
}

/*
char *getenv(const char *name);
*/
void my_getenv() {
	gCPU.gpr[3] = (unsigned int)getenv((const char *)gCPU.gpr[3]);
}

/*
sig_t signal(int sig, sig_t func);
*/
void my_signal() {
	/* TODO: do something! */
}

/*
int memcmp(const void *b1, const void *b2, size_t len);
*/
void my_memcmp() {
	gCPU.gpr[3] = (unsigned int)memcmp((const void *)gCPU.gpr[3], (const void *)gCPU.gpr[4], (size_t)gCPU.gpr[5]);
}

/*
void *memcpy(void *dst, const void *src, size_t len);
*/
void my_memcpy() {
	gCPU.gpr[3] = (unsigned int)memcpy((void *)gCPU.gpr[3], (void *)gCPU.gpr[4], (size_t)gCPU.gpr[5]);
}

/*
void *memmove(void *dst, const void *src, size_t len);
*/
void my_memmove() {
	gCPU.gpr[3] = (unsigned int)memmove((void *)gCPU.gpr[3], (void *)gCPU.gpr[4], (size_t)gCPU.gpr[5]);
}

/*
char *strncpy(char * restrict dst, const char * restrict src, size_t len);
*/
void my_strncpy() {
	gCPU.gpr[3] = (unsigned int)strncpy((char *)gCPU.gpr[3], (const char *)gCPU.gpr[4], (size_t)gCPU.gpr[5]);
}

/*
int isatty(int fd);
*/
void my_isatty() {
	gCPU.gpr[3] = (unsigned int)isatty((int)gCPU.gpr[3]);
}

/*
int fprintf(FILE * restrict stream, const char * restrict format, ...);
*/
void my_fprintf() {
	/* TODO: handle variadic */
	gCPU.gpr[3] = (unsigned int)fprintf(getFILE(gCPU.gpr[3]), (const char*)gCPU.gpr[4],gCPU.gpr[5],gCPU.gpr[6],gCPU.gpr[7],gCPU.gpr[8],gCPU.gpr[9],gCPU.gpr[10]);
}

/*
char *strcpy(char * restrict dst, const char * restrict src);
*/
void my_strcpy() {
	gCPU.gpr[3] = (unsigned int)strcpy((char*)gCPU.gpr[3], (const char*)gCPU.gpr[4]);
}

/*
int *__error();
*/
void my_error() {
#ifdef OS_DARWIN
	gCPU.gpr[3] = (unsigned int)__error();
#else
	gCPU.gpr[3] = (unsigned int)&errno;
#endif
}

/*
Runetype-Stuff
*/
void my_runetype() {
#ifndef OS_LINUX /* Linux does not know about ___runetype - we'll need another implementation, there */
	gCPU.gpr[3] = (unsigned int)___runetype(gCPU.gpr[3]);
#endif
}

/*
int lstat(const char *path, struct stat *sb);
*/
void my_lstat() {
	gCPU.gpr[3] = (unsigned int)lstat((const char*)gCPU.gpr[3], (struct stat*)gCPU.gpr[4]);
	convert_stat((struct stat*)gCPU.gpr[4]);
}

/*
int stat(const char *path, struct stat *sb);
*/
void my_stat() {
	gCPU.gpr[3] = (unsigned int)stat((const char*)gCPU.gpr[3], (struct stat*)gCPU.gpr[4]);
	convert_stat((struct stat*)gCPU.gpr[4]);
}

/*
int fstat(int fd, struct stat *sb);
*/
void my_fstat() {
	gCPU.gpr[3] = (int)fstat((int)gCPU.gpr[3], (struct stat*)gCPU.gpr[4]);
	convert_stat((struct stat*)gCPU.gpr[4]);
}

/*
int strcmp(const char *s1, const char *s2);
*/
void my_strcmp() {
	gCPU.gpr[3] = (unsigned int)strcmp((char*)gCPU.gpr[3], (const char*)gCPU.gpr[4]);
}

/*
char *strcat(char * restrict s, const char * restrict append);
*/
void my_strcat() {
	gCPU.gpr[3] = (unsigned int)strcat((char*)gCPU.gpr[3], (const char*)gCPU.gpr[4]);
}

/*
int open(const char *path, int flags, mode_t mode);
*/
void my_open() {
	/* "mode" is the same on Linux and Darwin */
	gCPU.gpr[3] = (unsigned int)open((const char*)gCPU.gpr[3], convert_open_flags(gCPU.gpr[4]), (mode_t)gCPU.gpr[5]);
}

/*
DIR *opendir(const char *filename);
*/
void my_opendir() {
	/* TODO: endianness in DIR */
	/* "mode" is the same on Linux and Darwin */
	gCPU.gpr[3] = (unsigned int)opendir((const char*)gCPU.gpr[3]);
}

/*
int strvis(char *dst, const char *src, int flag);
*/
void my_strvis() {
#ifdef OS_DARWIN
	gCPU.gpr[3] = (unsigned int)strvis((char*)gCPU.gpr[3], (const char*)gCPU.gpr[4], (int)gCPU.gpr[5]);
#else
	printf("\nwarning: strvis() not implemented on non-Darwin!\n");
#endif
}

/*
char *strsep(char **stringp, const char *delim);
*/
void my_strsep() {
	gCPU.gpr[3] = (unsigned int)strsep((char **)gCPU.gpr[3], (const char*) gCPU.gpr[4]);
}

/*
void errx(int eval, const char *fmt, ...);
*/
void my_errx() {
	/* TODO: handle variadic */
	errx((int)gCPU.gpr[3], (const char*)gCPU.gpr[4],gCPU.gpr[5],gCPU.gpr[6],gCPU.gpr[7],gCPU.gpr[8],gCPU.gpr[9],gCPU.gpr[10]);
}

/*
void *memset(void *b, int c, size_t len);
*/
void my_memset() {
	gCPU.gpr[3] = (unsigned int)memset((void*)gCPU.gpr[3], (int)gCPU.gpr[4],(size_t)gCPU.gpr[5]);
}

/*
int chmod(const char *path, mode_t mode);
*/
void my_chmod() {
	/* "mode" (r4) is the same on Linux and Darwin */
	gCPU.gpr[3] = (unsigned int)chmod((const char*)gCPU.gpr[3], (mode_t)gCPU.gpr[4]);
}

/*
int unlink(const char *path);
*/
void my_unlink() { 
	gCPU.gpr[3] = (unsigned int)unlink((const char*)gCPU.gpr[3]);
}

/*
ssize_t read(int d, void *buf, size_t nbytes);
*/
void my_read() {
	gCPU.gpr[3] = (unsigned int)read((int)gCPU.gpr[3],(void*)gCPU.gpr[4], (size_t)gCPU.gpr[5]);
}

/*
struct dirent *readdir(DIR *dirp);
*/
void my_readdir() {
	gCPU.gpr[3] = (unsigned int)readdir((DIR*)gCPU.gpr[3]);
}

/*
int atexit(void (*function)(void));
*/
void my_atexit() {
	/* Something should be implemented, here */
	gCPU.gpr[3] = 0;
}

/*
int atoi(const char *nptr);
*/
void my_atoi() {
	gCPU.gpr[3] = atoi((const char *)gCPU.gpr[3]);
}

/*
int fileno(FILE *stream);
*/
void my_fileno() {
	gCPU.gpr[3] = fileno(getFILE(gCPU.gpr[3]));
}

/*
void *malloc(size_t size);
*/
void my_malloc() {
	gCPU.gpr[3] = (unsigned int)spmalloc(gCPU.gpr[3]);
}

/*
void free(void *ptr);
*/
void my_free() {
	spfree((void*)gCPU.gpr[3]);
}

/*
int close(int d);
*/
void my_close() {
	gCPU.gpr[3] = (unsigned int)close(gCPU.gpr[3]);
}

/*
int closedir(DIR *dirp);
*/
void my_closedir() {
	gCPU.gpr[3] = (unsigned int)closedir((DIR*)gCPU.gpr[3]);
}

/*
ssize_t write(int d, const void *buf, size_t nbytes);
*/
void my_write() {
	gCPU.gpr[3] = (unsigned int)write((int)gCPU.gpr[3], (const void*)gCPU.gpr[4], (size_t)gCPU.gpr[5]);
}

/*
int utime(const char *file, const struct utimbuf *timep);
*/
void my_utime() {
	convert_utimbuf((struct utimbuf*)gCPU.gpr[4]); /* convert to host format */
	gCPU.gpr[3] = (unsigned int)utime((const char*)gCPU.gpr[3], (const struct utimbuf*)gCPU.gpr[4]);
	convert_utimbuf((struct utimbuf*)gCPU.gpr[4]); /* convert back */
}

/*
int chown(const char *path, uid_t owner, gid_t group);
*/
void my_chown() {
	gCPU.gpr[3] = (unsigned int)chown((const char*)gCPU.gpr[3], (uid_t)gCPU.gpr[4], (gid_t)gCPU.gpr[5]);
}

/*
void perror(const char *string);
*/
void my_perror() {
	perror((const char*)gCPU.gpr[3]);
}

/*
off_t lseek(int fildes, off_t offset, int whence);
*/
void my_lseek() {
	set_gpr64(3,4,
	          (unsigned int)lseek((int)gCPU.gpr[3], (off_t)gpr64(4,5), (int)gCPU.gpr[6]));
}

/*
char *ctime(const time_t *clock);
*/
void my_ctime() {
	gCPU.gpr[3] = (unsigned int)ctime((const time_t*)gCPU.gpr[3]);
}

unsigned int sorter_pc;
void ppc_cpu_run();

static int my_qsort_sorter(const void* a, const void* b) {
	int saved_pc = gCPU.pc;
	int saved_lr = gCPU.lr;
//	printf("my_qsort_sorter()\n");
	
	gCPU.gpr[3] = (int)a;
	gCPU.gpr[4] = (int)b;
	gCPU.lr = 0xdeadbeef;
	gCPU.pc = sorter_pc;
//	printf("+run() %x\n", gCPU.pc);
	ppc_cpu_run();
//	printf("-run()\n");

	gCPU.pc = saved_pc;
	gCPU.lr = saved_lr;
	return gCPU.gpr[3];
}

/*
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
*/
void my_qsort() {
	/* this one is tricky: the application passes us a pointer to a function
	   that can compare two elements of the array */
	/* TODO: for now, just do nothing! :-) */
//	printf("void qsort(void *base = %x, size_t nmemb = %x, size_t size = %x, int (*compar)(const void *, const void *) = %x);\n", gCPU.gpr[3], gCPU.gpr[4], gCPU.gpr[5], gCPU.gpr[6]);
	sorter_pc = gCPU.gpr[6];
	qsort((void*)gCPU.gpr[3], (size_t)gCPU.gpr[4], (size_t)gCPU.gpr[5], my_qsort_sorter);
}

/*
int getchar();
*/
void my_getchar() {
	gCPU.gpr[3] = getchar();
}

/*
struct tm *localtime(const time_t *clock);
*/
void my_localtime() {
	gCPU.gpr[3] = (unsigned int)localtime((const time_t*)gCPU.gpr[3]);
	convert_tm((struct tm*)gCPU.gpr[3]);
}

/*
time_t time(time_t *tloc);
*/
void my_time() {
	/* Since the return value also is stored at the specified location, we have to adjust the endianess */
	time_t mytime = (unsigned int)time((time_t *)gCPU.gpr[3]);
	if(gCPU.gpr[3])
		*((time_t *)gCPU.gpr[3]) = Host_to_BE32(*((time_t *)gCPU.gpr[3]));
	gCPU.gpr[3] = (unsigned int) mytime;
}

/*
int fclose(FILE *stream);
*/
void my_fclose() {
	gCPU.gpr[3] = (unsigned int)fclose(getFILE(gCPU.gpr[3]));
}

/*
int getc(FILE *stream);
*/
void my_getc() {
	gCPU.gpr[3] = (unsigned int)getc(getFILE(gCPU.gpr[3]));
}

/*
size_t fread(void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
*/
void my_fread() {
	gCPU.gpr[3] = (unsigned int)fread((void*)gCPU.gpr[3], (size_t)gCPU.gpr[4], (size_t)gCPU.gpr[5], getFILE(gCPU.gpr[6]));
}

/*
char *mktemp(char *template);
*/
void my_mktemp() {
	/* GNU linker says: "warning: the use of `mktemp' is dangerous, better use `mkstemp'" */
	gCPU.gpr[3] = (unsigned int)mktemp((char*)gCPU.gpr[3]);
}

/*
int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size);
*/
void my_setvbuf() {
	/* "mode" is the same on Linux and Darwin */
	gCPU.gpr[3] = (unsigned int)setvbuf(getFILE(gCPU.gpr[3]), (char*)gCPU.gpr[4], (int)gCPU.gpr[5], (size_t)gCPU.gpr[6]);
}

/*
int ferror(FILE *stream);
*/
void my_ferror() {
	gCPU.gpr[3] = (unsigned int)ferror(getFILE(gCPU.gpr[3]));
}

/*
long ftell(FILE *stream);
*/
void my_ftell() {
	gCPU.gpr[3] = (unsigned int)ftell(getFILE(gCPU.gpr[3]));
}

/*
int fseek(FILE *stream, long offset, int whence);
*/
void my_fseek() {
	/* "whence" is the same on Linux and Darwin */
	gCPU.gpr[3] = (unsigned int)fseek(getFILE(gCPU.gpr[3]),(long)gCPU.gpr[4], (int)gCPU.gpr[5]);
}

/*
int rename(const char *from, const char *to);
*/
void my_rename() {
	gCPU.gpr[3] = (unsigned int)rename((char*)gCPU.gpr[3], (char*)gCPU.gpr[4]);
}

/*
void warn(const char *fmt, ...);
*/
void my_warn() {
	/* TODO: Handle variadic */
	warn((const char *)gCPU.gpr[3],gCPU.gpr[4],gCPU.gpr[5],gCPU.gpr[6],gCPU.gpr[7],gCPU.gpr[8],gCPU.gpr[9],gCPU.gpr[10]);
}

/*
int getopt(int argc, char * const argv[], const char *optstring);
*/
void my_getopt() {
	/* Convert endianess in pointers
	   isn't there a better way? */
	int i, argc = gCPU.gpr[3];
	char **argv = (char **)gCPU.gpr[4];
	for(i=0; i<argc; i++)
		argv[i] = (char*)BE32_toHost((unsigned int)argv[i]);
	/* convert Darwin optargs into local ones
	 */
	optarg = (char *)Host_to_BE32 ((unsigned int)Darwin_optarg);
	optind = Host_to_BE32 (Darwin_optind);
	opterr = Host_to_BE32 (Darwin_opterr);
	optopt = Host_to_BE32 (Darwin_optopt);
	
	gCPU.gpr[3] = getopt(argc, argv, (const char *)gCPU.gpr[5]);
	
	/* Convert back again after returning */
	for(i=0; i<argc; i++)
		argv[i] = (char*)BE32_toHost((unsigned int)argv[i]);
	Darwin_optarg = (char *)BE32_toHost ((unsigned int)optarg);
	Darwin_optind = BE32_toHost (optind);
	Darwin_opterr = BE32_toHost (opterr);
	Darwin_optopt = BE32_toHost (optopt);
}

/*
char *setlocale(int category, const char *locale);
*/
void my_setlocale() {
	gCPU.gpr[3] = (unsigned int)setlocale((int)gCPU.gpr[3], (const char*)gCPU.gpr[4]);
}

/*
mode_t umask(mode_t numask);
*/
void my_umask() {
	gCPU.gpr[3] = (unsigned int)umask((mode_t)gCPU.gpr[3]);
}

/*
int tolower(int c);
*/
void my_tolower() {
	gCPU.gpr[3] = tolower(gCPU.gpr[3]);
}

/*
int toupper(int c);
*/
void my_toupper() {
	gCPU.gpr[3] = toupper(gCPU.gpr[3]);
}

/*
int ioctl(int d, unsigned long request, char *argp);
Very nice, on Darwin this function is not variadic like on other systems...
*/
void my_ioctl() {
	gCPU.gpr[3] = (unsigned int)ioctl((int)gCPU.gpr[3], (unsigned long)gCPU.gpr[4], gCPU.gpr[5]);
}

/*
int access(const char *path, int mode);
*/
void my_access() {
	gCPU.gpr[3] = access((const char*)gCPU.gpr[3], (int)gCPU.gpr[4]);
}

/*
uid_t getuid(void);
*/
void my_getuid() {
	gCPU.gpr[3] = (unsigned int)getuid();
}

/*
gid_t getgid(void);
*/
void my_getgid() {
	gCPU.gpr[3] = (unsigned int)getgid();
}

/*
int gethostname(char *name, size_t namelen);
*/
void my_gethostname() {
	gCPU.gpr[3] = gethostname((char *)gCPU.gpr[3], (size_t)gCPU.gpr[4]);
}

/*
int sethostname(const char *name, int namelen);
*/
void my_sethostname() {
	gCPU.gpr[3] = sethostname((const char *)gCPU.gpr[3], (int)gCPU.gpr[4]);
}

/*
int getdomainname(char *name, int namelen);
*/
void my_getdomainname() {
	gCPU.gpr[3] = getdomainname((char *)gCPU.gpr[3], gCPU.gpr[4]);
}

/*
int setdomainname(const char *name, int namelen);
*/
void my_setdomainname() {
	gCPU.gpr[3] = setdomainname((const char *)gCPU.gpr[3], gCPU.gpr[4]);
}

/*
unsigned int sleep(unsigned int seconds);
*/
void my_sleep() {
	gCPU.gpr[3] = sleep(gCPU.gpr[3]);
}

/*
void err(int eval, const char *fmt, ...);
*/
void my_err() {
	/* TODO: Handle variadic */
	err(gCPU.gpr[3], (const char *)gCPU.gpr[4],gCPU.gpr[5],gCPU.gpr[6],gCPU.gpr[7],gCPU.gpr[8],gCPU.gpr[9],gCPU.gpr[10]);
}

/*
char *dirname (const char *path);
*/
char *my_dirname() {
	gCPU.gpr[3] = (unsigned int)dirname ((const char *)gCPU.gpr[3]);
}

/*
mode_t getmode (const void *set, mode_t mode);
*/
void my_getmode() {
	gCPU.gpr[3] = (unsigned int)getmode ((const void *)gCPU.gpr[3],
					     (mode_t)gCPU.gpr[4]);
}

/*
void *setmode (const char *mode_str);
*/
void my_setmode() {
	gCPU.gpr[3] = (unsigned int)setmode ((const char *)gCPU.gpr[3]);
}

/*
int mkdir (const char *path, mode_t mode);
*/
void my_mkdir() {
	gCPU.gpr[3] = mkdir ((const char *)gCPU.gpr[3], (mode_t)gCPU.gpr[4]);
}

/*
int rmdir (const char *path);
*/
void my_rmdir() {
	gCPU.gpr[3] = rmdir ((const char *)gCPU.gpr[3]);
}

/*
int snprintf (char * restrict str, size_t size, const char * restrict format, ...);
*/
void my_snprintf() {
	gCPU.gpr[3] = snprintf ((char *)gCPU.gpr[3], (size_t)gCPU.gpr[4],
				(const char *)gCPU.gpr[5],
				// variadic
				gCPU.gpr[6], gCPU.gpr[7], gCPU.gpr[8],
				gCPU.gpr[9], gCPU.gpr[10]
				);
}

/*
char *getcwd(char *buf, size_t size);
*/
void my_getcwd() {
	gCPU.gpr[3] = (unsigned int)getcwd((char *)gCPU.gpr[3], (size_t)gCPU.gpr[4]);
}

/*
void unsetenv(const char *name);
*/
void my_unsetenv() {
	unsetenv((const char *)gCPU.gpr[3]);
}

/*
long strtol (const char * restrict nptr, char ** restrict endptr, int base);
*/
void my_strtol() {
	char **endptr = (char **)gCPU.gpr[4];
	*endptr = (char *)BE32_toHost ((unsigned int)*endptr);
	gCPU.gpr[3] = strtol ((const char *)gCPU.gpr[3], endptr, (int)gCPU.gpr[5]);
	*endptr = (char *)Host_to_BE32 ((unsigned int)*endptr);
}

/*
int setegid (uid_t euid);
*/
void my_setegid() {
	gCPU.gpr[3] = setegid ((uid_t)gCPU.gpr[3]);
}

/*
gid_t getegid (void);
*/
void my_getegid() {
	gCPU.gpr[3] = (unsigned int)getegid();
}

/*
uid_t geteuid (void);
*/
void my_geteuid() {
	gCPU.gpr[3] = (unsigned int)geteuid();
}

/*
void strmode (mode_t mode, char *bp);
*/
void my_strmode() {
	strmode ((mode_t)gCPU.gpr[3], (char *)gCPU.gpr[4]);
}

/*
void warnx (const char *fmt, ...);
*/
void my_warnx() {
	/* TODO: variadic */
	warnx ((const char *)gCPU.gpr[3], gCPU.gpr[4], gCPU.gpr[5],
	       gCPU.gpr[6], gCPU.gpr[7], gCPU.gpr[8], gCPU.gpr[9],
	       gCPU.gpr[10]);
}

/* This is a list with pointers to functions provided by the local environment */
/* Please do not forget to justify the #define NATIVELIBCOUNT in nativelib.h */
NativeLibEntry NativeLibs[] = {
	{"_puts",		(NativeLibFuncPtr) &my_puts		},
	{"_putc",		(NativeLibFuncPtr) &my_putc		},
	{"_putchar",		(NativeLibFuncPtr) &my_putchar		},
	{"_system",		(NativeLibFuncPtr) &my_system		},
	{"_printf",		(NativeLibFuncPtr) &my_printf		},
	{"_fopen",		(NativeLibFuncPtr) &my_fopen		},
	{"_fwrite",		(NativeLibFuncPtr) &my_fwrite		},
	{"_exit",		(NativeLibFuncPtr) &my_exit		},
	{"__exit",		(NativeLibFuncPtr) &my__exit		},
	{"_abort",		(NativeLibFuncPtr) 0			},
	{"_atexit",		(NativeLibFuncPtr) &my_atexit		},
	{"_atoi",		(NativeLibFuncPtr) my_atoi		},
	{"_calloc",		(NativeLibFuncPtr) 0			},
	{"_chmod",		(NativeLibFuncPtr) my_chmod		},
	{"_chown",		(NativeLibFuncPtr) my_chown		},
	{"_close",		(NativeLibFuncPtr) my_close		},
	{"_closedir",		(NativeLibFuncPtr) my_closedir		},
	{"_ctime",		(NativeLibFuncPtr) my_ctime		},
	{"_errno",		(NativeLibFuncPtr) 0			},
	{"_errx",		(NativeLibFuncPtr) my_errx		},
	{"_fflush",		(NativeLibFuncPtr) my_fflush		},
	{"_fgets",		(NativeLibFuncPtr) my_fgets		},
	{"_fprintf",		(NativeLibFuncPtr) my_fprintf		},
	{"_fputc",		(NativeLibFuncPtr) my_fputc		},
	{"_fputs",		(NativeLibFuncPtr) my_fputs		},
	{"_sync",		(NativeLibFuncPtr) my_fsync		},
	{"_setbuf",		(NativeLibFuncPtr) my_setbuf		},
	{"_free",		(NativeLibFuncPtr) my_free		},
	{"_fstat",		(NativeLibFuncPtr) my_fstat		},
	{"_getenv",		(NativeLibFuncPtr) my_getenv		},
	{"_isatty",		(NativeLibFuncPtr) my_isatty		},
	{"_lseek",		(NativeLibFuncPtr) my_lseek		},
	{"_lstat",		(NativeLibFuncPtr) my_lstat		},
	{"_malloc",		(NativeLibFuncPtr) my_malloc		},
	{"_realloc",		(NativeLibFuncPtr) 0			},
	{"_memcmp",		(NativeLibFuncPtr) my_memcmp		},
	{"_memcpy",		(NativeLibFuncPtr) my_memcpy		},
	{"_memset",		(NativeLibFuncPtr) my_memset		},
	{"_memmove",		(NativeLibFuncPtr) my_memmove		},
	{"_open",		(NativeLibFuncPtr) my_open		},
	{"_opendir",		(NativeLibFuncPtr) my_opendir		},
	{"_perror",		(NativeLibFuncPtr) my_perror		},
	{"_sprintf",		(NativeLibFuncPtr) my_sprintf		},
	{"_read",		(NativeLibFuncPtr) my_read		},
	{"_readdir",		(NativeLibFuncPtr) my_readdir		},
	{"_signal",		(NativeLibFuncPtr) my_signal		},
	{"_stat",		(NativeLibFuncPtr) my_stat		},
	{"_strcat",		(NativeLibFuncPtr) my_strcat		},
	{"_strcmp",		(NativeLibFuncPtr) my_strcmp		},
	{"_strcpy",		(NativeLibFuncPtr) my_strcpy		},
	{"_strcspn",		(NativeLibFuncPtr) 0			},
	{"_strlen",		(NativeLibFuncPtr) my_strlen		},
	{"_strncmp",		(NativeLibFuncPtr) my_strncmp		},
	{"_strncpy",		(NativeLibFuncPtr) my_strncpy		},
	{"_strchr",		(NativeLibFuncPtr) my_strchr		},
	{"_strrchr",		(NativeLibFuncPtr) my_strrchr		},
	{"_strspn",		(NativeLibFuncPtr) 0			},
	{"_strvis",		(NativeLibFuncPtr) my_strvis		},
	{"_strsep",		(NativeLibFuncPtr) my_strsep		},
	{"_strerror",		(NativeLibFuncPtr) 0			},
	{"_sysconf",		(NativeLibFuncPtr) my_sysconf		},
	{"_unlink",		(NativeLibFuncPtr) my_unlink		},
	{"_utime",		(NativeLibFuncPtr) my_utime		},
	{"_write",		(NativeLibFuncPtr) my_write		},
	{"_fileno",		(NativeLibFuncPtr) my_fileno		},
	{"___error",		(NativeLibFuncPtr) my_error		},
	{"____runetype",	(NativeLibFuncPtr) my_runetype		},
	{"_fseek",		(NativeLibFuncPtr) my_fseek		},
	{"_ferror",		(NativeLibFuncPtr) my_ferror		},
	{"_fread",		(NativeLibFuncPtr) my_fread		},
	{"_mktemp",		(NativeLibFuncPtr) my_mktemp		},
	{"_fclose",		(NativeLibFuncPtr) my_fclose		},
	{"_rename",		(NativeLibFuncPtr) my_rename		},
	{"_mktime",		(NativeLibFuncPtr) 0			},
	{"_localtime",		(NativeLibFuncPtr) my_localtime		},
	{"_time",		(NativeLibFuncPtr) my_time		},
	{"_qsort",		(NativeLibFuncPtr) my_qsort		},
	{"_getchar",		(NativeLibFuncPtr) my_getchar		},
	{"_fdopen",		(NativeLibFuncPtr) 0			},
	{"_setvbuf",		(NativeLibFuncPtr) my_setvbuf		},
	{"_sscanf",		(NativeLibFuncPtr) 0			},
	{"_readlink",		(NativeLibFuncPtr) 0			},
	{"_clearerr",		(NativeLibFuncPtr) 0			},
	{"_ftell",		(NativeLibFuncPtr) my_ftell		},
	{"_getc",		(NativeLibFuncPtr) my_getc		},
	{"_rmdir",		(NativeLibFuncPtr) my_rmdir		},
	{"_mbtowc",		(NativeLibFuncPtr) 0			},
	{"_warn",		(NativeLibFuncPtr) my_warn		},
	{"_getopt",		(NativeLibFuncPtr) my_getopt		},
	{"_setlocale",		(NativeLibFuncPtr) my_setlocale		},
	{"_tcsetattr",		(NativeLibFuncPtr) 0			},
	{"_tcgetattr",		(NativeLibFuncPtr) 0			},
	{"_gmtime",		(NativeLibFuncPtr) 0			},
	{"_symlink",		(NativeLibFuncPtr) 0			},
	{"_mkdir",		(NativeLibFuncPtr) my_mkdir		},
	{"_umask",		(NativeLibFuncPtr) my_umask		},
	{"____tolower",		(NativeLibFuncPtr) my_tolower		},
	{"____toupper",		(NativeLibFuncPtr) my_toupper		},
	{"_ioctl",		(NativeLibFuncPtr) my_ioctl		},
	{"_access",		(NativeLibFuncPtr) my_access		},
	{"_getuid",		(NativeLibFuncPtr) my_getuid		},
	{"_getgid",		(NativeLibFuncPtr) my_getgid		},
	{"_gethostname",	(NativeLibFuncPtr) my_gethostname	},
	{"_sethostname",	(NativeLibFuncPtr) my_sethostname	},
	{"_getdomainname",	(NativeLibFuncPtr) my_getdomainname	},
	{"_setdomainname",	(NativeLibFuncPtr) my_setdomainname	},
	{"_sleep",		(NativeLibFuncPtr) my_sleep		},
	{"_err",		(NativeLibFuncPtr) my_err		},
	{"_dirname",		(NativeLibFuncPtr) my_dirname		},
	{"_getmode",		(NativeLibFuncPtr) my_getmode		},
	{"_setmode",		(NativeLibFuncPtr) my_setmode		},
	{"_snprintf",		(NativeLibFuncPtr) my_snprintf		},
	{"_getcwd",		(NativeLibFuncPtr) my_getcwd		},
	{"_unsetenv",		(NativeLibFuncPtr) my_unsetenv		},
	{"_strtol",		(NativeLibFuncPtr) my_strtol		},
	{"_setegid",		(NativeLibFuncPtr) my_setegid		},
	{"_getegid",		(NativeLibFuncPtr) my_getegid		},
	{"_geteuid",		(NativeLibFuncPtr) my_geteuid		},
	{"_strmode",		(NativeLibFuncPtr) my_strmode		},
	{"_warnx",		(NativeLibFuncPtr) my_warnx		},
};

const unsigned int kNativeLibCount = sizeof (NativeLibs) / sizeof (NativeLibEntry);

void nativelib_execute(int nativelib_index) {
	NativeLibFuncPtr p = NativeLibs[nativelib_index].addr;
#ifdef DEBUG
	printf("\nlibc function %i (%s) at %08x\n", nativelib_index, NativeLibs[nativelib_index].name, p);
#endif
	if (!p) {
		printf("\nunknown libc function %i (%s) at %08x\n", nativelib_index, NativeLibs[nativelib_index].name, (unsigned int)p);
		exit(1);
	} else {
		p();
	}
	gCPU.npc = gCPU.lr & 0xfffffffc;
}

/* Create an array which is needed for handling references to stdout, stdin, stderr */
DarwinFILE nativeSF[] = {
	{stdin, ""},
	{stdout, ""},
	{stderr, ""}
};

#ifndef OS_DARWIN /* _RuneLocale is handled more natively on Mac OS */
Darwin_RuneLocale Darwin_DefaultRuneLocale = {
    Darwin_RUNE_MAGIC_1,
    "NONE",
    NULL /*_none_sgetrune */,
    NULL /*_none_sputrune */,
    0xFFFD,

    {   /*00*/  Host_to_BE32(Darwin_CTYPE_C),
		  Host_to_BE32(Darwin_CTYPE_C),
		  Host_to_BE32(Darwin_CTYPE_C),
		  Host_to_BE32(Darwin_CTYPE_C),
		  Host_to_BE32(Darwin_CTYPE_C),
		  Host_to_BE32(Darwin_CTYPE_C),
		  Host_to_BE32(Darwin_CTYPE_C),
		  Host_to_BE32(Darwin_CTYPE_C),
        /*08*/  Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C|Darwin_CTYPE_S|Darwin_CTYPE_B),
		Host_to_BE32(Darwin_CTYPE_C|Darwin_CTYPE_S),
		Host_to_BE32(Darwin_CTYPE_C|Darwin_CTYPE_S),
		Host_to_BE32(Darwin_CTYPE_C|Darwin_CTYPE_S),
		Host_to_BE32(Darwin_CTYPE_C|Darwin_CTYPE_S),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
        /*10*/  Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
        /*18*/  Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
		Host_to_BE32(Darwin_CTYPE_C),
        /*20*/  Host_to_BE32(Darwin_CTYPE_S|Darwin_CTYPE_B|Darwin_CTYPE_R),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
        /*28*/  Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
        /*30*/  Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|0),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|1),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|2),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|3),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|4),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|5),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|6),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|7),
        /*38*/  Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|8),
		Host_to_BE32(Darwin_CTYPE_D|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_X|9),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
        /*40*/  Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|10),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|11),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|12),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|13),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|14),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|15),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
        /*48*/  Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
        /*50*/  Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
        /*58*/  Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_U|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
        /*60*/  Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|10),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|11),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|12),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|13),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|14),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_X|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A|15),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
        /*68*/  Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
        /*70*/  Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
        /*78*/  Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_L|Darwin_CTYPE_R|Darwin_CTYPE_G|Darwin_CTYPE_A),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_P|Darwin_CTYPE_R|Darwin_CTYPE_G),
		Host_to_BE32(Darwin_CTYPE_C),
},
{   0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,
        0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,
	0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,
	0x18,   0x19,   0x1a,   0x1b,   0x1c,   0x1d,   0x1e,   0x1f,
	0x20,   0x21,   0x22,   0x23,   0x24,   0x25,   0x26,   0x27,
	0x28,   0x29,   0x2a,   0x2b,   0x2c,   0x2d,   0x2e,   0x2f,
	0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,
	0x38,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   0x3f,
	0x40,   'a',    'b',    'c',    'd',    'e',    'f',    'g',
	'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
	'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
	'x',    'y',    'z',    0x5b,   0x5c,   0x5d,   0x5e,   0x5f,
	0x60,   'a',    'b',    'c',    'd',    'e',    'f',    'g',
	'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
	'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
	'x',    'y',    'z',    0x7b,   0x7c,   0x7d,   0x7e,   0x7f,
	0x80,   0x81,   0x82,   0x83,   0x84,   0x85,   0x86,   0x87,
	0x88,   0x89,   0x8a,   0x8b,   0x8c,   0x8d,   0x8e,   0x8f,
	0x90,   0x91,   0x92,   0x93,   0x94,   0x95,   0x96,   0x97,
	0x98,   0x99,   0x9a,   0x9b,   0x9c,   0x9d,   0x9e,   0x9f,
	0xa0,   0xa1,   0xa2,   0xa3,   0xa4,   0xa5,   0xa6,   0xa7,
	0xa8,   0xa9,   0xaa,   0xab,   0xac,   0xad,   0xae,   0xaf,
	0xb0,   0xb1,   0xb2,   0xb3,   0xb4,   0xb5,   0xb6,   0xb7,
	0xb8,   0xb9,   0xba,   0xbb,   0xbc,   0xbd,   0xbe,   0xbf,
	0xc0,   0xc1,   0xc2,   0xc3,   0xc4,   0xc5,   0xc6,   0xc7,
	0xc8,   0xc9,   0xca,   0xcb,   0xcc,   0xcd,   0xce,   0xcf,
	0xd0,   0xd1,   0xd2,   0xd3,   0xd4,   0xd5,   0xd6,   0xd7,
	0xd8,   0xd9,   0xda,   0xdb,   0xdc,   0xdd,   0xde,   0xdf,
	0xe0,   0xe1,   0xe2,   0xe3,   0xe4,   0xe5,   0xe6,   0xe7,
	0xe8,   0xe9,   0xea,   0xeb,   0xec,   0xed,   0xee,   0xef,
	0xf0,   0xf1,   0xf2,   0xf3,   0xf4,   0xf5,   0xf6,   0xf7,
	0xf8,   0xf9,   0xfa,   0xfb,   0xfc,   0xfd,   0xfe,   0xff,
},
{   0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07,
        0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,
	0x10,   0x11,   0x12,   0x13,   0x14,   0x15,   0x16,   0x17,
	0x18,   0x19,   0x1a,   0x1b,   0x1c,   0x1d,   0x1e,   0x1f,
	0x20,   0x21,   0x22,   0x23,   0x24,   0x25,   0x26,   0x27,
	0x28,   0x29,   0x2a,   0x2b,   0x2c,   0x2d,   0x2e,   0x2f,
	0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,
	0x38,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   0x3f,
	0x40,   'A',    'B',    'C',    'D',    'E',    'F',    'G',
	'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
	'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
	'X',    'Y',    'Z',    0x5b,   0x5c,   0x5d,   0x5e,   0x5f,
	0x60,   'A',    'B',    'C',    'D',    'E',    'F',    'G',
	'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
	'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
	'X',    'Y',    'Z',    0x7b,   0x7c,   0x7d,   0x7e,   0x7f,
	0x80,   0x81,   0x82,   0x83,   0x84,   0x85,   0x86,   0x87,
	0x88,   0x89,   0x8a,   0x8b,   0x8c,   0x8d,   0x8e,   0x8f,
	0x90,   0x91,   0x92,   0x93,   0x94,   0x95,   0x96,   0x97,
	0x98,   0x99,   0x9a,   0x9b,   0x9c,   0x9d,   0x9e,   0x9f,
	0xa0,   0xa1,   0xa2,   0xa3,   0xa4,   0xa5,   0xa6,   0xa7,
	0xa8,   0xa9,   0xaa,   0xab,   0xac,   0xad,   0xae,   0xaf,
	0xb0,   0xb1,   0xb2,   0xb3,   0xb4,   0xb5,   0xb6,   0xb7,
	0xb8,   0xb9,   0xba,   0xbb,   0xbc,   0xbd,   0xbe,   0xbf,
	0xc0,   0xc1,   0xc2,   0xc3,   0xc4,   0xc5,   0xc6,   0xc7,
	0xc8,   0xc9,   0xca,   0xcb,   0xcc,   0xcd,   0xce,   0xcf,
	0xd0,   0xd1,   0xd2,   0xd3,   0xd4,   0xd5,   0xd6,   0xd7,
	0xd8,   0xd9,   0xda,   0xdb,   0xdc,   0xdd,   0xde,   0xdf,
	0xe0,   0xe1,   0xe2,   0xe3,   0xe4,   0xe5,   0xe6,   0xe7,
	0xe8,   0xe9,   0xea,   0xeb,   0xec,   0xed,   0xee,   0xef,
	0xf0,   0xf1,   0xf2,   0xf3,   0xf4,   0xf5,   0xf6,   0xf7,
	0xf8,   0xf9,   0xfa,   0xfb,   0xfc,   0xfd,   0xfe,   0xff,
},
};

Darwin_RuneLocale *native_CurrentRuneLocale = (Darwin_RuneLocale*)BE32_toHost((unsigned long)&Darwin_DefaultRuneLocale);
#else /* Darwin/PPC only */
Darwin_RuneLocale *native_CurrentRuneLocale = (Darwin_RuneLocale*)BE32_toHost((unsigned long)&_DefaultRuneLocale);
#endif

/* Provide some references for extern variables provided around getopt call */
char *Darwin_optarg = (char*)BE32_toHost((unsigned int)optarg);
int Darwin_optind = BE32_toHost(optind);
int Darwin_opterr = BE32_toHost(opterr);
int Darwin_optopt = BE32_toHost(optopt);
/* Note that on Darwin and BSD there is also optreset which we leave out for now since it is not supported on linux */
