/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* 

Description: Initialize the 8259 PIC:
void i8259_init(void)

*/
void i8259_init(void) {
    unsigned long flags;
    // include code to cli, save flags - spinlockirqsave

    // First, we mask all interrupts on master / slave : We use outb to send a masked signal (active high) to our mask variables
    outb(MASK_ALL_INT, master_mask);                // We are writing to the Data Port of Master - 0x21
    outb(MASK_ALL_INT, slave_mask);                 // We are writing to the Data Port of Slave - 0xA1

    /* We must initialize PIC by sending 4 keywords:
    W1. Tell PIC initialization is starting, cascade mode, 4 words
    W2. High bits of Vector #
    W3. Primary PIC: bit vector of secondary pic; Secondary PIC: input pin on primary PIC
    W4. ISA, normal / auto EOI
    */

    // Master:
    outb(ICW1, MASTER_8259_PORT);                   // We are writing to the Command Port of Master - 0x20
    outb(ICW2_MASTER, (MASTER_8259_PORT + 1));      // We are writing to the Data Port of Master - 0x21
    outb(ICW3_MASTER, (MASTER_8259_PORT + 1));      // We are writing to the Data Port of Master - 0x21
    outb(ICW4, (MASTER_8259_PORT + 1));             // We are writing to the Data Port of Master - 0x21

    // Slave:
    outb(ICW1, SLAVE_8259_PORT);                    // We are writing to the Command Port of Slave - 0xA0
    outb(ICW2_MASTER, (SLAVE_8259_PORT + 1));       // We are writing to the Data Port of Slave - 0xA1
    outb(ICW3_MASTER, (SLAVE_8259_PORT + 1));       // We are writing to the Data Port of Slave - 0xA1
    outb(ICW4, (SLAVE_8259_PORT + 1));              // We are writing to the Data Port of Slave - 0xA1


}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
}
