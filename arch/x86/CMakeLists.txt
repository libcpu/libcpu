PROJECT(x86)

ADD_LIBRARY(x86 STATIC
			8086_disasm.cpp
			8086_tag.cpp
			8086_translate.cpp)
