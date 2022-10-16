/** RTC (Real Time Clock) Handler
 * rtc.c
 *
*/

#include "rtc.h"
#include "lib.h"
#include "i8259.h"

volatile int interrupt_rtc = 0;
/*
void rtc_init(void)
Description: Initialize the RTC
Inputs: none
Outputs: none
Side Effects: Initialize the RTC
*/
void rtc_init(void) {
    // obtain older value of RTC RegA's old value
    outb(REG_B, RTC_PORT_IDX);                  // select B, disable NMI
    unsigned char prev_b = inb(RTC_PORT_RW);    // read curr B value
    outb(REG_B, RTC_PORT_IDX);                  // set idx again (due to resetting of index to reg D due to a read)
    outb(prev_b | BIT_SIX_ON, RTC_PORT_RW);     // turn on bit 6 and reqrite prev value  
    // turn on interrupts at IRQ number 8 because that is where RTC goes
    enable_irq(RTC_IRQ);
}

/*
void rtc_handler(void)
Description: RTC's Interrupt Handler
Inputs: none
Outputs: none
Side Effects: Code to handle the RTC Interrupt
*/
void rtc_handler(void) {
    // Disable interrupts
    unsigned long flags;
    cli_and_save(flags);
    // Read contents of Reg C - RTC will not generate another interrupt if this is not done
    outb(REG_C, RTC_PORT_IDX);     // select register C
    unsigned char temp = inb(RTC_PORT_RW);
    send_eoi(RTC_IRQ);
    restore_flags(flags);
    interrupt_rtc = 1;
}


/*
void rtc_handler(void)
Description: Change frequency of RTC
Inputs: new freqency
Outputs: Valid frequency (0) or invalid frequency (-1)
Side Effects: Changes the RTC Frequency
*/
int rtc_set_freq(int newfreq) {
    // Disable interrupts

    /* Check if rate is between 2 and 15 */
    if (newfreq < MINFREQ || newfreq > MAXFREQ)
        return -1;
    unsigned long flags;
    cli_and_save(flags);

    int rate = 0x0F;			                // rate must be above 2 and not over 15

    outb(REG_A, RTC_PORT_IDX);                  // set index to register A, disable NMI
    unsigned char prev_a = inb(RTC_PORT_RW);    // get initial value of register A
    outb(REG_A, RTC_PORT_IDX);
    outb(((prev & RATEBITS) | rate), RTC_PORT_RW);


    // outportb(0x70, 0x8A);
    // char prev = inportb(0x71);
    // outportb(0x70, 0x8A);		// reset index to A
    // outportb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
    // enable_ints();

    restore_flags(flags);
    return 0;

}