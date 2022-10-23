#include "filesys.h"
#include "lib.h"
static dentry_t currdentry;
static uint32_t offset; // change name 
static uint32_t diridx;
/* file_init
* Inputs:   none
* Outputs: int - 0 means done
* Description: Initializes all our pointers / variables and sets pointers for structs to be used.
*/
int32_t file_init(uint32_t startAddr) {
    fstart_adddr = startAddr;
    bootblockptr = (boot_block_t *)(fstart_adddr);
    currdentryptr = bootblockptr->dirEntries;
    inodeptr = (inode_t *)(bootblockptr + 1); // arpan change
    datablockptr = (dataBlock_t *)(bootblockptr + bootblockptr->num_of_inodes +1);
    return 0;
}

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
    // Name check
    const int8_t* s1 = (int8_t *)fname ;
    uint32_t namelen = strlen(s1);
    if (namelen > MAX_FILENAME_LEN)
        return -1;
    int namepres = 0;
    int i;
    diridx = 0;

    // This traverses through directory and tries to find the file with matching name.
    for (i = 0; i < bootblockptr->num_of_dirs; i++)
    {
        dentry_t tempd = currdentryptr[i];
        const int8_t* s2 = (int8_t *)(tempd.filename);

        // CHECKED IF DIR ENTRY IS VALID - either check filename null
        // OR check number of directory entries (assuming )
        
        int32_t temp = strncmp(s1, s2, MAX_FILENAME_LEN);
        // File was found, and we want the index of the file, and leave the loop
        if (temp == 0) {
            diridx = i;
            namepres = 1;
            break;
        }
    }

    if (!namepres) {
        return -1;                                                      // The file was not found. Leave.
    }
    read_dentry_by_index(diridx, dentry);                               // Put into our dentry the 
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
    if (index >= (DIR_ENTRIES - 1) || index < 0 || dentry == NULL) { 
        return -1;                                                      // -1 because we only have 63 indexes
    }
    //void* memcpy(void* dest, const void* src, uint32_t n) {
    dentry_t tempdent = (currdentryptr[index]);

    strncpy(dentry->filename, tempdent.filename, 32);
    dentry->ftype = tempdent.ftype;
    dentry->inode = tempdent.inode;

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
    if (buf == NULL)
        return -1;
    int j;
    int curDataIdx;
    uint32_t curNbytes = 0;
    uint8_t * cur_data;
    inode_t *curInodePtr = (inode_t *)(inodeptr + inode);
    
    /* check that the given inode is within the valid range. */
    if ( inode < 0 || (inode > (bootblockptr->num_of_inodes) - 1) )
        return -1;

    if (curInodePtr->length == 0){
        return -1;
    }
    // i goes through length bytes starting from offset

    /* Variable meanings: 
    curNbytes - running total of bytes done / read
    curDataIdx - data block index inside the current inode
    curdblockptr - pointer to current data block being read
    cur_data - uint8 array of data block (has 4096 values)
    curInodePtr - pointer to the relevant Inode
    j = index inside block - adjust for starting & ending & offset
    */
    
    while(curNbytes < (uint32_t)(curInodePtr->length) /*i++*/)  // traversing through 
    {
        //Offset checker; 
        if  (curNbytes >= curInodePtr->length - offset){
            return curNbytes;
        }
        // curDataIdx = (uint32_t)(inode_t *)(inodeptr + inode)->data_block[((offset + curNbytes)/FOUR_KILO_BYTE) % 1023]; // gets into the current data block
        int dblockidx =((offset + curNbytes)/FOUR_KILO_BYTE);
        if (dblockidx >= 1023)
            return curNbytes;
        
        curDataIdx = curInodePtr->data_block[dblockidx];
        dataBlock_t * curdblockptr = (dataBlock_t *)(datablockptr + curDataIdx);
        
        cur_data = curdblockptr->data;
        uint8_t temp, temp1;
        // Goes through each data block.
        for (j = ((curNbytes+offset)%FOUR_KILO_BYTE); (j < FOUR_KILO_BYTE) && (curNbytes < (uint32_t)(curInodePtr->length)); j++)
        {
            // go through data block and copy 1 byte at a time 
            // put data into buffer
            buf[curNbytes] = cur_data[j]; 
            temp =  cur_data[j];
            temp1 =  buf[curNbytes];
            curNbytes++;
        }
    }
    return curNbytes;
}


