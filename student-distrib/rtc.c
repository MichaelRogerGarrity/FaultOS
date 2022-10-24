/** RTC (Real Time Clock) Handler
 * rtc.c
 *
*/
#include "filesys.h"
#include "rtc.h"
#include "lib.h"
#include "i8259.h"

// Declaration for the RTC Test interrupts.  
void test_interrupts();

/*
void rtc_init(void)
Description:        Initialize the RTC
Inputs:             none
Outputs:            none
Side Effects:       Initialize the RTC
*/
void rtc_init(void) {

    /* btain older value of RTC RegA's old value */
    outb(REG_B, RTC_PORT_IDX);                  // select B, disable NMI
    unsigned char prev_b = inb(RTC_PORT_RW);    // read curr B value
    outb(REG_B, RTC_PORT_IDX);                  // set idx again (due to resetting of index to reg D due to a read)
    outb(prev_b | BIT_SIX_ON, RTC_PORT_RW);     // turn on bit 6 and reqrite prev value  

    outb(REG_A, RTC_PORT_IDX);              // select B, disable NMI
    unsigned char prev_a = inb(RTC_PORT_RW);    // read curr A value
    outb(REG_A, RTC_PORT_IDX);                  // set idx again (due to resetting of index to reg D due to a read)
    outb((prev_a & 0xF0) | 6, RTC_PORT_RW);     // mask bottom 4 bits | 6 and get bits 1 and 2 turn on bit 6 and reqrite prev value  
    interrupt_flag_rtc = 0;
    // turn on interrupts at IRQ number 8 because that is where RTC goes
    enable_irq(RTC_IRQ);
    rtc_set_freq(OPEN_AT_2HZ);
    return;
}

/*
void rtc_handler(void)
Description:        RTC's Interrupt Handler
Inputs:             none
Outputs:            none
Side Effects:       Does nothing
*/
extern void rtc_handler(void) {

    /* Read contents of Reg C - RTC will not generate another interrupt if this is not done */
    outb(REG_C, RTC_PORT_IDX);                  // select register C
    unsigned char temp = inb(RTC_PORT_RW);
    temp &= 0xFFFF;                             // Avoid warning of unused temp.
    /* Here we call test interrupts / or putc2 (new terminal function) to make sure our RTC is working. */
    // test_interrupts();
    // putc2('a');      
    interrupt_flag_rtc = 1;
    send_eoi(RTC_IRQ);
    return;
}


/*
void rtc_handler(void)
Description:        Change frequency of RTC
Inputs:             new freqency
Outputs:            Valid frequency (0) or invalid frequency (-1)
Side Effects:       Changes the RTC Frequency
*/
int rtc_set_freq(int newfreq) {
    // Disable interrupts

    unsigned long flags;
    cli_and_save(flags);
    int rate = 0x0F;			               // rate must be above 2 and not over 15
    /* Check if rate is between 2 and 15 */
    switch (newfreq) {
        /* Note: Numbers here are possible frequencies - they cannot be any other value. Taken from Datasheet. */
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
        // rate = RATE_FOR_2; break;
    }

    outb(REG_A, RTC_PORT_IDX);                  // set index to register A, disable NMI
    unsigned char prev_a = inb(RTC_PORT_RW);    // get initial value of register A
    outb(REG_A, RTC_PORT_IDX);
    outb(((prev_a & RATEBITS) | rate), RTC_PORT_RW);

    restore_flags(flags);
    return 0;

}

/* MP3 Checkpoint 2 Stuff: */

/* open_rtc
* Inputs:           fname
* Outputs:          -1 if RTC cannot be opened, 0 otherwise.
* Description:      rtc_open should reset the frequency to 2Hz.
RTC open()          initializes RTC frequency to 2HZ, return 0
*/
int32_t open_rtc(const uint8_t* filename) {
    if (filename == NULL)
        return -1;
    /* Start RTC off with the rate of 2. */
    rtc_set_freq(OPEN_AT_2HZ);
    return 0;
}


/* close_rtc
* Inputs:           file directory fd
* Outputs:          -1 if fd is invalid, otherwise 0
* Description:      RTC close() probably does nothing, unless you virtualize RTC, return 0
*/
int32_t close_rtc(int32_t fd) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL)
        return -1;
    return 0;
}

/* read_rtc
 * Inputs:          file directory fd
 *                  buffer buf
 *                  num of bytes to be copied nbytes
 * Outputs:         returning the number of bytes read and placed in the buffer.
 * Description      rtc read must only return once the RTC interrupt occurs.
                    You might want to use some sort of flag here (you will not need spinlocks)
                    RTC read() should block until the next interrupt, return 0
                    NOT for reading the RTC frequency.
 */
int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL || nbytes == NULL)
        return -1;
    while (!interrupt_flag_rtc) {
        /* This stays here until the RTC generates another interrupt. */
    }
    interrupt_flag_rtc = 0;
    // int8_t* name = "Int in RTC occurs once (Read only called once)";
    
    return 0;
}

/* write_file
 * Inputs:          none
 * Outputs:         -1 if any information is invalid. Otherwise, 0.
 * Description:
                    rtc write must get its input parameter through a buffer
                    and not read the value directly.
                    RTC write() must be able to change frequency, return 0 or -1
                    New frequency passed through buf ptr or through count?
                    Frequencies must be power of 2
 */
int32_t write_rtc(int32_t fd, const void* buf, int32_t nbytes) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL || nbytes == NULL)
        return -1;
    /* Set the default frequency to 2. */
    int chkfreq = 0;
    chkfreq = rtc_set_freq(OPEN_AT_2HZ);
    if (chkfreq < 0) 
        return -1;
    /* Extract the frequency from the buffer. */
    int32_t frequency;
    frequency = *((int*)buf);
    if ((frequency < MINFREQ) || (frequency > MAXFREQ)) return -1;
    /* critical section */
    cli();
    /* Check if the power of 2 is done in set frequency. */
    chkfreq = rtc_set_freq(frequency);
    if (chkfreq < 0) 
        return -1;
    sti();
    return 0;
}
