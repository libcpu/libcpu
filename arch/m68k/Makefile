OBJS=m68k.o isa.o disasm.o tag.o
BIGO=arch.o

all: arch-m68k

arch-m68k: $(OBJS)
	$(LD) -r -o $(BIGO) $(OBJS)

clean:
	rm -f $(OBJS) $(BIGO)
