OBJS=disasm.o isa.o recompile.o tag.o
BIGO=arch.o

all: arch-6502

arch-6502: $(OBJS)
	$(LD) -r -o $(BIGO) $(OBJS)

clean:
	rm -f $(OBJS) $(BIGO)
