#include "terminal.h"
#include "lib.h"
#include "i8259.h"

int terminal_open(void){
    int i;
    for(i=0; i<2000; i++){
        buf[i] = '\0';
    }
    return 0;
}

extern int terminal_close(void){
    return 0;
}

extern int terminal_read(){
    int i;
    int count;
    for(i=0; i < 128; i++){
        count = i;
        if(keyboardbuffer[i] == '\n'){
            break;
        }
        
    }
    return count;
}

extern int terminal_write(int num_bytes){
    int i;
    for(i=0; i<num_bytes; i++){
        putc(buf[i]);
    }
    return 0;
}