#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

uint8_t buf[2000];

int terminal_open(void);
extern int terminal_close(void);
extern int terminal_read();
extern int terminal_write(int numb_bytes);