/* 4 main File open/close/r/w functions: */

/* open_file
* Inputs:   fname - to call read file_name
* Outputs: int - -1 if failed
* Description: Uses read_dentry_by_name, initializes any temporary structures. 
*/
int32_t open_file(const uint8_t *filename) {
    /* Check if name is valid, and if read dentry call is valid. */
    const int8_t* s = (int8_t*)filename;
    uint32_t namelen = strlen(s);
    if (namelen > MAX_FILENAME_LEN) {
        return -1;
    }
    diridx = 0;
    offset = 0;
    if ((filename == NULL) || (read_dentry_by_name(filename, &currdentry) == -1))
        return -1;
    return 0;
}

/* close_file
* Inputs:   file directory fd
* Outputs: int - 0
* Description: () undo what you did in the open function, return 0
*/
int32_t close_file(int32_t fd) {
    return 0;
}

/* read_file
 * Inputs:      file directory fd
 *              buffer buf
 *              num of bytes to be copied nbytes
 * Outputs:     returning the number of bytes read and placed in the buffer.
 * Description: reads count bytes of data from file into buf. Call read_data.
 */
int32_t read_file(int32_t fd, void* buf, int32_t nbytes) {
    if (buf == NULL) 
        return -1;
    if (!nbytes)
        return 0;
    read_data(currdentry.inode, offset, buf, nbytes);
    offset += nbytes;
    return 0;
}

/* write_file
 * Inputs:   none
 * Outputs: -1
 * Description: should do nothing, return -1
 */
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL) 
        return -1;
    return -1;
}

/* 4 main File open/close/r/w functions: */

/* open_dir
* Inputs:   fname - to call read file_name
* Outputs: int - -1 if failed
* Description: () opens a directory file (note file types), return 0
read_dentry_by_name: name means filename
*/
int32_t open_dir(const uint8_t *filename) {
    const int8_t* s = (int8_t*)filename;
    uint32_t namelen = strlen(s);
    if ((filename == NULL) || namelen > MAX_FILENAME_LEN) {
        return -1;
    }
    //  read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
    read_dentry_by_name(filename, &currdentry);
    return 0;
}

/* close_dir
* Inputs:   file directory fd
* Outputs: int - 0
* Description: undo what you did in the open function, return 0
            */
int32_t close_dir(int32_t fd) {
    return 0;
}

/* read_dir
 * Inputs:      file directory fd
 *              buffer buf
 *              num of bytes to be copied nbytes
 * Outputs:     returning the number of bytes read and placed in the buffer.
 * Description: read files filename by filename, including “.”
read_dentry_by_index: index is NOT inode number

 */
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes) {
    if (buf == NULL)
        return -1;
    if (!nbytes)
        return 0;
    int i = 0;

    // loop through num of chars
    // uint32_t namelen = strlen(bootblockptr->dirEntries[i].filename);
    // INSERT NAMECHECK
    read_dentry_by_index(diridx, &currdentry);
    // strncpy(int8_t* dest, const int8_t*src, uint32_t n);
    int namelen =  strlen((int8_t *)(currdentry.filename));
    if ( namelen >= MAX_FILENAME_LEN)
        namelen =  MAX_FILENAME_LEN;
    
    int8_t wholestr[MAX_FILENAME_LEN];
    int travval = 0;
    for ( i = 0 ; i < MAX_FILENAME_LEN; i++) {
        if ((i < namelen)) {
            wholestr[i] = currdentry.filename[i];
        }
        else {
            wholestr[i] = '\0';
        }

    }
    strncpy((int8_t*)buf, (int8_t*)(wholestr), 32);
    // int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
    diridx++;
    return 0;
}

/* write_dir
 * Inputs:   none
 * Outputs: -1
 * Description: should do nothing, return -1
 */
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL)
        return -1;
    return -1;
}


