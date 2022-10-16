/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H


 /* Defined variables as used in the PIC functions */
#define KEYBOARD_IRQ            0x01
#define KEYBOARD_INPUT_RANGE    0x60


/* Ports that each Keyboard sits on */
#define KEYBOARD_PORT       0x60


  /* Externally-visible functions */

  /* Initialize both PICs */
void keyboard_init(void);
/* Keyboard's Interrupt Handler */
extern void keyboard_handler(void);

#endif /* _KEYBOARD_H */

