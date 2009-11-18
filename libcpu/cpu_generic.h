#include "libcpu.h"

/* emitter functions */
Value *arch_get_reg(cpu_t *cpu, uint32_t index, uint32_t bits, BasicBlock *bb);
void arch_put_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits, bool sext, BasicBlock *bb);
Value *arch_load32_aligned(cpu_t *cpu, Value *a, BasicBlock *bb);
void arch_store32_aligned(cpu_t *cpu, Value *v, Value *a, BasicBlock *bb);
Value *arch_load8(cpu_t *cpu, Value *addr, BasicBlock *bb);
Value *arch_load16_aligned(cpu_t *cpu, Value *addr, BasicBlock *bb);
void arch_store8(cpu_t *cpu, Value *val, Value *addr, BasicBlock *bb);
void arch_store16(cpu_t *cpu, Value *val, Value *addr, BasicBlock *bb);
void arch_branch(bool flag_state, BasicBlock *target1, BasicBlock *target2, Value *flag, BasicBlock *bb);
void arch_jump(BasicBlock *bb, BasicBlock *bb_target);

Value *arch_encode_bit(Value *flags, Value *bit, int shift, int width, BasicBlock *bb);
void arch_decode_bit(Value *flags, Value *bit, int shift, int width, BasicBlock *bb);

Value *arch_bswap(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb);
Value *arch_ctlz(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb);
Value *arch_cttz(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb);

/* FPU */
Value *arch_cast_fp32(cpu_t *cpu, Value *v, BasicBlock *bb);
Value *arch_cast_fp64(cpu_t *cpu, Value *v, BasicBlock *bb);
Value *arch_cast_fp80(cpu_t *cpu, Value *v, BasicBlock *bb);
Value *arch_cast_fp128(cpu_t *cpu, Value *v, BasicBlock *bb);

Value *arch_load_fp_reg(cpu_t *cpu, uint32_t index, uint32_t bits, BasicBlock *bb);
void arch_store_fp_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits, BasicBlock *bb);

Value *arch_sqrt(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb);

/* host functions */
uint32_t RAM32BE(uint8_t *RAM, addr_t a);

/*
 * a collection of preprocessor macros
 * that make the LLVM interface nicer
 */

#define LOAD(a) new LoadInst(a, "", false, bb)

#define CONSTs(s,v) ConstantInt::get(getIntegerType(s), v)
#define CONST1(v) CONSTs(1,v)
#define CONST8(v) CONSTs(8,v)
#define CONST16(v) CONSTs(16,v)
#define CONST32(v) CONSTs(32,v)
#define CONST64(v) CONSTs(64,v)

#define CONST(v) CONSTs(cpu->reg_size,v)

#define TRUNC(s,v) new TruncInst(v, getIntegerType(s), "", bb)
#define TRUNC1(v) TRUNC(1,v)
#define TRUNC8(v) TRUNC(8,v)
#define TRUNC16(v) TRUNC(16,v)
#define TRUNC32(v) TRUNC(32,v)

#define ZEXT(s,v) new ZExtInst(v, getIntegerType(s), "", bb)
#define ZEXT8(v) ZEXT(8,v)
#define ZEXT16(v) ZEXT(16,v)
#define ZEXT32(v) ZEXT(32,v)
#define ZEXT64(v) ZEXT(64,v)

#define SEXT(s,v) new SExtInst(v, getIntegerType(s), "", bb)
#define SEXT8(v) SEXT(8,v)
#define SEXT16(v) SEXT(16,v)
#define SEXT32(v) SEXT(32,v)
#define SEXT64(v) SEXT(64,v)

