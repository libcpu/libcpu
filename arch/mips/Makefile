OBJS=mips.o disasm.o recompile.o
BIGO=arch.o

all: arch-mips

arch-mips: $(OBJS)
	$(LD) -r -o $(BIGO) $(OBJS)

clean:
	rm -f $(OBJS) $(BIGO)
