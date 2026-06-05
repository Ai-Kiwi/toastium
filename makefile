ARCH ?= risc-v
BOARD ?= qemu

CC = riscv64-unknown-elf-gcc
ifeq ($(BOARD), qemu)
    CC += -DBOARD_TARGET=0
else ifeq ($(BOARD), VF2L) #visionfive 2 lite
    CC += -DBOARD_TARGET=1
else
    $(error Unknown BOARD=$(BOARD))
endif
GIT_VERSION_HASH := $(shell git describe --always --dirty)
CC += -DGIT_VERSION_HASH=\"$(GIT_VERSION_HASH)\"
CCF = -nostdlib -nostartfiles -ffreestanding -march=rv64gc_zba_zbb -mabi=lp64d -mcmodel=medany -O2 -ffreestanding -fno-builtin -fno-stack-protector \
	-I src/ \
	-I src/include \
	-I src/arch/$(ARCH)
LD = riscv64-unknown-elf-ld
LDF = -T link.ld
AS = riscv64-unknown-elf-as
ASF = -march=rv64gc_zba_zbb -mabi=lp64d

OC = riscv64-unknown-elf-objcopy

QEMU = qemu-system-riscv64
QEMUF = -machine virt -bios default -m 512M -smp 1 -nographic #-drive file=disk.img,format=raw,if=virtio
#QEMUF = -machine virt -bios default -m 512M -smp 1 -serial stdio -display sdl -device ramfb



SRCDIRS := src
SRC := $(shell find $(SRCDIRS) -type f -name "*.c")
ASM := $(shell find $(SRCDIRS) -type f -name "*.s")
ASMWPC := $(shell find $(SRCDIRS) -type f -name "*.S")


BUILD    := build
OBJ := $(patsubst %.c, $(BUILD)/%.o, $(SRC))
OBJ += $(patsubst %.s, $(BUILD)/%.o, $(ASM))
OBJ += $(patsubst %.S, $(BUILD)/%.o, $(ASMWPC))

TARGET = kernel
BIN := $(BUILD)/$(TARGET).bin
ELF := $(BUILD)/$(TARGET).elf


all: $(BIN)
	du -h $(BIN)

link.ld: link.ld.S
	$(CC) -E -P -DLINKER_SCRIPT=1 -I src/include -x c $< -o $@

$(BUILD)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(ASF) $< -o $@

$(BUILD)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CCF) -c -I src/include $< -o $@

$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CCF) -c $< -o $@

$(ELF): $(OBJ) link.ld
	$(LD) $(LDF) $(OBJ) -o $@

$(BIN): $(ELF)
	$(OC) -O binary $^ $@

run: $(BIN)
#tell the panic to be recompiled, means that git hash version will get updated.
	touch src/kernel/safety/panic.c
	$(QEMU) $(QEMUF) -kernel $(BIN)

asm-debug: $(BIN)
	$(QEMU) $(QEMUF) -kernel $(BIN) -d in_asm,cpu

debug: $(BIN)
	echo "connect with riscv64-unknown-elf-gdb build/kernel.elf"
	echo "target remote :1234"
	$(QEMU) $(QEMUF) -kernel $(BIN) -S -s

clean:
	rm -rf $(BUILD)
	rm -f link.ld