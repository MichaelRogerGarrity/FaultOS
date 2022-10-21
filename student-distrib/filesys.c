#include "filesys.h"

// dir r,w,open,close
// file r,w,open,clode

/*
multiboot_info_t *mbi;
mbi = (multiboot_info_t *) addr;
module_t* mod = (module_t*)mbi->mods_addr;
*/

uint32_t fstart_adddr;
bootblockptr = (boot_block_t *)fstartaddr;
currdentryptr = bootblockptr->dirEntries;
inodeptr = bootblockptr + 1; // we do +1 because we simply want to start after boot block in file.
// look back if + 1 is actually going 4kb at a time 

// fstart_adddr = multiboot_info.mods_addr; //starting address of files

/* read_dentry_by_name
* Inputs:   fname
*           dentry
* Outputs: int - -1 if failed
* Description: When successful, the first two calls fill in the
dentry t block passed as their second argu  \th the file name,
file type, and inode number for the file, then return 0.
            */

void set_start_faddr(uint32_t startAddr)
{
    fstart_adddr = startAddr;

    memcpy(dentry, fstart_adddr + 64 + 64 * index, 64);
}

int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
{
    int namepres = 0;
    // uint32_t startaddr = fstart_adddr + 64;
    uint32_t index = 0;
    for (int i = 0; i < 64; i++)
    {
        if ((strcmp(fname), currdentryptr[i].fname, 32) == 0){
            // We found the file name!
            index = i;
            namepres = 1;
            break;
        }
    }
    if (!namepres){
        return -1;
    }
    
    read_dentry_by_index(index, dentry);

    return 0;
}

/* read_dentry_by_index
* Inputs:   index, dentry ptr
*           dentry
* Outputs: int - -1 if failed
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
    //
    //*(fstart_addr + FOUR_KILO_BYTE*i)
    memcpy(dentry, fstart_adddr + 64 + 64 * index, 64);

    return 0;
}

/* read_data
 * Inputs:   inode, offset, buf, length
 * Outputs: int - -1 if failed
 * Description: 0. The last routine works much like the read system call,
 reading up to length bytes starting from position offset in the file with inode
 number inode and returning the number of bytes read and placed in the buffer.
 A return value of 0 thus indicates that the end of the file has been reached.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
    
    int i,j;
    int curDataIdx;
    int correctedOffset;
    correctedOffset = offset;
    int correctedDataIdx; 
    correctedDataIdx = 0;
    // for(i = 0; i<63; i++){
    //     if(bootBlock.dirEntries[i].inode == inode){
            
    //     }
    // }
    //dont need to search since i node unique so do offset from fstart
    //check the inode 
    if( (inodeptr[inode].length == 0) || (inode > (bootblockptr->num_of_inodes) - 1) ){
        return -1;
    }

    while(correctedOffset > 4000){ // calc corrected offset
        correctedOffset -=4000;
        correctedDataIdx ++;
    }

    for(i = 0; i<inodeptr[inode].length; i++){
        curDataIdx = dataBlock[inodeptr[inode].data_block[i]]; // gets into the current data block
        curDataIdx +=correctedDataIdx;
        for(j = 0; j<4000; j++){ // go through data file
            if(i == 0){
                j+=correctedOffset;
            }

            //put data into buffrt
        }        
    }

    

        return 0;
}
