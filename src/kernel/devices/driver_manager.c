#include "kernel/devices/driver_manager.h"
#include "drivers/uart/uart.h"
#include "kernel/devices/device_tree.h"
#include "kernel/memory/allocator.h"
#include "kernel/safety/panic.h"
#include "types.h"

driver_boot_stage_entry *root_entry;

void append_driver(driver_boot_stage_entry *new_entry) {
    new_entry->next_entry = NULL;

    if (root_entry == NULL) {
        root_entry = new_entry;
        return;
    }

    if (new_entry->stage <= root_entry->stage) {
        new_entry->next_entry = root_entry;
        root_entry = new_entry;
        return;
    }

    driver_boot_stage_entry *cur_entry = root_entry;
    u64 loop_num = 0;
    while (TRUE) {

        if (loop_num > 15) {
            PANIC("E", 0x0, 0x0, 0x0);
        }
        driver_boot_stage_entry *next_entry = cur_entry->next_entry;
        if (next_entry == NULL) {
            cur_entry->next_entry = new_entry;
            return;
        }

        if (new_entry->stage <= next_entry->stage) {
            new_entry->next_entry = next_entry;
            cur_entry->next_entry = new_entry;
            return;
        }
        loop_num++;
        cur_entry = next_entry;
    }
}

driver_boot_stage_entry *fetch_driver(driver_boot_stage cur_stage) {
    if (root_entry == NULL) {
        return NULL;
    }

    if (cur_stage < root_entry->stage) {
        return NULL;
    }

    driver_boot_stage_entry *return_entry = root_entry;
    root_entry = root_entry->next_entry;
    return return_entry;
}

void init_drivers() {
    root_entry = NULL;

    // loop over all possible devices and append

    // device_tree_print();
}