#define ADD(a,b) BinaryOperator::Create(Instruction::Add, a, b, "", bb)
#define SUB(a,b) BinaryOperator::Create(Instruction::Sub, a, b, "", bb)
#define MUL(a,b) BinaryOperator::Create(Instruction::Mul, a, b, "", bb)
#define SDIV(a,b) BinaryOperator::Create(Instruction::SDiv, a, b, "", bb)
#define UDIV(a,b) BinaryOperator::Create(Instruction::UDiv, a, b, "", bb)
#define SREM(a,b) BinaryOperator::Create(Instruction::SRem, a, b, "", bb)
#define UREM(a,b) BinaryOperator::Create(Instruction::URem, a, b, "", bb)
#define AND(a,b) BinaryOperator::Create(Instruction::And, a, b, "", bb)
#define OR(a,b) BinaryOperator::Create(Instruction::Or, a, b, "", bb)
#define XOR(a,b) BinaryOperator::Create(Instruction::Xor, a, b, "", bb)
#define SHL(a,b) BinaryOperator::Create(Instruction::Shl, a, b, "", bb)
#define LSHR(a,b) BinaryOperator::Create(Instruction::LShr, a, b, "", bb)
#define ASHR(a,b) BinaryOperator::Create(Instruction::AShr, a, b, "", bb)
#define ICMP_EQ(a,b) new ICmpInst(*bb, ICmpInst::ICMP_EQ, a, b, "")
#define ICMP_NE(a,b) new ICmpInst(*bb, ICmpInst::ICMP_NE, a, b, "")
#define ICMP_ULT(a,b) new ICmpInst(*bb, ICmpInst::ICMP_ULT, a, b, "")
#define ICMP_UGT(a,b) new ICmpInst(*bb, ICmpInst::ICMP_UGT, a, b, "")
#define ICMP_SLT(a,b) new ICmpInst(*bb, ICmpInst::ICMP_SLT, a, b, "")
#define ICMP_SGT(a,b) new ICmpInst(*bb, ICmpInst::ICMP_SGT, a, b, "")
#define ICMP_SGE(a,b) new ICmpInst(*bb, ICmpInst::ICMP_SGE, a, b, "")
#define ICMP_SLE(a,b) new ICmpInst(*bb, ICmpInst::ICMP_SLE, a, b, "")

/* shortcuts */
#define COM(x) XOR(x, CONST(-1ULL))
#define NEGs(s, x) SUB(CONST##s(0), x)
#define NEG(x) SUB(CONST(0), x)
#define NOT(a) XOR(a,CONST1(1))

/* floating point */
#define FPCONSTs(s,v) ConstantFP::get(getFloatType(bits), v)
#define FPCONST32(v) CONSTs(32,v)
#define FPCONST64(v) CONSTs(64,v)
#define FPCONST80(v) CONSTs(80,v)
#define FPCONST128(v) CONSTs(128,v)

#define FPCONST(v) FPCONSTs(cpu->reg_size,v)

#define FPTRUNC(s,v) new FPTruncInst(v, getFloatType(s), "", bb)
#define FPEXT(s,v) new FPExtInst(v, getFloatType(s), "", bb)

#define FPADD(a,b) ADD(a, b)
#define FPSUB(a,b) SUB(a, b)
#define FPMUL(a,b) MUL(a, b)
#define FPDIV(a,b) BinaryOperator::Create(Instruction::FDiv, a, b, "", bb)
#define FPREM(a,b) BinaryOperator::Create(Instruction::FRem, a, b, "", bb)

/* condition */
#define SELECT(c,a,b) (SelectInst::Create(c, a, b, "", bb))

/* interface to the GPRs */
#define R(i) arch_get_reg(cpu, i, 0, bb)
#define R32(i) arch_get_reg(cpu, i, 32, bb)

#define LET(i,v) arch_put_reg(cpu, i, v, 0, false, bb)
#define LET32(i,v) arch_put_reg(cpu, i, v, 32, true, bb)
#define LET_ZEXT(i,v) arch_put_reg(cpu, i, v, 1, false, bb)

/* interface to the FPRs */
#define FR(i) arch_load_fp_reg(cpu, i, 0, bb)
#define FR32(i) arch_load_fp_reg(cpu, i, 32, bb)
#define FR64(i) arch_load_fp_reg(cpu, i, 64, bb)
#define FR80(i) arch_load_fp_reg(cpu, i, 80, bb)
#define FR128(i) arch_load_fp_reg(cpu, i, 128, bb)

