#ifndef CPUEMULATOR_H_
#define CPUEMULATOR_H_

#include "../VM.h"

class CPUEmulator {
protected:
	VM* _vm;
	CPUEmulator(VM *vm);
public:
	virtual ~CPUEmulator();
	virtual void Run() = 0;
	inline VM* GetVM() {
		return _vm;
	}
	inline void IncreaseCounter(int increment) {
		_vm->Cop0Registers.Count += increment;
		_vm->NextVIInterrupt -= increment;
		_vm->AiCounter -= increment;
	}
};

#endif /* CPUEMULATOR_H_ */
