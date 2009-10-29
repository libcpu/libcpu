OBJS=main.o next_lib.o loader.o
BIGO=test.o
CXXFLAGS =-I../../libcpu -I../.. `llvm-config --cxxflags`

all: main

main:  $(OBJS)
	$(LD) -r -o $(BIGO) $(OBJS)

clean:
	rm -f $(OBJS) $(BIGO)
