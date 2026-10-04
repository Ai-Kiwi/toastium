ARCH ?= risc-v
CC = riscv64-unknown-elf-gcc

test: CC += -DQUICK_TEST_MODE
full_test: CC += -DDEEP_TEST_MODE

GIT_VERSION_HASH := $(shell git describe --always --dirty)
CC += -DGIT_VERSION_HASH=\"$(GIT_VERSION_HASH)\"
CCF = -nostdlib -nostartfiles -ffreestanding -march=rv64gc_zba_zbb -mabi=lp64d -mcmodel=medany -O2 -fno-builtin -fno-stack-protector \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wframe-larger-than=1024 \
	-Wvla \
	-Wimplicit-fallthrough \
	-Wno-sign-compare \
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


#Later date will fix this system
#for now have to switch with clean before hand

#run: BUILDTAG = run
#test: BUILDTAG = test
#run-traps: BUILDTAG = run
#debug: BUILDTAG = run
#build: BUILDTAG = run
#



#LASTBUILDTAGFILE = $(BUILD)/.last_build_tag
#
#ifneq ("$(shell cat $(LASTBUILDTAGFILE) 2>/dev/null)", "$(BUILDTAG)")
#	$(shell echo "Build config changed")
#	$(shell rm -rf $(BUILD)/tests)
#	$(shell mkdir -p $(BUILD))
#	$(shell echo "$(BUILDTAG)" > $(LASTBUILDTAGFILE))
#endif

all: $(BIN)
	du -h $(BIN)

link.ld: link.ld.S
	@$(CC) -E -P -DLINKER_SCRIPT=1 -I src/include -x c $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	@$(AS) $(ASF) $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	@$(CC) $(CCF) -c -I src/include $< -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CCF) -c $< -o $@

$(ELF): $(OBJ) link.ld
	@$(LD) $(LDF) $(OBJ) -o $@

$(BIN): $(ELF)
	@$(OC) -O binary $^ $@

run: $(BIN)
#tell the panic to be recompiled, means that git hash version will get updated.
	touch src/kernel/safety/panic.c
	touch src/tests/quick/run_quick_tests.c
	touch src/tests/deep/run_deep_tests.c
	@$(QEMU) $(QEMUF) -kernel $(BIN)

test: $(BIN)
#tell the panic to be recompiled, means that git hash version will get updated.
	touch src/kernel/safety/panic.c
	touch src/tests/quick/run_quick_tests.c
	touch src/tests/deep/run_deep_tests.c
	@$(QEMU) $(QEMUF) -kernel $(BIN)

full_test: $(BIN)
#tell the panic to be recompiled, means that git hash version will get updated.
	touch src/kernel/safety/panic.c
	touch src/tests/quick/run_quick_tests.c
	touch src/tests/deep/run_deep_tests.c
	@$(QEMU) $(QEMUF) -kernel $(BIN)

run-traps: $(BIN)
#tell the panic to be recompiled, means that git hash version will get updated.
	touch src/kernel/safety/panic.c
	$(QEMU) $(QEMUF) -kernel $(BIN) -d int

asm-debug: $(BIN)
	$(QEMU) $(QEMUF) -kernel $(BIN) -d in_asm,cpu

debug: $(BIN)
	@echo "connect with riscv64-unknown-elf-gdb build/kernel.elf"
	@echo "target remote :1234"
	$(QEMU) $(QEMUF) -kernel $(BIN) -S -s

clean:
	rm -rf $(BUILD)
	rm -f link.ld

build: $(BIN)