#STC15F104W 8pin dil package, 6 I/O pins
SDCC ?= sdcc
STCCODESIZE ?= 4089
SDCCLOPTS ?= --code-size $(STCCODESIZE) --iram-size 128 --xram-size 0 --stack-auto  --model-small --out-fmt-ihx
#--xram-size <Value> Causes the linker to check if the external ram usage is within limits of the given value.
#--stack-loc 0x44
#--data-loc 0x20
SDCCREV ?= -DSTC15F104W
SYSCLK ?= 11057
CFLAGS ?=  

SRC = src/main.c  
OBJ=  build/main.rel 
	
all: STCprogrammer.hex

build/main.rel: src/main.c src/stc8.h makefile
	$(SDCC)  -c $< -o $@ $(SDCCCOPTS)

STCprogrammer.hex: $(OBJ)
	$(SDCC) $^ -o build/STCprogrammer.hex $(SDCCLOPTS) $(SDCCREV) $(CFLAGS)
	@ tail -n 5 build/STCprogrammer.mem | head -n 2
	@ tail -n 1 build/STCprogrammer.mem 
	@ mv build/STCprogrammer.hex STCprogrammer.hex

