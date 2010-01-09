PROJECT(6502)

ADD_LIBRARY(6502 STATIC
			6502_arch.cpp
			6502_disasm.cpp
			6502_isa.cpp
			6502_translate.cpp
			6502_tag.cpp)
