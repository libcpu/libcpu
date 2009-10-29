PWD		:=$(shell pwd)
UNAME		:=$(shell uname)

CFLAGS_GLOBAL	=-I$(PWD) -I$(PWD)/libcpu -DARCH=$(ARCH) -DAPP=$(APP)
ifeq ($(UNAME), Darwin)
MAKE		= make
CC		= gcc
CFLAGS		= $(CFLAGS_GLOBAL) -Wall -Wdeclaration-after-statement -Werror
CXXFLAGS	=  $(CFLAGS_GLOBAL) -Wall -Werror `llvm-config --cxxflags`
endif
ifeq ($(UNAME), SunOS)
MAKE		= gmake
ifeq ($(CC),)
CC		= cc
endif
CFLAGS		= $(CFLAGS_GLOBAL)
endif

export CFLAGS
export CXXFLAGS
export CC

#
#
#

OBJS_LIBCPU=\
	libcpu/libcpu.o  \
	arch/6502/arch.o \
	arch/m68k/arch.o \
	arch/mips/arch.o

OBJS_6502=test/6502/test.o
OBJS_M68K=test/m68k/test.o
OBJS_NEXT68K=test/next68k/test.o
OBJS_MIPS=test/mips/test.o

OBJS=$(OBJS_LIBCPU) $(OBJS_6502) $(OBJS_M68K) $(OBJS_MIPS)

EXECUTABLE_6502=test_6502
EXECUTABLE_M68K=test_m68k
EXECUTABLE_NEXT68K=test_next68k
EXECUTABLE_MIPS=test_mips

EXECUTABLES=$(EXECUTABLE_6502) $(EXECUTABLE_M68K) $(EXECUTABLE_MIPS) $(EXECUTABLE_NEXT68K)

all:
	echo $(CFLAGS)
	$(MAKE) -C arch/6502
	$(MAKE) -C arch/m68k
	$(MAKE) -C arch/mips
	$(MAKE) -C libcpu
	$(MAKE) -C test/mips
	$(MAKE) -C test/6502
	$(MAKE) -C test/m68k
	$(MAKE) -C test/next68k
	$(CXX) -o $(EXECUTABLE_6502) $(OBJS_6502) $(OBJS_LIBCPU) `llvm-config --ldflags --libs`
	$(CXX) -o $(EXECUTABLE_M68K) $(OBJS_M68K) $(OBJS_LIBCPU) `llvm-config --ldflags --libs`
	$(CXX) -o $(EXECUTABLE_MIPS) $(OBJS_MIPS) $(OBJS_LIBCPU) `llvm-config --ldflags --libs`
	$(CXX) -o $(EXECUTABLE_NEXT68K) $(OBJS_NEXT68K) $(OBJS_LIBCPU) `llvm-config --ldflags --libs`

clean:
	$(MAKE) -C arch/6502 clean
	$(MAKE) -C arch/m68k clean
	$(MAKE) -C arch/mips clean
	$(MAKE) -C libcpu clean
	$(MAKE) -C test/mips clean
	$(MAKE) -C test/6502 clean
	$(MAKE) -C test/m68k clean
	$(MAKE) -C test/next68k clean
	rm -f $(OBJS) $(EXECUTABLES)
