CC = riscv64-unknown-elf-gcc
CCF = -nostdlib -nostartfiles -ffreestanding -march=rv64imac -mabi=lp64 -mcmodel=medany
LD = riscv64-unknown-elf-ld
LDF = -T src/link.ld
AS = riscv64-unknown-elf-as
ASF = -march=rv64imac -mabi=lp64

OC = riscv64-unknown-elf-objcopy

QEMU = qemu-system-riscv64
QEMUF = -machine virt -bios default -m 512M -smp 1 -nographic #-drive file=disk.img,format=raw,if=virtio

SRCDIRS := src
SRC := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
ASM := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.s))


BUILD    := build
OBJ := $(patsubst %.c, $(BUILD)/%.o, $(SRC))
OBJ += $(patsubst %.s, $(BUILD)/%.o, $(ASM))
TARGET = kernel
BIN := $(BUILD)/$(TARGET).bin
ELF := $(BUILD)/$(TARGET).elf


all: $(BIN)

$(BUILD)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(ASF) $< -o $@

$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CCF) -c $< -o $@

$(ELF): $(OBJ)
	$(LD) $(LDF) $^ -o $@

$(BIN): $(ELF)
	$(OC) -O binary $^ $@

run: $(BIN)
	$(QEMU) $(QEMUF) -kernel $(BIN)

clean:
	rm -rf $(BUILD)