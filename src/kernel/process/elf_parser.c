// this whole thing is bassicly going to need to be recoded when i actually go
// about using it pretty much. Tons of bugs and also going to use async system.

#include "drivers/uart/uart.h"
#include "include/def.h"
#include "include/endian.h"
#include "include/types.h"
#include "kernel/file_system/file_descriptor.h"
#include "kernel/memory/pager.h"
#include "kernel/memory/radix.h"

#define KEP_SUCCESS 0 // success
#define KEP_RNSR 1    // no start radix
#define KEP_IMH 2     // bad start header
#define KEP_IV 3      // incorrect version
#define KEP_IABI 4    // incorrect ABI
#define KEP_NSET 5    // unsupport elf type
#define KEP_NSAM 6    // Invalid arch machine
#define KEP_IBF 7     // Invalid bit format. basically not 64bit.
#define KEP_IOC 8     // Invalid os code
#define KEP_RF 9      // read failed

#define OS_CODE 0x54

#define ELF_TYPE_UNKNOWN 0x00
#define ELF_TYPE_RELOCATABLE 0x01
#define ELF_TYPE_EXECUTABLE 0x02
#define ELF_TYPE_SHARED 0x03
#define ELF_TYPE_CORE 0x04

#define ELF_MACHINE_RISC_V 0xF3

typedef struct {
    bool8 is_big_endian;
    u8 operating_system;
    u8 abi;
    u16 object_type;
    u16 arch_type;
    u64 entry;
    u64 start_program_header;
    u64 start_section_header;
    u64 cnt_program_header;
    u64 cnt_section_header;
    u64 idx_section_names;

    u32 arch_flags;

} elf_header;

static u8 read_u8(void *ptr, elf_header *header) {
    u8 *value = (u8 *)ptr;
    return *value;
}

static u16 read_u16(void *ptr, elf_header *header) {
    u16 *value = (u16 *)ptr;
    if (header->is_big_endian == TRUE) {
        return big_endian_u16_to_host(*value);
    } else {
        return little_endian_u16_to_host(*value);
    }
}

static u32 read_u32(void *ptr, elf_header *header) {
    u32 *value = (u32 *)ptr;
    if (header->is_big_endian == TRUE) {
        return big_endian_u32_to_host(*value);
    } else {
        return little_endian_u32_to_host(*value);
    }
}

static u64 read_u64(void *ptr, elf_header *header) {
    u64 *value = (u64 *)ptr;
    if (header->is_big_endian == TRUE) {
        return big_endian_u64_to_host(*value);
    } else {
        return little_endian_u64_to_host(*value);
    }
}

static u64 parse_header(file_descriptor *file, elf_header *header, u64 offset) {
    header->is_big_endian = TRUE;

    file_descriptor_seek(file, offset);
    u8 read_data[0x40];
    u64 read = file_descriptor_read(file, read_data, 0x40);
    if (read < 0x40) {
        return KEP_RF;
    }

    // magic header
    if (read_u32(&read_data[0x0], header) == 0x7F454c46) {
        uart_println_str("ELF_PARSER : Invalid magic header");
        return KEP_IMH;
    }

    // is 64bit
    if (read_u8(&read_data[0x04], header) != 2) {
        uart_println_str("ELF_PARSER : Incorrect arch bit format");
        return KEP_IBF;
    }

    header->is_big_endian = read_u8(&read_data[0x05], header) == 2;
    header->operating_system = read_u8(&read_data[0x07], header);
    header->abi = read_u8(&read_data[0x08], header);

    if (header->operating_system != OS_CODE) {
        uart_print_str("ELF_PARSER : Invalid OS code (");
        uart_print_u8_hex(header->operating_system);
        uart_print_str(")");
        return KEP_IOC;
    }

    header->object_type = read_u16(&read_data[0x10], header);
    header->arch_type = read_u16(&read_data[0x12], header);

    if (header->object_type != ELF_TYPE_EXECUTABLE) {
        uart_println_str("ELF_PARSER : Unsupported elf type");
        return KEP_NSET;
    }

    if (header->arch_type != ELF_MACHINE_RISC_V) {
        uart_println_str("ELF_PARSER : Unsupported architecture machine");
        return KEP_NSAM;
    }

    // from here on only 64bit versions as this OS only supports 64bit.
    // 32bit is just ignored and will fail if not 32bit file.

    if (read_u8(&read_data[0x14], header) != 1) {
        uart_println_str("ELF_PARSER : Unsupported elf version");
        return KEP_IV;
    }

    header->entry = read_u64(&read_data[0x18], header);
    header->start_program_header = read_u64(&read_data[0x20], header);
    header->start_section_header = read_u64(&read_data[0x28], header);

    header->arch_flags = read_u32(&read_data[0x30], header);
    // 0x34, size 2 bytes, e_ehsize. Size of this header
    // 0x36, size 2 bytes, e_phentsize. Size of program header table
    header->cnt_program_header = read_u16(&read_data[0x38], header);
    // 0x3A, size 2 bytes, e_shentsize. Size of section header table
    header->cnt_section_header = read_u16(&read_data[0x3C], header);
    header->idx_section_names = read_u16(&read_data[0x3E], header);

    return KEP_SUCCESS;
}

