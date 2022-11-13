#include "keyboard.h"
#include "lib.h"
#include "i8259.h"


int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_switch(int32_t newTerminal);

/* Terminal Struct stuff */

int32_t currTerminal;

/* Struct for the Termial. */
typedef struct terminalStruct{
        //uint32_t val[2];
    uint8_t terminalbuffer[128];
    uint8_t index;
    int currRTC;
    int activePCB;

}__attribute__ ((packed)) terminalStruct_t;



