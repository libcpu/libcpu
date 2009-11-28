PROJECT(libcpu)

ADD_LIBRARY(cpu SHARED
			cpu_generic.cpp
			disasm.cpp
			basicblock.cpp
			translate.cpp
			tag.cpp
			optimize.cpp
			fp.cpp
			idbg.cpp
			libcpu_main.cpp)

TARGET_LINK_LIBRARIES(cpu ${GUEST_ARCHITECTURES_ENABLED} readline)
TARGET_LINK_LLVM(cpu)