typedef struct {
    u32 type;
    u64 flags;
    u64 virt_addr;
    u64 file_loc;
    u64 size;
    u32 linked_section_idx;
    u32 extra_info;
    u64 alignment;
    u64 entrys_size;
} elf_section;

static u64 parse_section(file_descriptor *file, elf_section *header, u64 offset,
                         elf_header *mheader) {
    file_descriptor_seek(file, offset);
    u8 read_data[0x40];
    u64 read = file_descriptor_read(file, read_data, 0x40);
    if (read < 0x40) {
        return KEP_RF;
    }

    // 0x00, size 4 bytes, sh_name. Offset to the name location (in .shstrtab)

    header->type = read_u32(&read_data[0x04], mheader);
    header->flags = read_u64(&read_data[0x08], mheader);
    header->virt_addr = read_u64(&read_data[0x10], mheader);
    header->file_loc = read_u64(&read_data[0x18], mheader);
    header->size = read_u64(&read_data[0x20], mheader);
    header->linked_section_idx = read_u32(&read_data[0x28], mheader);
    header->extra_info = read_u32(&read_data[0x2C], mheader);
    header->alignment = read_u64(&read_data[0x30], mheader);
    header->entrys_size = read_u64(&read_data[0x38], mheader);

    return KEP_SUCCESS;
}

typedef struct {
    u32 type;
    u32 flags;
    u64 file_loc;
    u64 virt_mem_addr;
    u64 phys_mem_addr;
    u64 file_size;
    u64 mem_size;
    u64 alignment;
} elf_program;

static u64 parse_program(file_descriptor *file, elf_program *header, u64 offset,
                         elf_header *mheader) {
    file_descriptor_seek(file, offset);
    u8 read_data[0x38];
    u64 read = file_descriptor_read(file, read_data, 0x38);
    if (read < 0x38) {
        return KEP_RF;
    }

    header->type = read_u32(&read_data[0x0], mheader);
    header->flags = read_u32(&read_data[0x4], mheader);
    header->file_loc = read_u64(&read_data[0x08], mheader);
    header->virt_mem_addr = read_u64(&read_data[0x10], mheader);
    header->phys_mem_addr = read_u64(&read_data[0x18], mheader);
    header->file_size = read_u64(&read_data[0x20], mheader);
    header->mem_size = read_u64(&read_data[0x28], mheader);
    header->alignment = read_u64(&read_data[0x30], mheader);

    return KEP_SUCCESS;
}

u64 parse_elf(file_descriptor *file) {

    elf_header header;
    parse_header(file, &header, 0);

    for (u64 i = 0; i < header.cnt_program_header; i++) {
        u64 offset = header.start_program_header + (0x38 * i);
        elf_program program;
        parse_program(file, &program, offset, &header);
    }

    for (u64 i = 0; i < header.cnt_section_header; i++) {
        u64 offset = header.start_section_header + (0x40 * i);
        elf_section section;
        parse_section(file, &section, offset, &header);
    }

    // will from here somehow need to map data into load data for process.
    // Bassicly some form of store that will return info for ram virt address.
    // Probs will do as a sorted list for time being then later change to
    // faster approach.
    // Likely will do a permement file descriptor that is used for this.
    // Was planning to prepopulate page however will likely start with lazy
    // loading.

    return 1;
}