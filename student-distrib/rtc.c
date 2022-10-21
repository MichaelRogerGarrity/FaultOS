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

// Declaration for the RTC Test interrupts.  
void test_interrupts();

void rtc_init(void) {
    // obtain older value of RTC RegA's old value



    outb(REG_B, RTC_PORT_IDX);                  // select B, disable NMI
    unsigned char prev_b = inb(RTC_PORT_RW);    // read curr B value
    outb(REG_B, RTC_PORT_IDX);                  // set idx again (due to resetting of index to reg D due to a read)
    outb(prev_b | BIT_SIX_ON, RTC_PORT_RW);     // turn on bit 6 and reqrite prev value  

    outb(REG_A, RTC_PORT_IDX);              // select B, disable NMI
    unsigned char prev_a = inb(RTC_PORT_RW);    // read curr A value
    outb(REG_A, RTC_PORT_IDX);                  // set idx again (due to resetting of index to reg D due to a read)
    outb((prev_a & 0xF0) | 6, RTC_PORT_RW);     // mask bottom 4 bits | 6 and get bits 1 and 2 turn on bit 6 and reqrite prev value  

    
    // turn on interrupts at IRQ number 8 because that is where RTC goes
    enable_irq(RTC_IRQ);
    return;
}

/*
void rtc_handler(void)
Description: RTC's Interrupt Handler
Inputs: none
Outputs: none
Side Effects: Does nothing
*/
extern void rtc_handler(void) {
    
   /* Read contents of Reg C - RTC will not generate another interrupt if this is not done */
    outb(REG_C, RTC_PORT_IDX);                  // select register C
    unsigned char temp = inb(RTC_PORT_RW);
    temp &= 0xFFFF;                             // Avoid warning of unused temp.
    /* Here we call test interrupts to make sure our RTC is working. */
    // test_interrupts();
    //putc('a');
    send_eoi(RTC_IRQ);
    return;
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

    int rate = 0x0F;			               // rate must be above 2 and not over 15

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
        return -1; break;
    }

    outb(REG_A, RTC_PORT_IDX);                  // set index to register A, disable NMI
    unsigned char prev_a = inb(RTC_PORT_RW);    // get initial value of register A
    outb(REG_A, RTC_PORT_IDX);
    outb(((prev_a & RATEBITS) | rate), RTC_PORT_RW);

    restore_flags(flags);
    return 0;

}
// /* MP3 Checkpoint 2 Stuff: */

/* 
rtc read must only return once the RTC interrupt occurs. 
You might want to use some sort of flag here (you will 
not need spinlocks. Why?)

RTC read() should block until the next interrupt, return 0
► NOT for reading the RTC frequency

// */
// int rtc_read(){
//   interrupt_rtc = 0;
//   while(interrupt_rtc == 0){
//     /* do nothing... */
//   }
//   return 0;
// }
// /*
// rtc write must get its input parameter through a buffer 
// and not read the value directly.

// RTC write() must be able to change frequency, return 0 or -1
// ► New frequency passed through buf ptr or through count?
// ► Frequencies must be power of 2

// */
// int rtc_write(const void* buf, uint32_t num_bytes){

//   /* sanity check */
//   if(buf == NULL) return -1;
//   if(num_bytes != sizeof(uint32_t)) return -1;

//   /* load freq <- input buffer */
//   int32_t frequency;
//   frequency = *((int*) buf);

//   /* sanity check on frequency */
//   if(frequency < MINFREQ | frequency > MAXFREQ) return -1;
//   /* HOW TO CHECK IFF A POWER OF TWO???????????? 
//   i think if it just has a single one, then its good, but how do you just check that
//   */

//   /* critical section */
//   cli();
//   rtc_set_freq(frequency);
//   sti();
//   return 0;
// }
// /* 
// rtc_open should reset the frequency to 2Hz. 
// RTC open() initializes RTC frequency to 2HZ, return 0
// */
// int rtc_open(void){
//   rtc_set_freq(OPEN_AT_2HZ);
//   return 0;
// }
// /*
// RTC close() probably does nothing, unless you virtualize RTC, return 0
// */
// int rtc_close(void){
//   return 0;
// }
