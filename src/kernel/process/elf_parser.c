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

} elf_header;

u8 read_u8(void *ptr, elf_header *header) {
    u8 *value = (u8 *)ptr;
    return *value;
}

u16 read_u16(void *ptr, elf_header *header) {
    u16 *value = (u16 *)ptr;
    if (header->is_big_endian == TRUE) {
        return big_endian_u16_to_host(*value);
    } else {
        return little_endian_u16_to_host(*value);
    }
}

u32 read_u32(void *ptr, elf_header *header) {
    u32 *value = (u32 *)ptr;
    if (header->is_big_endian == TRUE) {
        return big_endian_u32_to_host(*value);
    } else {
        return little_endian_u32_to_host(*value);
    }
}

u64 parse_header(file_descriptor *file, elf_header *header, u64 offset) {
    header->is_big_endian = TRUE;

    file_descriptor_seek(file, offset);
    u8 read_data[0x40];
    u64 read = file_descriptor_read(file, read_data, 0x40);

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

    if (read_u8(&read_data[0x04], header) != 2) {
        uart_println_str("ELF_PARSER : Unsupported elf version");
        return KEP_IV;
    }
}

u64 parse_elf(file_descriptor *file) {

    u64 *entry_location_ptr = (u64 *)first_page[0x18];
    u64 entry_location = elf_little_endian == TRUE
                             ? little_endian_u64_to_host(*entry_location_ptr)
                             : big_endian_u64_to_host(*entry_location_ptr);

    u64 *start_program_header_table_ptr = (u64 *)first_page[0x20];
    u64 start_program_header_table =
        elf_little_endian == TRUE
            ? little_endian_u64_to_host(*start_program_header_table_ptr)
            : big_endian_u64_to_host(*start_program_header_table_ptr);

    u64 *start_section_header_table_ptr = (u64 *)first_page[0x28];
    u64 start_section_header_table =
        elf_little_endian == TRUE
            ? little_endian_u64_to_host(*start_section_header_table_ptr)
            : big_endian_u64_to_host(*start_section_header_table_ptr);

    // TODO: 0x30 contains flags on features of arch needed for it to run
    // currently this is just ignored. Planning to make it read and decline if
    // not needed in future in far future will activate software emulation
    // likely after warning user. it is safe to leave right now as risc-v will
    // fire exception fire then kernel will kill

    // size of elf header at 0x34 can just ignore as we know size. Fixed size

    // size of program header table at 0x36 can just ignore again. Fixed size

    u16 *program_header_entry_cnt_ptr = (u16 *)first_page[0x12];
    u16 program_header_entry_cnt =
        elf_little_endian == TRUE
            ? little_endian_u16_to_host(*program_header_entry_cnt_ptr)
            : big_endian_u16_to_host(*program_header_entry_cnt_ptr);

    u16 *section_header_table_entry_cnt_ptr = (u16 *)first_page[0x12];
    u16 section_header_table_entry_cnt =
        elf_little_endian == TRUE
            ? little_endian_u16_to_host(*section_header_table_entry_cnt_ptr)
            : big_endian_u16_to_host(*section_header_table_entry_cnt_ptr);

    u16 *idx_section_header_table_entry_cnt_ptr = (u16 *)first_page[0x12];
    u16 idx_section_header_table_entry_cnt =
        elf_little_endian == TRUE
            ? little_endian_u16_to_host(*idx_section_header_table_entry_cnt_ptr)
            : big_endian_u16_to_host(*idx_section_header_table_entry_cnt_ptr);

    // program header

    // for each one will load in data from what is stored
    // get a array of header data then will iterate on that.
    // hard part is keeping in mind page locations for performance.

    for (s32 i = 0; i < program_header_entry_cnt; i++) {
    }

    // section header

    // loaded successfully return nothing
    return KEP_SUCCESS;
}