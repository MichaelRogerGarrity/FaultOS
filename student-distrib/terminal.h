#include "keyboard.h"
#include "lib.h"
#include "i8259.h"


int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const uint8_t* buf, int32_t nbytes);


