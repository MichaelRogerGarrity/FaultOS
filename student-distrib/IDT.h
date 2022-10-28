/* IDT.h - Defines used in interactions with the IDT
 */

#ifndef _IDT_H
#define _IDT_H

#define KEYBOARD_IDT_ENTRY          0x21
#define RTC_IDT_ENTRY               0x28
#define TOTAL_IDT_ENTRIES           256
#define SYSTEM_CALL_IDT_ENTRY       0x80

/* Function Definitions for the IDT Functions.  */
/* Externally-visible functions */

int (*funcs[TOTAL_IDT_ENTRIES])();
int generic_interrupt();
int divide_error();
int RESERVED();
int NMI();
int breakpoint();
int overflow();
int bound();
int InvalidOpcode();
int WAIT();
int DoubleFalt();
int overrun();
int TSS();
int segment();
int stackSegment();
int protect();
int pageFault();
int RESERVED2();
int FPU();
int allign();
int machine();
int SIMD();
int system_call_placeholder();
void init_IDT();

                                         /* System Call Number: */
int32_t halt(uint8_t status);                               // 1
int32_t execute(const uint8_t* command);                    // 2
int32_t read(int32_t fd, void* buf, int32_t nybytes);       // 3
int32_t write(int32_t fd, void* buf, int32_t nbytes);       // 4
int32_t open(const uint8_t* filename);                      // 5
int32_t close(int32_t fd);                                  // 6
int32_t getargs(uint8_t* buf, int32_t nbytes);              // 7
int32_t vidmap(uint8_t** screen_start);                     // 8
int32_t set_handler(int32_t signum, void* handler_address); // 9
int32_t sigreturn(void);                                    // 10

#endif /* _IDT_H */
