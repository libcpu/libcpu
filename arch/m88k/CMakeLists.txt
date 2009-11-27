PROJECT(m88k)

ADD_LIBRARY(m88k STATIC
			m88k_arch.cpp
			m88k_disasm.cpp
			m88k_insn.cpp
			m88k_translate.cpp)

