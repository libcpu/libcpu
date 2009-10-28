OBJS=libcpu_main.o cpu_generic.o
BIGO=libcpu.o

all: libcpu

libcpu:  $(OBJS)
	$(LD) -r -o $(BIGO) $(OBJS)

clean:
	rm -f $(OBJS) $(BIGO)
