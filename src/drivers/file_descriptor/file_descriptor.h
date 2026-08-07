#ifndef PROCESS_H
#define PROCESS_H

#include "kernel/memory/hashmap.h"
#include "kernel/memory/list.h"
#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/parser.h"

typedef enum {
    FSFD_FILE,
    FSFD_FOLDER,
    FSFD_RAM_MAP,
    FSFD_DEVICE, //maps to a physical device, custom system. E.g would be a gpu firmware where it redirects data
    FSFD_SOCKET,
    FSFD_PIPE,
} file_descriptor_type;

typedef struct {
    file_descriptor_type FSFD_TYPE;
    u64 drive;
} FS_FILE;

typedef struct {
    file_descriptor_type FSFD_TYPE;
    u64 drive;
} FS_FOLDER;

typedef struct {
    file_descriptor_type FSFD_TYPE;

} FS_RAM_MAP;

typedef struct {
    file_descriptor_type FSFD_TYPE;

} FS_DEVICE;

typedef struct {
    file_descriptor_type FSFD_TYPE;

} FS_SOCKET;

typedef struct {
    file_descriptor_type FSFD_TYPE;

} FS_PIPE;









//mappings can either eb 

typedef struct {
    u64 process_upto;
    hashmap process_hashmap;
    list running_list;
} __attribute__((aligned(64))) process_handler_state;



#endif