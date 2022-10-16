/** Keyboard Driver
 * rtc.c
 *
*/

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

uint8_t scancode_map_normal[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\0', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '*',
    '\0', ' ', '\0'
};


// uint8_t scancode_map_shift[KEYBOARD_INPUT_RANGE] = {

// };

/*
void keyboard_init(void)
Description: Initialize the Keyboard
Inputs: none
Outputs: none
Side Effects: Enables interripts on IRQ 1.
*/
void keyboard_init(void) {
    enable_irq(KEYBOARD_IRQ);

}

/*
void keyboard_handler(void)
Description: Keyboard's Interrupt Handler
Inputs: none
Outputs: none
Side Effects: Prints what was typed on the keyboard.
*/
extern void keyboard_handler(void) {
    unsigned long flags;
    cli_and_save(flags);

    uint8_t keycode = inb(KEYBOARD_PORT);

    if (keycode < 0 || keycode >= KEYBOARD_INPUT_RANGE) {
        send_eoi(KEYBOARD_IRQ);
        restore_flags(flags);

    }
    putc(scancode_map_normal[keycode]);
    send_eoi(KEYBOARD_IRQ);
    restore_flags(flags);
}
