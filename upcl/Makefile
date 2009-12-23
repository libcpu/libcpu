RM=rm -f
CC=/Developer/usr/bin/llvm-gcc
CXX=/Developer/usr/bin/llvm-g++
CFLAGS=-W -Wall -Wextra -O0 -g -I. #-DYYDEBUG=1
CXXFLAGS=$(CFLAGS)
FLEX=flex
BISON=bison -d
LDFLAGS=-g
LIBS=

OBJS=main.o \
		 strnatcmp.o \
		 ast/lexer.o \
		 ast/parser.o \
		 ast/parse.o \
		 ast/dumper.o \
		 sema/simple_expr_evaluator.o \
		 sema/register_dep_tracker.o \
		 sema/convert.o \
		 sema/expr_convert.o \
		 sema/register_file_builder.o \
		 c/sema_analyzer.o \
		 c/register_def.o \
		 c/sub_register_def.o \
		 c/expression.o \
		 c/cast_expression.o \
		 c/unary_expression.o \
		 c/binary_expression.o \
		 c/bit_slice_expression.o \
		 c/bit_combine_expression.o \
		 c/register_expression.o \
		 c/signed_expression.o \
		 c/assign_expression.o \
		 c/expression_dumper.o

all: upcc

clean:
	$(RM) $(OBJS) upcc
	$(RM) ast/parser.cpp ast/parser.hpp
	$(RM) ast/lexer.cpp

upcc: $(OBJS)
	$(CXX) $(LDFLAGS) -o upcc $(OBJS) $(LIBS)

main.o: main.cpp ast/ast.h ast/parser.hpp

ast/lexer.cpp: ast/parser.hpp ast/lexer.l ast/ast.h
	$(FLEX) -o ast/lexer.cpp ast/lexer.l

ast/parser.cpp ast/parser.hpp: ast/parser.y ast/ast.h
	$(BISON) -o ast/parser.cpp ast/parser.y

