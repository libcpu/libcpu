OBJS=m88k_arch.o m88k_disasm.o m88k_insn.o m88k_recompile.o
BIGO=arch.o

all: arch-m88k

arch-m88k: $(OBJS)
	$(LD) -r -o $(BIGO) $(OBJS)

clean:
	rm -f $(OBJS) $(BIGO)
