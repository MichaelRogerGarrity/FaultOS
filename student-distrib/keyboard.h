/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#include "i8259.h"


#ifndef _KEYBOARD_H
#define _KEYBOARD_H


 /* Defined variables as used in the PIC functions */
#define KEYBOARD_IRQ                0x01
#define KEYBOARD_INPUT_RANGE        59

/* Defined variables as given by OsDev: https://wiki.osdev.org/PS/2_Keyboard */
#define KEYBOARD_SHIFT_DOWN         42
#define KEYBOARD_SHIFT_DOWN2        54
#define KEYBOARD_SHIFT_UP           170
#define KEYBOARD_SHIFT_UP2          182
#define KEYBOARD_CAPS_LOCK          58
#define KEYBOARD_ALT_DOWN           56
#define KEYBOARD_ALT_UP             184
#define KEYBOARD_CTRL_DOWN          29
#define KEYBOARD_CTRL_UP            158
#define KEYBOARD_ENTER              28

/* Ports that each Keyboard sits on */
#define KEYBOARD_PORT               0x60

/* Buffer for characters written to terminal */
uint8_t keyboardbuffer[128];
int capslock;
int shiftflag;
int ctrlflag;
int altflag;
int currkey;

/* Externally-visible functions */

/* Initialize both PICs */
void keyboard_init(void);
/* Keyboard's Interrupt Handler */
extern void keyboard_handler(void);



#endif /* _KEYBOARD_H */
