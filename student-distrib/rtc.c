/** RTC (Real Time Clock) Handler
 * rtc.c
 *
*/

#include "rtc.h"
#include "lib.h"
#include "i8259.h"

//volatile int interrupt_rtc = 0;
/*
void rtc_init(void)
Description: Initialize the RTC
Inputs: none
Outputs: none
Side Effects: Initialize the RTC
*/
void rtc_init(void) {
    // obtain older value of RTC RegA's old value

    unsigned long flags;
    cli_and_save(flags);
    outb(REG_B, RTC_PORT_IDX);                  // select B, disable NMI
    unsigned char prev_b = inb(RTC_PORT_RW);    // read curr B value
    outb(REG_B, RTC_PORT_IDX);                  // set idx again (due to resetting of index to reg D due to a read)
    outb(prev_b | BIT_SIX_ON, RTC_PORT_RW);     // turn on bit 6 and reqrite prev value  
    // turn on interrupts at IRQ number 8 because that is where RTC goes
    enable_irq(RTC_IRQ);
    restore_flags(flags);
}

/*
void rtc_handler(void)
Description: RTC's Interrupt Handler
Inputs: none
Outputs: none
Side Effects: Does nothing
*/
extern void rtc_handler(void) {
    
    // Read contents of Reg C - RTC will not generate another interrupt if this is not done
    outb(REG_C, RTC_PORT_IDX);     // select register C
    unsigned char temp = inb(RTC_PORT_RW);
    temp &= 0xFFFF;
    putc('a');
    send_eoi(RTC_IRQ);
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

    unsigned long flags;
    cli_and_save(flags);

    int rate = 0x0F;			                // rate must be above 2 and not over 15

    /* Check if rate is between 2 and 15 */
    switch (newfreq) {
        /* Note: Numbers here are possible frequencies - they cannot be any other value. */
    case 1024:
        rate = RATE_FOR_1024; break;
    case 512:
        rate = RATE_FOR_512; break;
    case 256:
        rate = RATE_FOR_256; break;
    case 128:
        rate = RATE_FOR_128; break;
    case 64:
        rate = RATE_FOR_64; break;
    case 32:
        rate = RATE_FOR_32; break;
    case 16:
        rate = RATE_FOR_16; break;
    case 8:
        rate = RATE_FOR_8; break;
    case 4:
        rate = RATE_FOR_4; break;
    case 2:
        rate = RATE_FOR_2; break;
    default:
        return -1;
    }

    outb(REG_A, RTC_PORT_IDX);                  // set index to register A, disable NMI
    unsigned char prev_a = inb(RTC_PORT_RW);    // get initial value of register A
    outb(REG_A, RTC_PORT_IDX);
    outb(((prev_a & RATEBITS) | rate), RTC_PORT_RW);

    restore_flags(flags);
    return 0;

}

