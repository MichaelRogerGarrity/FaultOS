#include "terminal.h"
#include "lib.h"
#include "i8259.h"
#include "filesys.h"

/*
int32_t terminal_open(const uint8_t* filename)
Description: Terminal's open function
Inputs: const uint8_t* filename = name of file to be opened
Outputs: returns int32_t = 0 on success
*/
int32_t terminal_open(const uint8_t* filename){
    if(filename == NULL) return -1;
    return 0;
}

/*
int32_t terminal_close(int32_t fd)
Description: Terminal's close function
Inputs: int32_t fd = fild descriptor to close
Outputs: returns int32_t = 0 on success
*/
int32_t terminal_close(int32_t fd){
    if(fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    disable_cursor();
    return 0;
}

/*
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Terminal's read function. Reads from keyboardbuffer into the buffer passed in as buf
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = buffer to be written to
        int32_t nbytes = number of bytes to read from keyboardbuffer
Outputs: returns int32_t = number of bytes read
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    uint8_t* buft = (uint8_t*)buf;
    if(buft == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) return -1;
    int i;
    int count;
    int j;
    if(nbytes > KEYBOARD_BUFFER_MAX_SIZE){   // handles overflow by just chopping off extra bytes
        nbytes = KEYBOARD_BUFFER_MAX_SIZE;
    }
    /* Will loop through infinitely waiting for an enter key from user to return with their input */
    while(1){
        if(enterflag){
            int nullflag;
            nullflag = 0;
            /* Will copy keyboardbuffer to buf */
            for(i=0; i < nbytes; i++){
                if(keyboardbuffer[i] == '\0' && !nullflag){
                    count = i;
                    nullflag = 1;
                }
                    //count = i;
                    buft[i] = keyboardbuffer[i];
            }
            /* Clears out keyboardbuffer */
            keyboardbuffersize = 0;
            while(keyboardbuffer[keyboardbuffersize] != '\n'){
                keyboardbuffersize++;
            }
            keyboardbuffersize++;
            for(j=0; j<KEYBOARD_BUFFER_MAX_SIZE; j++){
                keyboardbuffer[j] = '\0';   // clear our buffer to prevent an infinite loop of read/write on the same line 
            }
            currkey = 0;
            //charcount = 0;
            enterflag = 0;
            return count-1;
        }

    //return 0;
    }
}

/*
int32_t terminal_write(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Terminal's read function. Reads from keyboardbuffer into the buffer passed in as buf
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = buffer to be read from
        int32_t nbytes = number of bytes to write to screen from buf
Outputs: returns int32_t = 0 on success
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    uint8_t* buft = (uint8_t*)buf;
    if(buft == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) return -1;
    int i;
    int linecount;


    set_screen_x(0);
    
    linecount = charcount/NUM_COLS;
    charcount = 0;
    //set_screen_y(get_screen_y()-linecount); // handles unknown number of lines fed as input
    if(keyboardbuffersize != 0)
    set_screen_y(get_screen_y()+1);
    for(i=0; i<nbytes; i++){
        if(buft[i] != '\0'){
            if(buft[i] == '\t'){ // tab is equivalent to four spaces
                putc2(' ');
                putc2(' ');
                putc2(' ');
                putc2(' '); 
            }
            else if (buft[i] == '\n')  {
                putc2('\n'); 
            }
            else {
                putc2(buft[i]); 
            }
        }
    }
    keyboardbuffersize = 0;

    return 0;
}


