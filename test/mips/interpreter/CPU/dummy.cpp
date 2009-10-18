#include <stdio.h>
#include "../VM.h"
void VM::LoadState() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
u32 VM::InnerRead32(unsigned int) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
u32 VM::MapTLB(unsigned int, bool) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
u64 VM::ReadMem64(unsigned int) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
u16 VM::ReadMem16(unsigned int) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::SaveState() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::CalculateAiCounter() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
TRegister* VM::GetRegisters() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::TLBWI() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::WriteMem32(unsigned int, unsigned int) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::WriteMem8(unsigned int, unsigned char) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::TLBP() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
u8 VM::ReadMem8(unsigned int) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
int logf(char const*, ...) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::WriteMem64(unsigned int, unsigned int, unsigned int) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::TLBWR() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::WriteMem16(unsigned int, unsigned short) {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
void VM::TLBR() {
	printf("%s:%d\n", __func__, __LINE__); exit(1);
}
