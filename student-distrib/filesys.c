#include "filesys.h"

static dentry_t currdentry;
static uint32_t offset; // change name 
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n);
// dir r,w,open,close
// file r,w,open,clode

/*
multiboot_info_t *mbi;
mbi = (multiboot_info_t *) addr;
module_t* mod = (module_t*)mbi->mods_addr;
*/
// ARPAN said put this in an init - call from kernel


/* file_init
* Inputs:   none
* Outputs: int - 0 means done
* Description: Initializes all our pointers / variables
            */
int32_t file_init(uint32_t startAddr) {
    fstart_adddr = startAddr;
    bootblockptr = (boot_block_t *)(fstart_adddr);
    currdentryptr = bootblockptr->dirEntries;
    inodeptr = (inode_t *)(bootblockptr + 1); // arpan change
    datablockptr = (dataBlock_t *)(bootblockptr + bootblockptr->num_of_inodes); // arpan change
    return 0;
}



// bootblockptr = (boot_block_t *)fstartaddr;
// currdentryptr = bootblockptr->dirEntries;
// // inodeptr = bootblockptr + 1; // we do +1 because we simply want to start after boot block in file.
// inodeptr = (inode_t *)(bootblockptr + 1); // arpan change
// // look back if + 1 is actually going 4kb at a time
// fstart_adddr = multiboot_info.mods_addr; //starting address of files
// void set_start_faddr(uint32_t startAddr)
// {
//     fstart_adddr = startAddr;

//     memcpy(dentry, fstart_adddr + 64 + 64 * index, 64);
// }

/* read_dentry_by_name
* Inputs:   fname
*           dentry
* Outputs: int - -1 if failed
* Description: When successful, the first two calls fill in the
dentry t block passed as their second argu  \th the file name,
file type, and inode number for the file, then return 0.
            */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
{
    int namepres = 0;
    int i;
    // uint32_t startaddr = fstart_adddr + 64;
    uint32_t index = 0;
    for (i = 0; i < 64; i++)
    {

        // int32_t isequal = strncmp((int8_t*) fname, const int8_t* s2, uint32_t n) {
    
        if (strcmp((int8_t*)fname, (int8_t*)currdentryptr[i].filename, 32 == 0)) //change to? bootblockptr->dirEntries[i].filename
        {
            // We found the file name!
            index = i;
            namepres = 1;
            break;
        }
    }
    if (!namepres)
    {
        return -1;
    }

    read_dentry_by_index(index, dentry);

    return 0;
}

/* read_dentry_by_index
* Inputs:   index, dentry ptr
*           dentry
* Outputs: int - -1 if failed || indicating a non-existent file or invalid index in the case of the first two calls,
* Description: When successful, the first two calls fill in the
dentry t block passed as their second argument with the file name,
file type, and inode number for the file, then return 0.
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
{
    if (index >= (DIR_ENTRIES - 1))
    { // -1 because we only have 63 indexes
        return -1;
        // find the addr to start copying memcpy(void* dest, const void* src, uint32_t n)
    }

    memcpy(&dentry, &(currdentryptr[index]), 64);
    //*(fstart_addr + FOUR_KILO_BYTE*i)
    // memcpy(fstart_adddr + 64 + 64 * index, dentry, 64);
    // dentry = currdentryptr[index]; // arpan change - but i think it sets pointers (not good)
    return 0;
}

/* read_data
 * Inputs:      inode, offset, buf, length
 * Outputs:     returning the number of bytes read and placed in the buffer.
 * Description: 0. The last routine works much like the read system call,
 reading up to length bytes starting from position offset in the file with inode
 number inode and returning the number of bytes read and placed in the buffer.
 A return value of 0 thus indicates that the end of the file has been reached.
-1 on failure, invalid inode number
Note that the directory entries are indexed starting with 0.
Also note that the read data call can only check that the given inode is within the valid range. 
It does not check that the inode actually corresponds to a file (not all inodes are used). 
However, if a bad data block number is found within the file bounds of the given inode,
the function should also return -1.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{

    int j;
    int curDataIdx;
    uint32_t curNbytes = 0;
    uint8_t * cur_data;
    
    /* check that the given inode is within the valid range. */
    if ( inode < 0 || (inode > (bootblockptr->num_of_inodes) - 1) )
        return -1;

    // arpan change: inodeptr[inode].length change (inode_t *)(inodeptr + inode).length

    // for(i = 0; i<63; i++){
    //     if(bootBlock.dirEntries[i].inode == inode){

    //     }
    // }
    // dont need to search since i node unique so do offset from fstart
    // check the inode
    // if ( ((inode_t *)(inodeptr + inode).length == 0) || (inode > (bootblockptr->num_of_inodes) - 1))
    // { // arpan change
    //     return -1;
    // }
    if ((inode_t *)(inodeptr + inode)->length == 0){
        return -1;
    }
        
    
    if (offset >= inodeptr->length){
        return 0; // mentioned page 19, appendix b
    }
    
    // /* Why would we "correct" the offset? We just escape */
    // while (correctedOffset > FOUR_KILO_BYTE)
    // { // calc corrected offset
    //     correctedOffset -= FOUR_KILO_BYTE;
    //     correctedDataIdx++;
    // }
    // i goes through length bytes starting from offset
    while(curNbytes < (uint32_t) ((inode_t *)(inodeptr + inode))->length /*i++*/)  // traversing through 
    {
        // Offset checker; 
        if  ( curNbytes >= inodeptr->length - offset){
            return curNbytes;
        }
        curDataIdx = (uint32_t)(inode_t *)(inodeptr + inode)->data_block[(offset + curNbytes)%FOUR_KILO_BYTE]; // gets into the current data block
        // cur_data =  (uint8_t *)(inode_t *)(inodeptr + inode)->data_block; //want the ptr 
        dataBlock_t * curdblockptr = (dataBlock_t *)(datablockptr + curDataIdx);
        
        cur_data = curdblockptr->data; //want the ptr 
        //curDataIdx += correctedDataIdx;
        
        for (j = (curNbytes%FOUR_KILO_BYTE) + offset; j < FOUR_KILO_BYTE && (curNbytes < (inode_t *)(inodeptr + inode)->length); j++)
        { // go through data block and copy 1 byte at a time 

            // put data into buffer
            buf[curNbytes] = cur_data[j]; // or do memcpy if this doesnt work doing one byte at a time 
            curNbytes++;
        }
    }

    return curNbytes;
}


