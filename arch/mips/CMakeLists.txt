PROJECT(mips)

ADD_LIBRARY(mips STATIC
			mips_disasm.cpp
			mips_arch.cpp
			mips_translate.cpp)

