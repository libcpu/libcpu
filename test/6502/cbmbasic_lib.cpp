//LLVM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>  // getcwd
#else
#include <unistd.h>
#endif
#include "stat.h"
#include "readdir.h"
#define SETZ(a) Z=a;
#define SETN(a) N=a;
#define SETSZ(a) Z = (a)? 0:1; N = ((signed char)(a))<0?1:0
#define SETV(a) V = (a)? 1:0
#define SETNC(a) C = (a)&0x100? 0:1
//LLVM

#ifndef sun
#define __inline inline
#else
#define __inline
#endif

static unsigned char *RAM = 0;
static unsigned short PC;
static unsigned char A, X, Y, S;
static unsigned int N, V, D, I, Z, C;

static int
stack4(unsigned short a, unsigned short b, unsigned short c, unsigned short d) {
//	printf("stack4: %x,%x,%x,%x\n", a, b, c, d);
	if (*(unsigned short*)(&RAM[0x0100+S+1]) + 1 != a) return 0;
	if (*(unsigned short*)(&RAM[0x0100+S+3]) + 1 != b) return 0;
	if (*(unsigned short*)(&RAM[0x0100+S+5]) + 1 != c) return 0;
	if (*(unsigned short*)(&RAM[0x0100+S+7]) + 1 != d) return 0;
	return 1;
}

/* These are required for LDA_STA_CALLOUT, do nothing, and have no performance impact */
static inline void
func_lda_abs(unsigned short a) {
	A = RAM[a];
	SETSZ(A);
}

static inline void
func_bit_abs(unsigned short a) {
	unsigned char temp8 = RAM[a];
	SETSZ(temp8);
	SETV((temp8>>6)&1); 
}
static inline void
func_sta_abs(unsigned short a) {
		RAM[a] = A;
}

/*
 * CHRGET/CHRGOT
 * CBMBASIC implements CHRGET/CHRGOT as self-modifying
 * code in the zero page. This cannot be done with
 * static recompilation, so here is a reimplementation
 * of these functions in C.
0073   E6 7A      INC $7A
0075   D0 02      BNE $0079
0077   E6 7B      INC $7B
0079   AD XX XX   LDA $XXXX
007C   C9 3A      CMP #$3A   ; colon
007E   B0 0A      BCS $008A
0080   C9 20      CMP #$20   ; space
0082   F0 EF      BEQ $0073
0084   38         SEC
0085   E9 30      SBC #$30   ; 0
0087   38         SEC
0088   E9 D0      SBC #$D0
008A   60         RTS
*/
static void
CHRGET_common(int inc) {
	unsigned short temp16;
	if (!inc) goto CHRGOT_start;
CHRGET_start:
	RAM[0x7A]++; SETSZ(RAM[0x7A]);
	if (!Z) goto CHRGOT_start;
	RAM[0x7B]++; SETSZ(RAM[0x7B]);
CHRGOT_start:
	A = RAM[RAM[0x7A] | RAM[0x7B]<<8]; SETSZ(A);
	temp16 = ((unsigned short)A) - ((unsigned short)0x3A); SETNC(temp16); SETSZ(temp16&0xFF);
	if (C) return;
	temp16 = ((unsigned short)A) - ((unsigned short)0x20); SETNC(temp16); SETSZ(temp16&0xFF);
	if (Z) goto CHRGET_start;
	C = 1;
	temp16 = (unsigned short)A-(unsigned short)0x30-(unsigned short)(1-C); SETV(((A ^ temp16) & 0x80) && ((A ^ 0x30) & 0x80)); A = (unsigned char)temp16; SETSZ(A); SETNC(temp16);
	C = 1;
	temp16 = (unsigned short)A-(unsigned short)0xD0-(unsigned short)(1-C); SETV(((A ^ temp16) & 0x80) && ((A ^ 0xD0) & 0x80)); A = (unsigned char)temp16; SETSZ(A); SETNC(temp16);
}

static void
CHRGET() {
	CHRGET_common(1);
}
static void
CHRGOT() {
	CHRGET_common(0);
}


/************************************************************/
/* KERNAL interface implementation                          */
/* http://members.tripod.com/~Frank_Kontros/kernal/addr.htm */
/************************************************************/

/* KERNAL constants */
#if 0
#define RAM_BOT 0x0400 /* we could just as well start at 0x0400, as there is no screen RAM */
#else
#define RAM_BOT 0x0800
#endif
#define RAM_TOP 0xA000
#define KERN_ERR_NONE					0
#define KERN_ERR_FILE_NOT_FOUND			4
#define KERN_ERR_DEVICE_NOT_PRESENT		5
#define KERN_ERR_MISSING_FILE_NAME		8
#define KERN_ERR_ILLEGAL_DEVICE_NUMBER	9