#define LETFP(i,v) arch_store_fp_reg(cpu, i, v, 0, bb)

/* interface to memory */
#define LOAD8(i,v) arch_put_reg(cpu, i, arch_load8(cpu,v,bb), 8, false, bb)
#define LOAD8S(i,v) arch_put_reg(cpu, i, arch_load8(cpu,v,bb), 8, true, bb)
#define LOAD16(i,v) arch_put_reg(cpu, i, arch_load16_aligned(cpu,v,bb), 16, false, bb)
#define LOAD16S(i,v) arch_put_reg(cpu, i, arch_load16_aligned(cpu,v,bb), 16, true, bb)
#define LOAD32(i,v) arch_put_reg(cpu, i, arch_load32_aligned(cpu,v,bb), 32, true, bb)

#define STORE8(v,a) arch_store8(cpu,v, a, bb)
#define STORE16(v,a) arch_store16(cpu,v, a, bb)
#define STORE32(v,a) arch_store32_aligned(cpu,v, a, bb)

/* byte swap */
#define SWAP16(v) arch_bswap(cpu, 16, v, bb)
#define SWAP32(v) arch_bswap(cpu, 32, v, bb)
#define SWAP64(v) arch_bswap(cpu, 64, v, bb)

/* bit fiddling */
#define CTTZ(s,v) arch_cttz(cpu, s, v, bb)
#define CTTZ8(v)  CTTZ(8,v)
#define CTTZ16(v) CTTZ(16,v)
#define CTTZ32(v) CTTZ(32,v)
#define CTTZ64(v) CTTZ(64,v)

#define CTLZ(s,v) arch_ctlz(cpu, s, v, bb)
#define CTLZ8(v)  CTLZ(8,v)
#define CTLZ16(v) CTLZ(16,v)
#define CTLZ32(v) CTLZ(32,v)
#define CTLZ64(v) CTLZ(64,v)

#define FFS(s,v) CTTZ(s,v)
#define FFS8(v)  FFS(8,v)
#define FFS16(v) FFS(16,v)
#define FFS32(v) FFS(32,v)
#define FFS64(v) FFS(64,v)

#define FFC(s,v) CTTZ(s,COM(v))
#define FFC8(v)  FFC(8,v)
#define FFC16(v) FFC(16,v)
#define FFC32(v) FFC(32,v)
#define FFC64(v) FFC(64,v)

/* host */
#define RAM32(RAM,a) RAM32BE(RAM,a)

/* branch */
#define BRANCH(f,pc1,pc2,v) arch_branch(f,pc1,pc2,v,bb)

/* jump */
#define JUMP arch_jump(bb, bb_target)

/* bitcasts to int */
#define IBITCASTs(s, v)  new BitCastInst(v, getIntegerType(s), "", bb)
#define IBITCAST32(v) IBITCASTs(32, v)
#define IBITCAST64(v) IBITCASTs(64, v)

/* bitcasts between float
 * NOTE: these functions do bitcast for integer data types
 *       and truncation or extension for float data types,
 *       they're defined this way as a convenience for
 *       those architectures (like ARM, M88K) where the 
 *       GPRs serve also as floating point registers.
 */
#define FPBITCASTs(s, v) arch_cast_fp##s(cpu, v, bb)
#define FPBITCAST32(v) FPBITCASTs(32, v)
#define FPBITCAST64(v) FPBITCASTs(64, v)
#define FPBITCAST80(v) FPBITCASTs(80, v)
#define FPBITCAST128(v) FPBITCASTs(128, v)

/* float <-> int */
#define FPTOSI(s, v) new FPToSIInst(v, getIntegerType(s), "", bb)
#define SITOFP(s, v) new SIToFPInst(v, getFloatType(s), "", bb)
#define FPTOUI(s, v) new FPToUIInst(v, getIntegerType(s), "", bb)
#define UITOFP(s, v) new UIToFPInst(v, getFloatType(s), "", bb)

/* float intrsinics */
#define FPSQRT(v)    arch_sqrt(cpu, 64, v, bb)

