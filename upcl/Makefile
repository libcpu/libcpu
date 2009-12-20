RM=rm -f
CC=/Developer/usr/bin/llvm-gcc
CXX=/Developer/usr/bin/llvm-g++
CFLAGS=-W -Wall -Wextra -O0 -g -I. #-DYYDEBUG=1
CXXFLAGS=$(CFLAGS)
FLEX=flex
BISON=bison -d
LDFLAGS=-g
LIBS=

OBJS=do.o lexer.o parser.o parse.o dumper.o \
	 simple_const_expr.o sema_analyzer.o \
	 register_def.o sub_register_def.o \
	 expression.o unary_expression.o binary_expression.o \
	 register_expression.o assign_expression.o \
	 bit_slice_expression.o bit_combine_expression.o \
	 cast_expression.o signed_expression.o \
	 expression_dumper.o

all: upcc

clean:
	$(RM) $(OBJS) upcc
	$(RM) parser.cpp parser.hpp
	$(RM) lexer.cpp

upcc: $(OBJS)
	$(CXX) $(LDFLAGS) -o upcc $(OBJS) $(LIBS)

do.o: do.cpp ast.h parser.hpp

lexer.cpp: parser.hpp lexer.l ast.h
	$(FLEX) -o lexer.cpp lexer.l

parser.cpp parser.hpp: parser.y ast.h
	$(BISON) -o parser.cpp parser.y