/* KERNAL internal state */
static unsigned char kernal_msgflag, kernal_status = 0;
static unsigned short kernal_filename;
static unsigned char kernal_filename_len;
static unsigned char kernal_lfn, kernal_dev, kernal_sec;
static int kernal_quote = 0;

/* shell script hack */
static int readycount = 0;
static int interactive = 1;
static FILE *f;

#if 0
static void
init_os(int argc, char **argv) {
	if (argc>1) {
		interactive = 0;
		f = fopen(argv[1], "r");
		if (fgetc(f)=='#') {
			char c;
			do {
				c = fgetc(f);
			} while((c!=13)&&(c!=10));
		} else {
			fseek(f, 0, SEEK_SET);
		}
	} else {
		interactive = 1;
		f = NULL;
	}
}
#endif

static inline void
SETMSG() {
		kernal_msgflag = A;
		A = kernal_status;
}


static inline void
MEMTOP() {
#if DEBUG /* CBMBASIC doesn't do this */
	if (!C) {
		printf("UNIMPL: set top of RAM");
		exit(1);
	}
#endif
	X = RAM_TOP&0xFF;
	Y = RAM_TOP>>8;
}

/* MEMBOT */
static inline void
MEMBOT() {
#if DEBUG /* CBMBASIC doesn't do this */
	if (!C) {
		printf("UNIMPL: set bot of RAM");
		exit(1);
	}
#endif
	X = RAM_BOT&0xFF;
	Y = RAM_BOT>>8;
}

/* READST */
static inline void
READST() {
		A = kernal_status;
}

/* SETLFS */
static inline void
SETLFS() {
		kernal_lfn = A;
		kernal_dev = X;
		kernal_sec = Y;
}

/* SETNAM */
static inline void
SETNAM() {
		kernal_filename = X | Y<<8;
		kernal_filename_len = A;
}

/* OPEN */
static inline void
OPEN() {
		printf("UNIMPL: OPEN\n");
		exit(1);
}

/* CLOSE */
static inline void
CLOSE() {
		printf("UNIMPL: CLOSE\n");
		exit(1);
}

/* CHKIN */
static inline void
CHKIN() {
		printf("UNIMPL: CHKIN\n");
		exit(1);
}

/* CHKOUT */
static inline void
CHKOUT() {
		printf("UNIMPL: CHKOUT\n");
		exit(1);
}

/* CLRCHN */
static inline void
CLRCHN() {
#ifdef DEBUG
		printf("WARNING: UNIMPL: CLRCHN\n");
#endif
}

static const char run[] = { 'R', 'U', 'N', 13 };

static int fakerun = 0;
static int fakerun_index = 0;

/* CHRIN */
static inline void
CHRIN() {
	if ((!interactive) && (readycount==2)) {
		exit(0);
	}
	if (!f) {
		A = getchar(); /* stdin */
	} else {
		if (fakerun) {
			A = run[fakerun_index++];
			if (fakerun_index==sizeof(run))
				f = 0; /* switch to stdin */
		} else {
			A = fgetc(f);
			if ((A==255)&&(readycount==1)) {
				fakerun = 1;
				fakerun_index = 0;
				A = run[fakerun_index++];
			}
		}
	}
	if (A==255)
		exit(0);
	if (A=='\n') A = '\r';
	C = 0;
}

/* CHROUT */
static inline void
CHROUT() {
#if 0
int a = *(unsigned short*)(&RAM[0x0100+S+1]) + 1;
int b = *(unsigned short*)(&RAM[0x0100+S+3]) + 1;
int c = *(unsigned short*)(&RAM[0x0100+S+5]) + 1;
int d = *(unsigned short*)(&RAM[0x0100+S+7]) + 1;
printf("CHROUT: %d @ %x,%x,%x,%x\n", A, a, b, c, d);
#endif
	if (!interactive) {
		if (stack4(0xe10f,0xab4a,0xab30,0xe430)) {
			/* COMMODORE 64 BASIC V2 */
			C = 0;
			return;
		}
		if (stack4(0xe10f,0xab4a,0xab30,0xe43d)) {
			/* 38911 */
			C = 0;
			return;
		}
		if (stack4(0xe10f,0xab4a,0xab30,0xe444)) {
			/* BASIC BYTES FREE */
			C = 0;
			return;
		}
	}
	if (stack4(0xe10f,0xab4a,0xab30,0xa47b)) {
		/* READY */
		if (A=='R') readycount++;
		if (!interactive) {
			C = 0;
			return;
		}
	}
	if (stack4(0xe10f,0xab4a,0xaadc,0xa486)) {
		/*
		 * CR after each entered numbered program line:
		 * The CBM screen editor returns CR when the user
		 * hits return, but does not print the character,
		 * therefore CBMBASIC does. On UNIX, the terminal
		 * prints all input characters, so we have to avoid
		 * printing it again
		 */
		C = 0;
		return;
	}
	
#if 0
	printf("CHROUT: %c (%d)\n", A, A);
#else
	switch (A) {
		case 10:
			kernal_quote = 0;
			break;
		case 13:
			kernal_quote = 0;
			putchar(13);
			putchar(10);
			break;
		case 147: /* clear screen */
#ifndef NO_CLRHOME
			if (!kernal_quote)
				printf("%c[2J%c[;H", 27, 27);
#endif
			break;
		case '"':
			kernal_quote = 1-kernal_quote;
			// fallthrough
		default:
			putchar(A);
	}
#endif
	fflush(stdout);
	C = 0;
}

