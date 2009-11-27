PROJECT(arm)

ADD_LIBRARY(arm STATIC
			arm_disasm.cpp
			arm_arch.cpp
			arm_translate.cpp)

