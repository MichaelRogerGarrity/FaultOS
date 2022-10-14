#include "x86_desc.h"

init_IDT(){
int i;
for(i = 0; i < 21; i++){
    idt_desc_t curr;

    curr_func_addr = funcs[i];

    curr.offset_15_00 = 0x00FF & curr_func_addr;
    curr.offset_31_16 = 0xFF00 & curr_func_addr;
    curr.reserved4 = 0;
    curr.reserved3 = 0;
    curr.reserved2 = 1;
    curr.reserved1 = 1;
    curr.size = 1;
    curr.reserved0 = 0;
    curr.dpl = 0;
    curr.present = 1;
    idt[i] = curr;

}


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