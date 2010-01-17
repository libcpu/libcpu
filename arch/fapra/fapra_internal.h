#include "libcpu.h"

int arch_fapra_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
int arch_fapra_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
int arch_fapra_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);
Value *arch_fapra_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);

#define INSTR(a) RAM32LE(cpu->RAM, a)

enum {
  LDW = 0x10,
  STW = 0x11,
  LDB = 0x1C,
  STB = 0x1D,

  LDIH = 0x20,
  LDIL = 0x21,

  JMP = 0x30,
  BRA = 0x34,
  BZ = 0x35,
  BNZ = 0x36,
  NOP = 0x32,
  CALL = 0x33,
  BL = 0x37,
  RFE = 0x3F,

  ADDI = 0x0F,
  ADD = 0x00,
  SUB = 0x01,
  AND = 0x02,
  OR = 0x03,
  NOT = 0x05,
  SARI = 0x0B,
  SAL = 0x06,
  SAR = 0x07,
  MUL = 0x08,

  PERM = 0x09,
  RDC8 = 0x0A,
  TGE = 0x0C,
  TSE = 0x0D
};
