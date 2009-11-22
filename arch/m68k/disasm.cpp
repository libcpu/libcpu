//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
#include "types.h"
#include "isa.h"
#include "libcpu.h"

enum {
	TASK_DIS,
	TASK_REC,
	TASK_LEN
};

enum {
	IS_REG,
	IS_MEM
};

int sizenum[] = { 0, 8, 32, 16 };

#define DIS if (task==TASK_DIS)
//#define DIS
#define REC if (task==TASK_REC)
#define LEN if (task==TASK_LEN)


#define ABS(x) (x<0?-x:x)

#define REG0 bits(opcode,0,2)
#define MOD0 bits(opcode,3,5)
#define MOD1 bits(opcode,6,8)
#define REG1 bits(opcode,9,11)

#define CONDITION bits(opcode,8,11)	/* Bcc/DBcc */
#define QDATA0 bits(opcode,0,7)		/* quick data */
#define QDATA9 bits(opcode,9,11)	/* quick data */
#define DIRECTION8 bits(opcode,8,8)	/* for ADD/SUB/CMP */
#define DIRECTION10 bits(opcode,10,10)

#define SIZE6 decodesize1(bits(opcode,6,6))		/* single bit size */
#define SIZE1213 bits(opcode,12,13)				/* two bit size */
#define SIZE67 decodesize3(bits(opcode,6,7))	/* three bit size (two bit effective) */
#define SIZEA68 decodesizea3(bits(opcode,6,8))	/* three bit, A registers */

inline int
decodesize1(int size) {
	return size == 0 ? SIZE_W : SIZE_L;
}

inline int
decodesizea3(int size) {
	return size & 4 ? SIZE_L : SIZE_W;
}

inline int
decodesize3(int size) {
	switch (size) {
		case 0:
			return SIZE_B;
		case 1:
			return SIZE_W;
		case 2:
			return SIZE_L;
		default:
			return SIZE_INVLD;
	}
	return 0;
}

/* returns number of bytes of an immediate argument */
inline int
sizetoimmbytes(int size) {
	switch (size) {
		case SIZE_B:
			return 2; /* (!) */
		case SIZE_W:
			return 2; /* (!) */
		case SIZE_L:
			return 4; /* (!) */
	}
	printf("sizetoimmbytes");
	exit(1);
}

static int
lengthmodreg(int mod, int reg, int size) {
//printf("2mod=%d,reg=%d\n", mod, reg);
	switch (mod) {
		case 0:				/* Dn */
		case 1:				/* An */
		case 2:				/* (An) */
		case 3:				/* (An)+ */
		case 4:				/* -(An) */
			return 0;
		case 5:				/* d16(An) */
			return 2;
		case 6:				/* (bd,An,Xn) */
			return 2;
		case 7:
			if (reg == 0)	/* (xxx).w */
				return 2;
			if (reg == 1)	/* (xxx).l */
				return 4;
			if (reg == 2)	/* (d16,pc) */
				return 2;
			if (reg == 4)	/* #data */
				return sizetoimmbytes(size);
	}
	return 0;
}

static int
lengthdisp(cpu_t *cpu, addr_t pc, uint16_t opcode) {
	int32_t disp = (int8_t)bits(opcode,0,7);
	switch (disp) {
		case 0:
			return 4;
		case -1:
			return 6;
	}
	return 2;
}

static uint32_t
decodeimm(cpu_t *cpu, addr_t pc, int size) {
//printf("imm!\n");
	switch (size) {
		case SIZE_B:
//printf("imm: %x\n", RAM16(pc+2) & 0xFF);
			return RAM16(pc+2) & 0xFF;
		case SIZE_W:
			return RAM16(pc+2);
		case SIZE_L:
			return RAM32(pc+2);
	}
	printf("decodeimm  pc=%llx", pc);
	exit(1);
	return 0;
}

enum {
	FLAG_POSTINC
};

