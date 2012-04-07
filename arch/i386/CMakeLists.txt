PROJECT(i386)

ADD_LIBRARY(i386 STATIC
			i386_decode.cpp
			i386_disasm.cpp
			i386_tag.cpp
			i386_translate.cpp)
