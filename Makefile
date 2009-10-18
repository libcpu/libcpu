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
	arch/mips/arch.o

OBJS_6502=test/6502/test.o
OBJS_MIPS=test/mips/test.o

OBJS=$(OBJS_LIBCPU) $(OBJS_6502) $(OBJS_MIPS)

EXECUTABLE_6502=test_6502
EXECUTABLE_MIPS=test_mips

EXECUTABLES=$(EXECUTABLE_6502) $(EXECUTABLE_MIPS)

all:
	echo $(CFLAGS)
	$(MAKE) -C arch/6502
	$(MAKE) -C arch/mips
	$(MAKE) -C libcpu
	$(MAKE) -C test/mips
	$(MAKE) -C test/6502
	$(CXX) -o $(EXECUTABLE_6502) $(OBJS_6502) $(OBJS_LIBCPU) `llvm-config --ldflags --libs`
	$(CXX) -o $(EXECUTABLE_MIPS) $(OBJS_MIPS) $(OBJS_LIBCPU) `llvm-config --ldflags --libs`

clean:
	$(MAKE) -C arch/6502 clean
	$(MAKE) -C arch/mips clean
	$(MAKE) -C libcpu clean
	$(MAKE) -C test/mips clean
	$(MAKE) -C test/6502 clean
	rm -f $(OBJS) $(EXECUTABLES)