static void
decodemodreg(int task, cpu_t *cpu, addr_t pc, int size, int mod, int reg, char *op1, unsigned int maxop, const char **attr, int *rm) {
//printf("1mod=%d,reg=%d\n", mod, reg);
	int flag;
	*attr = "";
	switch (mod) {
		case 0:
			DIS snprintf(op1, maxop, "d%d", reg);
			REC snprintf(op1, maxop, "d,%d", reg);
			REC *rm=IS_REG;
			return;
		case 1:
			DIS snprintf(op1, maxop, "a%d", reg);
			REC snprintf(op1, maxop, "a,%d", reg);
			REC *rm=IS_REG;
			return;
		case 2:
			DIS snprintf(op1, maxop, "(a%d)", reg);
			REC snprintf(op1, maxop, "RAM%d,READ_a(%d)", sizenum[size], reg);
			REC *rm=IS_MEM;
			return;
		case 3:
			DIS snprintf(op1, maxop, "(a%d)+", reg);
			REC snprintf(op1, maxop, "RAM%d,READ_a(%d)", sizenum[size], reg);
			REC *rm=IS_MEM;
			flag = FLAG_POSTINC;
			return;
		case 4:
			DIS snprintf(op1, maxop, "-(a%d)", reg);
			REC snprintf(op1, maxop, "RAM32,%d+READ_a(%d)", (int16_t)RAM16(pc+2), reg);
			REC *rm=IS_MEM;
			REC *attr = "abs";
			return;
		case 5:
			DIS snprintf(op1, maxop, "%s%x(a%d)", RAM16(pc+2)<0?"-0x":"0x", ABS(RAM16(pc+2)), reg);
			REC snprintf(op1, maxop, "RAM%d,READ_a(%d)+0x%x", sizenum[size], reg, RAM16(pc+2));
			REC *rm=IS_MEM;
			return;
		case 6:
			/* TODO incomplete */
			DIS snprintf(op1, maxop, "%x(a%d,%c%d.?)", RAM16(pc+2)&0xFF, reg, bits(RAM16(pc+2),15,15)?'a':'d', bits(RAM16(pc+2),12,14));
			REC snprintf(op1, maxop, "RAM%d,0x%x+READ_a(%d)+READ_%c(%d)", sizenum[size], RAM16(pc+2)&0xFF, reg, bits(RAM16(pc+2),15,15)?'a':'d', bits(RAM16(pc+2),12,14));
			REC *rm=IS_MEM;
			return;
		case 7:
			if (reg == 0) {
				DIS snprintf(op1, maxop, "(0x%04x).w", RAM16(pc+2));
				REC snprintf(op1, maxop, "RAM32,0x%04x", RAM16(pc+2));
				REC *rm=IS_MEM;
				REC *attr = "abs";
				return;
			}
			if (reg == 1) {
				DIS snprintf(op1, maxop, "(0x%08x).l", RAM32(pc+2));
				REC snprintf(op1, maxop, "RAM32,0x%08x", RAM32(pc+2));
				REC *rm=IS_MEM;
				return;
			}
			if (reg == 2) {
				DIS snprintf(op1, maxop, "0x%08llx", RAM16(pc+2)+pc+2);
				REC snprintf(op1, maxop, "RAM%d,0x%llx", sizenum[size], RAM16(pc+2)+pc+2);
				REC *rm=IS_MEM;
				return;
			}
			if (reg == 4) {
				DIS snprintf(op1, maxop, "#0x%x", decodeimm(cpu, pc, size));
				REC snprintf(op1, maxop, "IMM,0x%02x", decodeimm(cpu, pc, size));
				REC *rm=IS_MEM;
				REC *attr = "imm";
				return;
			}
			/* else fallthrough */
		default:
			DIS snprintf(op1, maxop, "%d/%d", mod, reg);
			return;
	}
}

int32_t
arch_disasm_get_disp(cpu_t *cpu, addr_t pc, uint16_t opcode) {
	int32_t disp = (int8_t)bits(opcode,0,7);
	switch (disp) {
		case 0:
			disp = RAM16(pc+2);
			break;
		case -1:
			disp = RAM32(pc+2);
			break;
	}
	return disp;
}

void
decodereglist(uint16_t mask, char *op1, unsigned int maxop) {
	int i;
	*op1 = 0;

	for (i=0; i<8; i++) {
		if (mask & (1<<(15-i))) {
			snprintf(op1+strlen(op1), maxop-strlen(op1), "d%d ",i);
		}
	}
	for (i=0; i<8; i++) {
		if (mask & (1<<(7-i))) {
			snprintf(op1+strlen(op1), maxop-strlen(op1), "a%d ",i);
		}
	}
}

