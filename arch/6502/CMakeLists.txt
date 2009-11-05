PROJECT(6502)

ADD_LIBRARY(6502 STATIC
			disasm.cpp
			isa.cpp
			recompile.cpp
			tag.cpp)

