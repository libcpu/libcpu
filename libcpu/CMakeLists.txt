PROJECT(libcpu)

ADD_LIBRARY(cpu SHARED
			frontend.cpp
			disasm.cpp
			basicblock.cpp
			function.cpp
			translate.cpp
			translate_all.cpp
			translate_singlestep.cpp
			tag.cpp
			optimize.cpp
			fp.cpp
			idbg.cpp
			interface.cpp)

TARGET_LINK_LIBRARIES(cpu ${GUEST_ARCHITECTURES_ENABLED} readline)
TARGET_LINK_LLVM(cpu)
