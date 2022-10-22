#include "terminal.h"
#include "lib.h"
#include "i8259.h"

int32_t terminal_open(const uint8_t* filename){
    // int i;
    // for(i=0; i<2000; i++){
    //     buf[i] = '\0';
    // }
    return 0;
}

int32_t terminal_close(int32_t fd){
    return 0;
}

int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes){
    int i;
    int count;
    int j;
    if(nbytes > KEYBOARD_BUFFER_MAX_SIZE){   // handles overflow by just chopping off extra bytes
        nbytes = KEYBOARD_BUFFER_MAX_SIZE;
    }
    while(1){
        if(enterflag){
        for(i=0; i < nbytes; i++){
            count = i;
            buf[i] = keyboardbuffer[i];
        }
        keyboardbuffersize = 0;
        while(keyboardbuffer[keyboardbuffersize] != '\n'){
        keyboardbuffersize++;
        }
        keyboardbuffersize++;
        for(j=0; j<KEYBOARD_BUFFER_MAX_SIZE; j++){
                    keyboardbuffer[j] = '\0';   // clear our buffer to prevent an infinite loop of read/write on the same line
                }
                currkey = 0;
                enterflag = 0;
                return count;
        }

    //return 0;
    }
}

int32_t terminal_write(int32_t fd, const uint8_t* buf, int32_t nbytes){
    int i;
    set_screen_x(0);
    for(i=0; i<keyboardbuffersize; i++){
        putc2(buf[i]);
    }
    //i = puts2(buf, nbytes);

    return 0;
}


