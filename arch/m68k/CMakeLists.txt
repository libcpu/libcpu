PROJECT(m68k)

ADD_LIBRARY(m68k STATIC
			m68k_disasm.cpp
			m68k_isa.cpp
			m68k_arch.cpp
			m68k_tag.cpp)

