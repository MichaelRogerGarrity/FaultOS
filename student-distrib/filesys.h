#include "multiboot.h"
#include "lib.h"

int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);

/* Variables used in the program: */
#define MAX_DATA_BLOCKS
#define MAX_FILENAME_LENGTH         32
#define RESERVED_BITS_DENTRY        6
#define RESERVED_BITS_BOOTBL        13
#define DIR_ENTRIES                 64
#define BOOT_DIR_ENTRIES            64
#define FOUR_KILO_BYTE              4096

/* Pointers needed to initialie our structs */
boot_block_t * bootblockptr;
dentry_t * currdentryptr;
inode_t * currinodeptr;


/* Boot Block Struct */
typedef struct f2_dir
{
    uint32_t num_of_dirs;
    uint32_t num_of_inodes;
    uint32_t num_of_dblocks;
    uint32_t reserved[RESERVED_BITS_BOOTBL];
    // 16 entries for d entries
    dentry_t dirEntries[DIR_ENTRIES]; // index 0,1 reserved

 } __attribute__((packed)) boot_block_t;


/* Dentry Struct */
typedef struct f1_dir
{

    // Possible array:
    uint8_t filename[MAX_FILENAME_LENGTH];
    uint32_t ftype;
    uint32_t inode;

    uint32_t reserved[RESERVED_BITS_DENTRY];

} __attribute__((packed)) dentry_t;



/* Inode Struct */
typedef struct f3_dir
{
    uint32_t length;
    uint32_t data_block[1020];

} __attribute__((packed)) inode_t;



/* Data Struct */
typedef struct f3_dir
{
    uint8_t data[4000];
} __attribute__((packed)) dataBlock_t;

// 4 structs
