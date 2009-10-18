#include "CPUInterpreter.h"
#include <string>
using std::string;

int logf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	int result = vprintf(format, args);
	va_end(args);
	return result;
}

int main() {
	VM *vm = new VM(true, 512, "/Users/mist/tmp/MARIO64_US_.V64");
	CPUEmulator *cpuEmulator = new CPUInterpreter(vm);
	for (;;) {
		cpuEmulator->Run();
	}
}

//printf("%s:%d\n", __func__, __LINE__);
