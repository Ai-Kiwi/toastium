CC = riscv64-unknown-elf-gcc

CCF = -nostdlib -march=rv64gc_zba_zbb -mabi=lp64d -mcmodel=medany -O2 -fno-builtin \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wframe-larger-than=1024 \
	-Wvla \
	-Wimplicit-fallthrough \
	-Wno-sign-compare \
	-static \
	-I./libs \


SRCDIRS := src
SRC := $(shell find $(SRCDIRS) -type f -name "*.c")


BUILD    := build
OBJ := $(patsubst %.c, $(BUILD)/%.o, $(SRC))

TARGET = init
ELF := $(BUILD)/$(TARGET).elf

LIBS = -L./libs \
	-lc

OC = riscv64-unknown-elf-objcopy


all: $(BIN)
	du -h $(BIN)

$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CCF) -c $< $(LIBS) -o $@ 


$(ELF): $(OBJ)
	$(CC) $(CCF) $^ $(LIBS) -o $@


clean:
	rm -rf $(BUILD)

build: $(ELF)
	du -h $(ELF)