/* LOAD */
static inline void
LOAD() {
		FILE *f;
		unsigned char savedbyte;
		unsigned short start;
		unsigned short end;

		if (A) {
			printf("UNIMPL: VERIFY\n");
			exit(1);
		}
		if (!kernal_filename_len) {
			C = 1;
			A = kernal_status = KERN_ERR_MISSING_FILE_NAME;
			return;
		}

		if (RAM[kernal_filename]=='$') {
			DIR *dirp;
			struct dirent *dp;
			struct stat st;
			unsigned short old_memp, memp = 0x0801; // TODO hack!
			int i;
			int file_size;

			old_memp = memp;
			memp += 2;
			RAM[memp++] = 0;
			RAM[memp++] = 0;
			RAM[memp++] = 0x12; /* REVERS ON */
			RAM[memp++] = '"';
			for (i=0; i<16; i++)
				RAM[memp+i] = ' ';
			getcwd((char*)&RAM[memp], 256);
			memp += strlen((char*)&RAM[memp]); /* only 16 on COMMODORE DOS */
			RAM[memp++] = '"';
			RAM[memp++] = ' ';
			RAM[memp++] = '0';
			RAM[memp++] = '0';
			RAM[memp++] = ' ';
			RAM[memp++] = '2';
			RAM[memp++] = 'A';
			RAM[memp++] = 0;

			RAM[old_memp] = (memp) & 0xFF;
			RAM[old_memp+1] = (memp) >> 8;
			
			dirp = opendir(".");
			while ((dp = readdir(dirp))) {
				size_t namlen = strlen(dp->d_name);
				stat(dp->d_name, &st);
				file_size = (st.st_size + 253)/254;
				if (file_size>0xFFFF)
					file_size = 0xFFFF;
				old_memp = memp;
				memp += 2;
				RAM[memp++] = file_size & 0xFF;
				RAM[memp++] = file_size >> 8;
				if (file_size<1000) {
					RAM[memp++] = ' ';
					if (file_size<100) {
						RAM[memp++] = ' ';
						if (file_size<10) {
							RAM[memp++] = ' ';
						}
					}
				}
				RAM[memp++] = '"';
				if (namlen>16)
					namlen=16; /* TODO hack */
				memcpy(&RAM[memp], dp->d_name, namlen);
				memp += namlen;
				RAM[memp++] = '"';
				for (i=namlen; i<16; i++)
					RAM[memp++] = ' ';
				RAM[memp++] = ' ';
				RAM[memp++] = 'P';
				RAM[memp++] = 'R';
				RAM[memp++] = 'G';
				RAM[memp++] = ' ';
				RAM[memp++] = ' ';
				RAM[memp++] = 0;

				RAM[old_memp] = (memp) & 0xFF;
				RAM[old_memp+1] = (memp) >> 8;
			}
			RAM[memp] = 0;
			RAM[memp+1] = 0;
			(void)closedir(dirp);
			end = memp + 2;

/*
for (i=0; i<255; i++) {
	if (!(i&15))
		printf("\n %04X  ", 0x0800+i);
	printf("%02X ", RAM[0x0800+i]);
}
*/
			goto load_noerr;
		}

		savedbyte = RAM[kernal_filename+kernal_filename_len]; /* TODO possible overflow */
		RAM[kernal_filename+kernal_filename_len] = 0;
		f = fopen((char*)&RAM[kernal_filename], "rb");
		if (!f) {
			C = 1;
			A = kernal_status = KERN_ERR_FILE_NOT_FOUND;
			return;
		}
		start = ((unsigned char)fgetc(f)) | ((unsigned char)fgetc(f))<<8;
		if (!kernal_sec)
			start = X | Y<<8;
		end = start + fread(&RAM[start], 1, 65536-start, f); /* TODO may overwrite ROM */
		printf("LOADING FROM $%04X to $%04X\n", start, end);
		fclose(f);
load_noerr:
		X = end & 0xFF;
		Y = end >> 8;
		C = 0;
		A = kernal_status = KERN_ERR_NONE;
		return;
}