static inline int
disreclen(int task, cpu_t *cpu, addr_t pc, char *line, unsigned int max_line, addr_t start, int optimized_dispatch) {
	uint16_t opcode = RAM16(pc);
	int dr;
	int32_t disp;
	char op1[40], op2[40];
	const char *mnemo = "";
	int len = 2; /* default */
	const char *attr;
	int rm, rm2;

	if (line) *line = 0;

#if 0 /* debug */
	printf("pc = %x (task=%d)\n", pc, task);
	char debug_line[100];
	if (!line) {
		line = debug_line;
		max_line = sizeof(debug_line);
	}
#endif

	switch (bits(opcode,12,15)) {
		case 0:
			switch (bits(opcode,8,11)) {
				case 0: mnemo = "or"; break;
				case 2: mnemo = "and"; break;
				case 4: mnemo = "sub"; break;
				case 6: mnemo = "add"; break;
				case 10: mnemo = "eor"; break;
				case 12: mnemo = "cmp"; break;
				default: mnemo = "???"; break;
			}
			decodemodreg(task, cpu, pc, SIZE67, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
			DIS snprintf(line, max_line, "%si.%c #0x%x, %s", mnemo, sizechar[SIZE67], decodeimm(cpu, pc+lengthmodreg(MOD0, REG0, SIZE67), SIZE67), op1);
			len = 2+lengthmodreg(MOD0, REG0, SIZE67)+sizetoimmbytes(SIZE67);
			break;
		case 1:
		case 2:
		case 3:	/* MOV */
			decodemodreg(task, cpu, pc+lengthmodreg(MOD0,REG0,SIZE1213), SIZE1213, MOD1, REG1, op2, sizeof(op2), &attr, &rm2);
			decodemodreg(task, cpu, pc, SIZE1213, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
			DIS snprintf(line, max_line, "move.%c %s, %s", sizechar[SIZE1213], op1, op2);
			REC {
//				if (rm2==IS_REG)
				snprintf(line, max_line, "move%d (READ_%s, WRITE_%s);", sizenum[SIZE1213], op1, op2);
			}
			len = 2+lengthmodreg(MOD0, REG0, SIZE1213)+lengthmodreg(MOD1, REG1, SIZE1213);
			break;
		case 4:
			if (opcode==0x4e75) {			/* RTS */
				DIS snprintf(line, max_line, "rts");
				REC snprintf(line, max_line, "ret();");
				len = 2;
				break;
			}
			if (opcode==0x4e71) {			/* NOP */
				DIS snprintf(line, max_line, "nop");
				REC snprintf(line, max_line, "nop;");
				len = 2;
				break;
			}
			if (MOD1==7) { /* LEA */
				decodemodreg(task, cpu, pc, SIZE_L, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
				DIS snprintf(line, max_line, "lea %s, a%d", op1, REG1);
				REC snprintf(line, max_line, "lea (EA_%s, WRITE_a,%d);", op1, REG1);
				len = 2+lengthmodreg(MOD0, REG0, SIZE_L);
				break;
			}
			switch (bits(opcode,8,11)) {
				case 2:	/* CLR */
					decodemodreg(task, cpu, pc, SIZE67, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
					DIS snprintf(line, max_line, "clr.%c %s", sizechar[SIZE67], op1);
					REC snprintf(line, max_line, "move%d (READ_IMM,0, WRITE_%s);", sizenum[SIZE67], op1);
					len = 2+lengthmodreg(MOD0, REG0, SIZE67);
					break;
				case 4:	/* NEG */
					decodemodreg(task, cpu, pc, SIZE67, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
					DIS snprintf(line, max_line, "neg.%c %s", sizechar[SIZE67], op1);
					REC snprintf(line, max_line, "neg%d (READ_IMM,0, WRITE_%s);", sizenum[SIZE67], op1);
					len = 2+lengthmodreg(MOD0, REG0, SIZE67);
					break;
				case 10:	/* TST */
					decodemodreg(task, cpu, pc, SIZE67, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
					DIS snprintf(line, max_line, "tst.%c %s", sizechar[SIZE67], op1);
					REC snprintf(line, max_line, "tst%d(READ_%s);", sizenum[SIZE67], op1);
					len = 2+lengthmodreg(MOD0, REG0, SIZE67);
					break;
				case 14:
					 if (bits(opcode,6,7) == 2) {	/* JSR */
						decodemodreg(task, cpu, pc, SIZE_L, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
						DIS snprintf(line, max_line, "jsr %s", op1);
						len = 2+lengthmodreg(MOD0, REG0, 0);
						REC snprintf(line, max_line, "call (READ_%s,0x%08llx);", op1, pc+len);
						break;
					} else if (bits(opcode,0,7) == 0x56) {	/* LINKW */ // 68030?
						decodemodreg(task, cpu, pc, SIZE_L, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
						DIS snprintf(line, max_line, "linkw %s, %04x", op1, decodeimm(cpu, pc, SIZE_W));
						len = 4+lengthmodreg(MOD0, REG0, 0);
						REC snprintf(line, max_line, "linkw (READ_%s,0x%08llx);", op1, pc+len);
						break;
					} else if (bits(opcode,0,7) == 0x5e) {	/* UNLK */ // 68030?
						decodemodreg(task, cpu, pc, SIZE_L, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
						DIS snprintf(line, max_line, "unlk %s", op1);
						len = 2+lengthmodreg(MOD0, REG0, 0);
						REC snprintf(line, max_line, "linkw (READ_%s,0x%08llx);", op1, pc+len);
						break;
					} else {
						// DIS snprintf(line, max_line, "??? opcode %04x (%x)", opcode, bits(opcode,12,15));
						DIS snprintf(line, max_line, "??? opcode %04x (%x)", opcode, bits(opcode,0,15));
					}
					break;
					
				case 8:
				case 12:
					if (bits(opcode,6,7) == 1 && DIRECTION10 == 0) {	/* PEA */
						decodemodreg(task, cpu, pc, SIZE_L, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
						DIS snprintf(line, max_line, "pea.%c %s", sizechar[SIZE_L], op1);
						REC snprintf(line, max_line, "pea%d(READ_%s);", sizenum[SIZE_L], op1);
						len = 2+lengthmodreg(MOD0, REG0, SIZE67);
						break;
					} else if (bits(opcode,7,7)==1) { /* MOVEM */
						dr = DIRECTION10;
						decodemodreg(task, cpu, pc, SIZE6, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
						decodereglist(RAM16(pc+2), op2, sizeof(op2));
						if (!dr) {
							DIS snprintf(line, max_line, "movem.%c %s, %s", sizechar[SIZE6], op2, op1);
						} else {
							DIS snprintf(line, max_line, "movem.%c %s, %s", sizechar[SIZE6], op1, op2);
						}
						REC snprintf(line, max_line, "/*TODO*/");
						len = 4+lengthmodreg(MOD0, REG0, SIZE6);
						break;
					} else
						DIS snprintf(line, max_line, "??? opcode %04x (%x)", opcode, bits(opcode,12,15));
						break;
				default:
					DIS snprintf(line, max_line, "??? opcode %04x (%x)", opcode, bits(opcode,12,15));
					break;
			}
			break;
		case 5:
			if (bits(opcode,6,7)==3) {
				if (MOD0==1) { /* DBcc */
					DIS snprintf(line, max_line, "db%s d%d, 0x%08llx", condstr_db[CONDITION], REG0, pc+2+RAM16(pc+2));
					REC snprintf(line, max_line, "db%s (d[%d], l%llX);", condstr_db[CONDITION], REG0, pc+2+RAM16(pc+2));
					len = 4;
				}
				else if (MOD0==7) { /* TRAPcc */
					DIS snprintf(line, max_line, "trap%s d%d, 0x%08llx", condstr_db[CONDITION], REG0, pc+2+RAM16(pc+2));
					REC snprintf(line, max_line, "trap%s (d[%d], l%llX);", condstr_db[CONDITION], REG0, pc+2+RAM16(pc+2));
					// TODO: decode OPMODE etc.! (p. 8.16 68kPM), optional word or long word
					len = 2;
				}
				else { /* Scc */
					DIS snprintf(line, max_line, "s%s d%d", condstr_db[CONDITION], REG0);
					REC snprintf(line, max_line, "s%s d[%d]);", condstr_db[CONDITION], REG0);
					len = 2;
				}
			} else {	/* ADDQ/SUBQ */
				decodemodreg(task, cpu, pc, SIZE67, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
				DIS snprintf(line, max_line, "%s.%c #0x%08x, %s", bits(opcode,8,8)? "subq":"addq", sizechar[SIZE67], QDATA9, op1);
				REC snprintf(line, max_line, "%s%d (0x%x, READ_%s, WRITE_%s);", bits(opcode,8,8)? "subq":"addq", sizenum[SIZE67], QDATA9, op1, op1);
				len = 2+lengthmodreg(MOD0, REG0, SIZE67);
			}
			break;
		case 6:	/* Bcc */
			disp = arch_disasm_get_disp(cpu, pc, opcode);
			DIS snprintf(line, max_line, "b%s 0x%08llx", condstr[CONDITION], pc+2+disp);
			DIS snprintf(line, max_line, "b%s (l%llX);", condstr[CONDITION], pc+2+disp);
			len = lengthdisp(cpu, pc, opcode);
			break;
		case 7:
			if (bits(opcode,8,8)==0) { /* MOVQ */
				DIS snprintf(line, max_line, "movq #0x%08x, d%d", QDATA0, REG1);
				REC snprintf(line, max_line, "move32 (READ_IMM,0x%x, WRITE_d,%d);", QDATA0, REG1);
				len = 2;
			} else
				DIS snprintf(line, max_line, "??? opcode %04x (%x)", opcode, bits(opcode,12,15));
			break;
		case 9:
		case 11:
		case 13:
			switch (bits(opcode,12,15)) {
				case 9: mnemo = "sub"; break;
				case 11: mnemo = "cmp"; break;
				case 13: mnemo = "add"; break;
				default: mnemo = "???"; break;
			}
			if (bits(opcode,6,7)==3) {	/* ADDA/SUBA/... */
					decodemodreg(task, cpu, pc, SIZEA68, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
					DIS snprintf(line, max_line, "%sa.%c %s, a%d", mnemo, sizechar[SIZEA68], op1, REG1);
					REC snprintf(line, max_line, "%s%d (READ_%s, READ_a,%d, WRITE_a,%d);", mnemo, sizenum[SIZEA68], op1, REG1, REG1);
					len = 2+lengthmodreg(MOD0, REG0, SIZEA68); // TODO: BUG?
					fprintf(stderr, ">>> %d\n", lengthmodreg(MOD0, REG0, SIZEA68));
					break;
			} else {					/* ADD/SUB/... */
				decodemodreg(task, cpu, pc, SIZE67, MOD0, REG0, op1, sizeof(op1), &attr, &rm);
				if (!DIRECTION8) {
					DIS snprintf(line, max_line, "%s%d (%s, d%d)", mnemo, sizenum[SIZE67], op1, REG1);
					REC snprintf(line, max_line, "%s%d (READ_%s, READ_d,%d, WRITE_d,%d);", mnemo, sizenum[SIZE67], op1, REG1, REG1);
				} else {
					DIS snprintf(line, max_line, "%s%d (d%d, %s)", mnemo, sizenum[SIZE67], REG1, op1);
					REC snprintf(line, max_line, "%s%d (READ_d%d, READ_%s, WRITE_%s);", mnemo, sizenum[SIZE67], REG1, op1, op1);
				}
				len = 2+lengthmodreg(MOD0, REG0, SIZE67);
				break;
			}
		case 14:
			switch (bits(opcode,3,4)) {
				case 0: mnemo = "as"; break;
				case 1: mnemo = "ls"; break;
				case 2: mnemo = "rox"; break;
				case 3: mnemo = "ro"; break;
			}
			if (bits(opcode,5,5)==0) {	/* count */
				int count = bits(opcode,9,11);
				if (!count) count=8;
				DIS snprintf(line, max_line, "%s%c.%c #%d, d%d", mnemo, bits(opcode,8,8)?'l':'r', sizechar[SIZE67], count, REG0);
			} else {	/* register */
				DIS snprintf(line, max_line, "%s%c.%c d%d, d%d", mnemo, bits(opcode,8,8)?'l':'r', sizechar[SIZE67], bits(opcode,9,11), REG0);
			}
			len = 2;
			break;
		default:
			DIS snprintf(line, max_line, "??? opcode %04x (%x)", opcode, bits(opcode,12,15));
	}
//DIS printf("dis = %s\n", line);
//printf("len = %x\n", len);
	return len;
}

/*
 * Write an ASCII disassembly of one instruction at "pc"
 * in "RAM" into "line" (max length "max_line"), return
 * number of bytes consumed.
 */
int
arch_m68k_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line) {
	return disreclen(TASK_DIS, cpu, pc, line, max_line, 0, 0);
}

int
arch_m68k_recompile_cond(cpu_t *, addr_t, BasicBlock*)
{
	printf("unimplemented!\n");
	exit(1);
}

int
arch_m68k_recompile_instr(cpu_t *, addr_t, BasicBlock*, BasicBlock*, BasicBlock *bb_target, BasicBlock *bb_cond, BasicBlock *bb_next)
{
	printf("unimplemented!\n");
	exit(1);
}

int
arch_m68k_instr_length(cpu_t *cpu, addr_t pc) {
	return disreclen(TASK_LEN, cpu, pc, 0, 0, 0, 0);
}


int
arch_get_ramsize() {
	return 256*1024; /* 256 KB */
}

