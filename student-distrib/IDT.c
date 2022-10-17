#include "x86_desc.h"
#include "IDT.h"
#include "lib.h"
#include "linkageheader.h"

int generic_interrupt()
{
    printf("Generic non-descript interrupt");
    while (1)
    {
    }
    return 0;
}

int divide_error()
{
    printf("Divide by zero error");
    while (1)
    {
    }
    return 0;
}

int RESERVED()
{
    printf("Reserved for Intel");
    while (1)
    {
    }
    return 0;
}

int NMI()
{
    printf("Nonmaskable external interrupt");
    while (1)
    {
    }
    return 0;
}

int breakpoint()
{
    printf("Breakpoint reached");
    while (1)
    {
    }
    return 0;
}

int overflow()
{
    printf("Overflow");
    while (1)
    {
    }
    return 0;
}

int bound()
{
    printf("Bounds range exceeded (BOUND)");
    while (1)
    {
    }
    return 0;
}

int InvalidOpcode()
{
    printf("Invalid opcode");
    while (1)
    {
    }
    return 0;
}

int WAIT()
{
    printf("Device not available");
    while (1)
    {
    }
    return 0;
}

int DoubleFalt()
{
    printf("Double fault");
    while (1)
    {
    }
    return 0;
}

int overrun()
{
    printf("Coprocessor segment overrun");
    while (1)
    {
    }
    return 0;
}

int TSS()
{
    printf("Invalid TSS");
    while (1)
    {
    }
    return 0;
}

int segment()
{
    printf("Segment not present");
    while (1)
    {
    }
    return 0;
}

int stackSegment()
{
    printf("Stack-segment fault");
    while (1)
    {
    }
    return 0;
}

int protect()
{
    printf("General protection fault");
    while (1)
    {
    }
    return 0;
}

int pageFault()
{
    printf("Page fault");
    while (1)
    {
    }
    return 0;
}

int RESERVED2()
{
    printf("Reserved");
    while (1)
    {
    }
    return 0;
}

int FPU()
{
    printf("x87 FPU error");
    while (1)
    {
    }
    return 0;
}

int allign()
{
    printf("Allignment check");
    while (1)
    {
    }
    return 0;
}

int machine()
{
    printf("Machine check");
    while (1)
    {
    }
    return 0;
}

int SIMD()
{
    printf("SIMD Floating-Point Exception");
    while (1)
    {
    }
    return 0;
}

void init_IDT()
{
    int i;

    // initializes our functions array
    for (i = 0; i < 256; i++)
    {
        if (i == 0)
            funcs[i] = &divide_error;
        else if (i == 1)
            funcs[i] = &RESERVED;
        else if (i == 2)
            funcs[i] = &NMI;
        else if (i == 3)
            funcs[i] = &breakpoint;
        else if (i == 4)
            funcs[i] = &overflow;
        else if (i == 5)
            funcs[i] = &bound;
        else if (i == 6)
            funcs[i] = &InvalidOpcode;
        else if (i == 7)
            funcs[i] = &WAIT;
        else if (i == 8)
            funcs[i] = &DoubleFalt;
        else if (i == 9)
            funcs[i] = &overrun;
        else if (i == 10)
            funcs[i] = &TSS;
        else if (i == 11)
            funcs[i] = &segment;
        else if (i == 12)
            funcs[i] = &stackSegment;
        else if (i == 13)
            funcs[i] = &protect;
        else if (i == 14)
            funcs[i] = &pageFault;
        else if (i == 15)
            funcs[i] = &RESERVED2;
        else if (i == 16)
            funcs[i] = &FPU;
        else if (i == 17)
            funcs[i] = &allign;
        else if (i == 18)
            funcs[i] = &machine;
        else if (i == 19)
            funcs[i] = &SIMD;
        else
            funcs[i] = &generic_interrupt;
    }

    // populates our IDT
    for (i = 0; i < 256; i++)
    {
        idt_desc_t curr;

        int *curr_func_addr = (void *)funcs[i];

        // curr.offset_15_00 = 0x0000FFFF & curr_func_addr;
        // curr.offset_31_16 = 0xFFFF0000 & curr_func_addr;

        if (curr_func_addr != 0)
        {
            curr.seg_selector = KERNEL_CS;
            curr.reserved4 = 0;
            curr.reserved3 = 0;
            curr.reserved2 = 1;
            curr.reserved1 = 1;
            curr.size = 1;
            curr.reserved0 = 0;
            curr.dpl = 0;
            curr.present = 1;
            SET_IDT_ENTRY(curr, (uint32_t *)curr_func_addr);
            idt[i] = curr;
        }
        else
        {
            printf("Bad function got into our array somehow");
            while (1)
            {
            };
        }
    }
    
    SET_IDT_ENTRY(idt[0x21], keyboard_handler_function);            // Keyboard is in IDT entry table 0x21
    SET_IDT_ENTRY(idt[0x28], rtc_handler_linkage);                  // RTC is in IDT entry table 0x28

    // lidt(idt_desc_ptr);

    // puts system call
    return;
}

// alternative way is to, instead of looping, declare each separately. All comes
// down to whether we can store function addresses in an array like this.

// uint64_t DE;
// uint64_t DB;
// uint64_t BP;
// uint64_t OF;
// uint64_t BR;
// uint64_t UD;
// uint64_t NM;
// uint64_t DF;
// uint64_t CSO;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
// uint64_t TS;
