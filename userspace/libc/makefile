CC = riscv64-unknown-elf-gcc

CCF = -nostdlib -nostartfiles -ffreestanding -march=rv64gc_zba_zbb -mabi=lp64d -mcmodel=medany -O2 -fno-builtin -fno-stack-protector \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wframe-larger-than=1024 \
	-Wvla \
	-Wimplicit-fallthrough \
	-Wno-sign-compare \
	-static

SRCDIRS := src
SRC := $(shell find $(SRCDIRS) -type f -name "*.c")
ASM := $(shell find $(SRCDIRS) -type f -name "*.s")
ASMWPC := $(shell find $(SRCDIRS) -type f -name "*.S")


BUILD    := build
OBJ := $(patsubst %.c, $(BUILD)/%.o, $(SRC))
OBJ += $(patsubst %.s, $(BUILD)/%.o, $(ASM))
OBJ += $(patsubst %.S, $(BUILD)/%.o, $(ASMWPC))

TARGET = libc
STATIC := $(BUILD)/$(TARGET).a


all: $(BIN)
	du -h $(BIN)

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	@$(AS) $(ASF) $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	@$(CC) $(CCF) -c -I src/include $< -o $@

$(BUILD)/%.o: %.c
	echo "build file "
	@mkdir -p $(dir $@)
	$(CC) $(CCF) -c $< -o $@

#from my understanding limiting what is here limits whats in scope of it

$(STATIC): $(OBJ)
	echo "build archive"
	ar rcs $@  $^


clean:
	rm -rf $(BUILD)

build: $(STATIC)