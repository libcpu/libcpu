NAME	:= i8086

BIN	:= $(NAME).bin
ELF	:= $(NAME).elf
OBJ	:= $(NAME).o

all: $(BIN)

$(BIN): $(ELF)
	objcopy -O binary $< $@

$(ELF): $(OBJ)
	ld -nostdlib -static $< -o $@

%.o: %.S
	gcc -nostdinc -c $< -o $@

clean:
	rm -f $(BIN) $(ELF) $(OBJ)
.PHONY: clean
