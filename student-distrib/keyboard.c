/** Keyboard Driver
 * rtc.c
*/
#include "keyboard.h"
#include "lib.h"
#include "i8259.h"


/* A map that can directly print the character relative to the scan code. */
uint8_t scancode_map_normal[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '*',
    '\0', ' ', '\0'
};

 uint8_t scancode_map_shift[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\0', '\0',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '|', '~', '\0', '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '\0', '*',
    '\0', ' ', '\0'
 };

 uint8_t scancode_map_caps_lock[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\0', '\0',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[', ']', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
    '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', '\0', '*',
    '\0', ' ', '\0'
};

uint8_t scancode_map_shift_and_caps_lock[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '{', '}', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',
    '|', '~', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '<', '>', '?', '\0', '*',
    '\0', ' ', '\0'
};

/*
void keyboard_init(void)
Description: Initialize the Keyboard
Inputs: none
Outputs: none
Side Effects: Enables interripts on IRQ 1.
*/
void keyboard_init(void) {
    capslock = 0;
    shiftflag = 0;
    ctrlflag = 0;
    altflag = 0;
    currkey = 0;
    enterflag = 0;
    enable_irq(KEYBOARD_IRQ);
    return;
}

/*
void keyboard_handler(void)
Description: Keyboard's Interrupt Handler
Inputs: none
Outputs: none
Side Effects: Prints what was typed on the keyboard.
*/
extern void keyboard_handler(void) {
    uint32_t keycode = inb(KEYBOARD_PORT);

    if((keycode == KEYBOARD_SHIFT_DOWN) || (keycode == KEYBOARD_SHIFT_DOWN2)){
        shiftflag = 1;
    }
    else if((keycode == KEYBOARD_SHIFT_UP) || (keycode == KEYBOARD_SHIFT_UP2)){
        shiftflag = 0;
    }
    else if(keycode == KEYBOARD_CAPS_LOCK){
        if(capslock){
            capslock = 0;
        }
        else{
            capslock = 1;
        }     
    }
    else if(keycode == KEYBOARD_ALT_DOWN){
        altflag = 1;
    }
    else if(keycode == KEYBOARD_ALT_UP){
        altflag = 0;
    }
    else if(keycode == KEYBOARD_CTRL_DOWN){
        ctrlflag = 1;
    }
    else if(keycode == KEYBOARD_CTRL_UP){
        ctrlflag = 0;
    }

    if((ctrlflag) && (keycode == KEYBOARD_L_KEY_DOWN)){
        int i;
        clear();
        currkey = 0;
        for(i=0; i<KEYBOARD_BUFFER_MAX_SIZE; i++){
            keyboardbuffer[i] = '\0';
        }
        set_screen_x(0);
        set_screen_y(0);
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    
    /* If it is out of range or is a function key processed above, we simply send an EOI and leave. */
    if ((keycode < 0) || (keycode >= KEYBOARD_INPUT_RANGE) || (keycode == KEYBOARD_CAPS_LOCK) || (keycode == KEYBOARD_CTRL_DOWN) || (keycode == KEYBOARD_ALT_DOWN) || (keycode == KEYBOARD_SHIFT_DOWN) || (keycode == KEYBOARD_SHIFT_DOWN2) || (currkey >=128)) {
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    /* Otherwise we retrive the character pressed. */
    char output;
    if((capslock == 1) && (shiftflag == 1)){
        output = scancode_map_shift_and_caps_lock[keycode];
    }
    else if(capslock == 1){
        output = scancode_map_caps_lock[keycode];
    }
    else if(shiftflag == 1){
        output = scancode_map_shift[keycode];
    }
    else{
        output = scancode_map_normal[keycode];
    }

    if(keycode == KEYBOARD_BACKSPACE){
        if(get_screen_x() > 0){
            set_screen_x(get_screen_x()-1);
            putc2(output);
            currkey = currkey - 1;
            keyboardbuffer[currkey] = output;
            set_screen_x(get_screen_x()-1);
            send_eoi(KEYBOARD_IRQ);
            return;
        }
        else if(currkey > 79){
            set_screen_y(get_screen_y()-1);
            set_screen_x(79);
            putc(output);
            currkey = currkey - 1;
            keyboardbuffer[currkey] = output;
            set_screen_x(79);
            send_eoi(KEYBOARD_IRQ);
            return;
        }
    }

    /* We next add that character to our buffer and print it to the screen if there is still room in the buffer */
    if(currkey < (KEYBOARD_BUFFER_MAX_SIZE - 1)){  // keyboard buffer size -1, since last char can only be newline
        keyboardbuffer[currkey] = output;
        currkey = currkey + 1;
        if(keycode == KEYBOARD_ENTER){
            enterflag = 1;
        }
        else
        putc2(output);
    }
    else if((keycode == KEYBOARD_ENTER) && (currkey == (KEYBOARD_BUFFER_MAX_SIZE - 1))){
        keyboardbuffer[currkey] = output;
        currkey = currkey + 1;
        enterflag = 1;
    }
    send_eoi(KEYBOARD_IRQ);
    return;
}

/*
    NOTES TO SELF
    Caps lock and Shift should XOR
    
    Caps Lock | Shift | Output
    0         | 0     | lowercase
    0         | 1     | UPPERCASE
    1         | 0     | UPPERCASE
    1         | 1     | lowercase
*/