// /* 4 main File open/close/r/w functions: */

// /* open_file
// * Inputs:   fname - to call read file_name
// * Outputs: int - -1 if failed
// * Description: Uses read_dentry_by_name, initializes any temporary structures. 
//             */
// int32_t open_file(const uint8_t *filename) {
//     /* Check if name is valid, and if read dentry call is valid. */
//     offset = 0;
//     if ((filename == NULL) || (read_dentry_by_name(filename, &currdentry) == -1))
//         return -1;
//     return 0;
// }

// /* close_file
// * Inputs:   file directory fd
// * Outputs: int - 0
// * Description: () undo what you did in the open function, return 0
//             */
// int32_t close_file(int32_t fd) {
//     return 0;
// }

// /* read_file
//  * Inputs:      file directory fd
//  *              buffer buf
//  *              num of bytes to be copied nbytes
//  * Outputs:     returning the number of bytes read and placed in the buffer.
//  * Description: reads count bytes of data from file into buf. Call read_data.
//  */
// int32_t read_file(int32_t fd, void* buf, int32_t nbytes) {
    
//     if (!nbytes)
//         return 0;
    
//         // get inode pointer (inode_t *)(inodeptr + inode)->length - this inode comes from curr dentry
//     read_data(currdentry.inode, offset, buf, nbytes);
//     offset += nbytes;
//     return 0;
// }

// /* write_file
//  * Inputs:   none
//  * Outputs: -1
//  * Description: should do nothing, return -1
//  */
// int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
//     return -1;
// }

// /* 4 main File open/close/r/w functions: */

// /* open_dir
// * Inputs:   fname - to call read file_name
// * Outputs: int - -1 if failed
// * Description: () opens a directory file (note file types), return 0
// read_dentry_by_name: name means filename

//             */
// int32_t open_dir(const uint8_t *filename) {
//     /* Check if name is valid, and if read dentry call is valid. */
//     // offset = 0;
//     // if ((filename == NULL) || (read_dentry_by_name(filename, &currdentry) == -1))
//     //     return -1;
//     return 0;
// }

// /* close_dir
// * Inputs:   file directory fd
// * Outputs: int - 0
// * Description: undo what you did in the open function, return 0
//             */
// int32_t close_dir(int32_t fd) {
//     return 0;
// }

// /* read_dir
//  * Inputs:      file directory fd
//  *              buffer buf
//  *              num of bytes to be copied nbytes
//  * Outputs:     returning the number of bytes read and placed in the buffer.
//  * Description: read files filename by filename, including “.”
// read_dentry_by_index: index is NOT inode number

//  */
// int32_t read_dir(int32_t fd,  void* buf, int32_t nbytes) {
    
//     if (!nbytes)
//         return 0;
    
//         // get inode pointer (inode_t *)(inodeptr + inode)->length - this inode comes from curr dentry
//     //read_data(currdentry.inode, offset, buf, nbytes);
//     //offset += nbytes;
//     return 0;
// }

// /* write_dir
//  * Inputs:   none
//  * Outputs: -1
//  * Description: should do nothing, return -1
//  */
// int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes) {
//     return -1;
// }