/* SAVE */
static inline void
SAVE() {
		FILE *f;
		unsigned char savedbyte;
		unsigned short start;
		unsigned short end;

		start = RAM[A] | RAM[A+1]<<8;
		end = X | Y << 8;
		if (end<start) {
			C = 1;
			A = kernal_status = KERN_ERR_NONE;
			return;
		}
		if (!kernal_filename_len) {
			C = 1;
			A = kernal_status = KERN_ERR_MISSING_FILE_NAME;
			return;
		}
		savedbyte = RAM[kernal_filename+kernal_filename_len]; /* TODO possible overflow */
		RAM[kernal_filename+kernal_filename_len] = 0;
		f = fopen((char*)&RAM[kernal_filename], "wb"); /* overwrite - these are not the COMMODORE DOS semantics! */
		if (!f) {
			C = 1;
			A = kernal_status = KERN_ERR_FILE_NOT_FOUND;
			return;
		}
		fputc(start & 0xFF, f);
		fputc(start >> 8, f);
		fwrite(&RAM[start], end-start, 1, f);
		fclose(f);
		C = 0;
		A = kernal_status = KERN_ERR_NONE;
}

/* SETTIM */
static inline void
SETTIM() {
		printf("UNIMPL: SETTIM\n");
		exit(1);
}

/* RDTIM */
static inline void
RDTIM() {
		printf("UNIMPL: RDTIM\n");
		exit(1);
}

/* STOP */
static inline void
STOP() {
		SETZ(0); /* TODO we don't support the STOP key */
}

/* GETIN */
static inline void
GETIN() {
	CHRIN();
}

/* CLALL */
static inline void
CLALL() {
}

/* PLOT */
static inline void
PLOT() {
		/*
		 * TODO we always return 0/0 as the cursor position
		 * setting the cursor is unused by CBMBASIC
		 */
		X = 0;
		Y = 0;
}


/* IOBASE */
static inline void
IOBASE() {
		printf("UNIMPL: IOBASE\n");
		exit(1);
}

int
kernal_dispatch(
	unsigned char *ram,
	unsigned short *pc,
	unsigned char *a,
	unsigned char *x,
	unsigned char *y,
	unsigned char *s,
	unsigned char *p)
{
	RAM = ram;
	PC = *pc;
	A = *a;
	X = *x;
	Y = *y;
	S = *s;
	N = (*p >> 7) & 1;
	V = (*p >> 6) & 1;
	D = (*p >> 3) & 1;
	I = (*p >> 2) & 1;
	Z = (*p >> 1) & 1;
	C = (*p >> 0) & 1;
	switch(PC) {
		case 0x0073:	CHRGET();	break;
		case 0x0079:	CHRGOT();	break;
		case 0xFF90:	SETMSG();	break;
		case 0xFF99:	MEMTOP();	break;
		case 0xFF9C:	MEMBOT();	break;
		case 0xFFB7:	READST();	break;
		case 0xFFBA:	SETLFS();	break;
		case 0xFFBD:	SETNAM();	break;
		case 0xFFC0:	OPEN();		break;
		case 0xFFC3:	CLOSE();	break;
		case 0xFFC6:	CHKIN();	break;
		case 0xFFC9:	CHKOUT();	break;
		case 0xFFCC:	CLRCHN();	break;
		case 0xFFCF:	CHRIN();	break;
		case 0xFFD2:	CHROUT();	break;
		case 0xFFD5:	LOAD();		break;
		case 0xFFD8:	SAVE();		break;
		case 0xFFDB:	SETTIM();	break;
		case 0xFFDE:	RDTIM();	break;
		case 0xFFE1:	STOP();		break;
		case 0xFFE4:	GETIN();	break;
		case 0xFFE7:	CLALL();	break;
		case 0xFFF0:	PLOT();		break;
		case 0xFFF3:	IOBASE();	break;
		default:
			return 0;
	}
	*pc = PC;
	*a = A;
	*x = X;
	*y = Y;
	*s = S;
	*p = ((N & 1)<<7) |
		 ((V & 1)<<6) |
		 ((D & 1)<<3) |
		 ((I & 1)<<2) |
		 ((Z & 1)<<1) |
		 ((C & 1)<<0);
	return 1;
}

