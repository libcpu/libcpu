PROJECT(m68k)

ADD_LIBRARY(m68k STATIC
			disasm.cpp
			isa.cpp
			m68k.cpp
			tag.cpp)

