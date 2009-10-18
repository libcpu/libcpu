OBJS=mips.o
BIGO=arch.o

all: arch-mips

arch-mips: $(OBJS)
	$(LD) -r -o $(BIGO) $(OBJS)

clean:
	rm -f $(OBJS) $(BIGO)
