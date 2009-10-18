#ifndef CPUINTERPRETER_H_
#define CPUINTERPRETER_H_

#include "CPUEmulator.h"
#include "../stdincludes.h"
#include "Opcode.h"

class CPUInterpreter: public virtual CPUEmulator {
private:
	TRegister *_registers;
	void InvalidOpcode();
	void UnhandledOpcode(std::string desc);
public:
	CPUInterpreter(VM* vm);
	void Run();
	void CheckMIInterrupt(u32 DoValue, u32 MIValue);
	void ExecuteCommand(TOpcode op);
	void ExecuteDelay();
	void SaveLinkRegister(int index);
	void BranchCompareRegister(TOpcode op, bool equal, bool likely);
	void BranchCompareZero(TOpcode op, TComparison comparison, bool likely, bool Link);
	void JumpTo(u32 targetPC);
};

#endif /* CPUINTERPRETER_H_ */
