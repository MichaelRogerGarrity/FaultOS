/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

 /* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*
void i8259_init(void)
Description: Initialize the 8259 PIC:
Inputs: none
Outputs: none
Side Effects: Initialize the 8259 PIC, masks all interrupts
*/
void i8259_init(void) {

    // Maintaining status of interrupt prior to the init function through spinlock_irqsave
    unsigned long flags;
    cli_and_save(flags);

    // First, we mask all interrupts on master / slave : We use outb to send a masked signal (active high) to our mask variables
    master_mask = MASK_ALL_INT;                 // We are writing to the Data Port of Master - 0x21
    slave_mask = MASK_ALL_INT;                  // We are writing to the Data Port of Slave - 0xA1

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
    outb(ICW4, (SLAVE_8259_PORT + 1));             // We are writing to the Data Port of Master - 0x21

    // Slave:
    outb(ICW1, SLAVE_8259_PORT);                    // We are writing to the Command Port of Slave - 0xA0
    outb(ICW2_SLAVE, (SLAVE_8259_PORT + 1));       // We are writing to the Data Port of Slave - 0xA1
    outb(ICW3_SLAVE, (SLAVE_8259_PORT + 1));       // We are writing to the Data Port of Slave - 0xA1
    outb(ICW4, (SLAVE_8259_PORT + 1));              // We are writing to the Data Port of Slave - 0xA1

    // Restore the masks and flag
    outb(master_mask, (MASTER_8259_PORT + 1));
    outb(slave_mask, (SLAVE_8259_PORT + 1));
    restore_flags(flags);

}

/*
void enable_irq(uint32_t irq_num)
Description:  Enable (unmask) the specified IRQ
Inputs: irq_num
Outputs: none
Side Effects: Makes the IRQ active
*/

void enable_irq(uint32_t irq_num) {
    // Check if within range
    if (irq_num > 15 || irq_num < 0)
        return;

    // unsigned long flags;
    // cli_and_save(flags);


    uint8_t mask = 0x01;         // 0000 0001 - 1 will be shifted to current irq's position
    /*
    For the first 7 IRQs, we will put it in the master port.
    Then, till 15, we will put it in the slave port.
    */
    if (irq_num <= 7) {
        mask <<= irq_num;       // move 1 to correct position
        mask = ~mask;           // needs to be active low
        // enable irq on master_mask
        master_mask = master_mask & mask;
        outb(master_mask, (MASTER_8259_PORT + 1));
    }
    else {
        mask <<= (irq_num - 8);         // move 0 to correct position
        mask = ~mask;                   // needs to be active low
        // enable irq on slave_mask, since val is more than 7
        slave_mask = slave_mask & mask;
        outb(slave_mask, (SLAVE_8259_PORT + 1));
        // we must also unmask port 2 on master
        master_mask = master_mask & PORT_2_UNMASK;
        outb(master_mask, (MASTER_8259_PORT + 1));
    }
    // restore_flags(flags);
}

/*
void disable_irq(uint32_t irq_num)
Description:  Disable (mask) the specified IRQ
Inputs: irq_num
Outputs: none
Side Effects: Makes the IRQ port inactive
*/
void disable_irq(uint32_t irq_num) {
    // Check if within range
    if (irq_num > 15 || irq_num < 0)
        return;
    uint8_t mask = 0x01;        // 0000 0001 - 1 will be shifted to current irq's position


    if (irq_num > 7) {
        mask <<= (irq_num - 8);
    }
    else
        mask <<= irq_num;           // move 1 to correct position
    unsigned long flags;
    cli_and_save(flags);

    if (irq_num <= 7) {
        // enable irq on master_mask
        master_mask = master_mask | mask;
        outb(master_mask, (MASTER_8259_PORT + 1));
    }
    else {
        // enable irq on slave_mask, since val is more than 7
        slave_mask = slave_mask | mask;
        outb(slave_mask, (SLAVE_8259_PORT + 1));
        // we must also unmask port 2 on master
        master_mask = master_mask | ~PORT_2_UNMASK;
        outb(master_mask, (MASTER_8259_PORT + 1));
    }
    restore_flags(flags);
}

/*
void send_eoi(uint32_t irq_num)
Description:  Send end-of-interrupt signal for the specified IRQ
Inputs: irq_num
Outputs: none
Side Effects: Sends EOI signal
*/
void send_eoi(uint32_t irq_num) {
    // Check if within range
    if (irq_num > 15 || irq_num < 0)
        return;

    unsigned long flags;
    cli_and_save(flags);

    if (irq_num <= 7) {
        // enable irq on master_mask
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
    else {
        // obtain the correct port to mask:
        uint32_t slave_num = irq_num - 8;
        outb(EOI | slave_num, SLAVE_8259_PORT);
        outb(EOI | 2, MASTER_8259_PORT);
    }
    restore_flags(flags);

}
