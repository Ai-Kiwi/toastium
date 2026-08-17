#ifndef DEVICE_DRIVER_MANAGER
#define DEVICE_DRIVER_MANAGER

// Order matters here. Something before will boot before stuff afterwards.
// Nothing can happen before allocator that is when first item happens
typedef enum {
    DBS_ALLOCATOR,
} driver_boot_stage;

typedef struct {
    driver_boot_stage stage;
    void *next_entry;
} driver_boot_stage_entry;

void init_drivers();
void append_driver(driver_boot_stage_entry *entry);

#endif