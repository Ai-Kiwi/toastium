#include "include/types.h"
#include "include/def.h"
#include "kernel/memory/radix.h"
#include "kernel/memory/pager.h"
#include "drivers/uart/uart.h"
#include "include/endian.h"

#define KEP_RNSR 1 //no start radix
#define KEP_IMH 2 //bad start header
#define KEP_IV 3 //incorrect version
#define KEP_IABI 4 //incorrect ABI
#define KEP_NSET 5 //unsupport elf type
#define KEP_NSAM 6 //Invalid arch machine
#define KEP_IBF 6 //Invalid bit format. basically not 64bit.


#define OS_ABI 0x54

#define ELF_TYPE_UNKNOWN 0x00
#define ELF_TYPE_RELOCATABLE 0x01
#define ELF_TYPE_EXECUTABLE 0x02
#define ELF_TYPE_SHARED 0x03
#define ELF_TYPE_CORE 0x04

#define ELF_MACHINE_RISC_V 0xF3

//input args will later be changed as move to disk reading happens
//returns response code
u64 kernel_parse_elf(u64 *location_bytes, u64 size_bytes) {
    u64 page_cnt = (size_bytes / KERNEL_PAGE_SIZE) + 1;

    u64 leading_zeros = __builtin_clzl(page_cnt);
    u64 compressed_size = (63-leading_zeros);

    u64 needed_depth = (compressed_size / 4) + 1;

    //setup a way so that it can be read as one sequential set despite being loaded as 
    u64 table_root = kernel_radix_create_tree(4); //one nibble

    u64 ptr_location = 0x0;

    u64 chunk_num = ptr_location / KERNEL_PAGE_SIZE;
    u64 location_in_chunk = ptr_location % KERNEL_PAGE_SIZE;

    u64 *page_data = (u64 *)kernel_radix_get_child(table_root, chunk_num, needed_depth, 4);

    //load data into pages
    for (u64 page_num=0; page_num < size_bytes / KERNEL_PAGE_SIZE; page_num++) {
        u64 *chunk_data = (u64 *)kernel_pager_acquire(); 
        u64 chunk_ptr = (u64)kernel_radix_create_child(table_root, page_num, (u64)chunk_data, needed_depth, 4);
        for (u64 j=0; j < KERNEL_PAGE_SIZE/8; j++) {
            u64 data_location = (page_num * KERNEL_PAGE_SIZE) + (j * 8);

            if (data_location >= size_bytes) {
                break;
            }
            chunk_data[j] = location_bytes[data_location / 8];

        }
    }

    //will be large enough to store all data at start
    u8 *first_page = (u8 *)kernel_radix_get_child(table_root, 0, needed_depth, 4);
    if (!(u64)first_page) {
        uart_println_str("ELF_PARSER : First page no radix child");
        return KEP_RNSR;
    }

    u32 *magic_ptr = (u32 *)first_page;

    if (big_endian_u32_to_host(*magic_ptr) == 0x7F454c46) {
        uart_println_str("ELF_PARSER : Invalid magic header");
        return KEP_IMH;
    }

    bool8 elf_64bit = first_page[0x04] == 2;
    if (elf_64bit == FALSE){
        uart_println_str("ELF_PARSER : Incorrect arch bit format");
        return KEP_IBF;
    }
    bool8 elf_little_endian = first_page[0x05] == 1;
    u8 elf_version = first_page[0x06];
    if (elf_version != 1){
        uart_println_str("ELF_PARSER : Incorrect version");
        return KEP_IV;
    }
    u8 operating_system = first_page[0x07];
    if (operating_system != OS_ABI) {
        uart_print_str("ELF_PARSER : Invalid ABI (");
        uart_print_u8_hex(operating_system);
        uart_print_str(")");
        return KEP_IABI;
    }

    //0x08 further arch ABI details for my os ignored
    //0x09 - 0x0F padding not used

    u16 *elf_type_ptr = (u16 *)first_page[0x10];
    u16 elf_type = elf_little_endian == TRUE ?
    little_endian_u16_to_host(*elf_type_ptr) :
    big_endian_u16_to_host(*elf_type_ptr);

    if (elf_type != ELF_TYPE_EXECUTABLE) {
        uart_println_str("ELF_PARSER : Unsupported elf type");
        return KEP_NSET;
    }


    u16 *elf_machine_ptr = (u16 *)first_page[0x12];
    u16 elf_machine = elf_little_endian == TRUE ?
    little_endian_u16_to_host(*elf_machine_ptr) :
    big_endian_u16_to_host(*elf_machine_ptr);

    if (elf_machine != ELF_MACHINE_RISC_V) {
        uart_println_str("ELF_PARSER : Unsupported architecture machine");
        return KEP_NSAM;
    }

    //0x14-0x17 version reused again

    //8bytes on 64bit 4bytes on 32bit. For 32bit it needs changing
    //This means that the offset and location will change
    //this applies to many below so if 32bit support needs to be looped over

    u64 *entry_location_ptr = (u64 *)first_page[0x18];
    u64 entry_location = elf_little_endian == TRUE ?
    little_endian_u64_to_host(*entry_location_ptr) :
    big_endian_u64_to_host(*entry_location_ptr);

    u64 *start_program_header_table_ptr = (u64 *)first_page[0x20];
    u64 start_program_header_table = elf_little_endian == TRUE ?
    little_endian_u64_to_host(*start_program_header_table_ptr) :
    big_endian_u64_to_host(*start_program_header_table_ptr);

    u64 *start_section_header_table_ptr = (u64 *)first_page[0x28];
    u64 start_section_header_table = elf_little_endian == TRUE ?
    little_endian_u64_to_host(*start_section_header_table_ptr) :
    big_endian_u64_to_host(*start_section_header_table_ptr);

    //TODO: 0x30 contains flags on features of arch needed for it to run
    //currently this is just ignored. Planning to make it read and decline if not needed in future
    //in far future will activate software emulation likely after warning user.
    //it is safe to leave right now as risc-v will fire exception fire then kernel will kill

    //size of elf header at 0x34 can just ignore as we know size. Fixed size

    //size of program header table at 0x36 can just ignore again. Fixed size

    u16 *program_header_entry_cnt_ptr = (u16 *)first_page[0x12];
    u16 program_header_entry_cnt = elf_little_endian == TRUE ?
    little_endian_u16_to_host(*program_header_entry_cnt_ptr) :
    big_endian_u16_to_host(*program_header_entry_cnt_ptr);

    u16 *section_header_table_entry_cnt_ptr = (u16 *)first_page[0x12];
    u16 section_header_table_entry_cnt = elf_little_endian == TRUE ?
    little_endian_u16_to_host(*section_header_table_entry_cnt_ptr) :
    big_endian_u16_to_host(*section_header_table_entry_cnt_ptr);

    u16 *idx_section_header_table_entry_cnt_ptr = (u16 *)first_page[0x12];
    u16 idx_section_header_table_entry_cnt = elf_little_endian == TRUE ?
    little_endian_u16_to_host(*idx_section_header_table_entry_cnt_ptr) :
    big_endian_u16_to_host(*idx_section_header_table_entry_cnt_ptr);



    //program header

    //for each one will load in data from what is stored
    //get a array of header data then will iterate on that.
    //hard part is keeping in mind page locations for performance.

    for (s32 i = 0; i < program_header_entry_cnt; i++) {

    }



    //section header




    //loaded successfully return nothing
    return 0;
}