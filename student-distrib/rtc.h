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




/* Ports that RTC/CMOS sits on */
#define RTC_PORT_IDX    0x70
#define RTC_PORT_RW     0x71
#define REG_A           0x8A
#define REG_B           0x8B
#define REG_C           0x8C
#define REG_D           0x8D

