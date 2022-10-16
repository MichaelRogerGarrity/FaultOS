/* rtc.h - Defines used in interactions with the RTC
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

 /* Defined variables as used in the PIC functions */
#define BIT_SIX_ON          0x40
#define RTC_IRQ             0x08
#define MINFREQ             0x02
#define MAXFREQ             1024
#define RATEBITS            0xF0


/* Ports that RTC/CMOS sits on */
#define RTC_PORT_IDX    0x70
#define RTC_PORT_RW     0x71
#define REG_A           0x8A
#define REG_B           0x8B
#define REG_C           0x8C
#define REG_D           0x8D

/* Possible rates corresponding to valid frequencies: */
#define RATE_FOR_1024   0x06    // 0110
#define RATE_FOR_512    0x07    // 0111
#define RATE_FOR_256    0x08    // 1000
#define RATE_FOR_128    0x09    // 1001
#define RATE_FOR_64     0x0A    // 1010
#define RATE_FOR_32     0x0B    // 1011
#define RATE_FOR_16     0x0C    // 1100
#define RATE_FOR_8      0x0D    // 1101
#define RATE_FOR_4      0x0E    // 1110
#define RATE_FOR_2      0x0F    // 1111



  /* Externally-visible functions */

/* Initialize the RTC */
void rtc_init(void);
/* RTC's Interrupt Handler */
extern void rtc_handler(void);
/* Change frequency of RTC */
int rtc_set_freq(int newfreq);

#endif /* _RTC_H */

