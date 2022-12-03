/** PIT Handler + Scheduler
 * scheduler.c
 *
 */
#include "lib.h"
#include "i8259.h"
#include "scheduler.h"

int cnt_pit_handler = 0;

// used code from https://wiki.osdev.org/Programmable_Interval_Timer

/*
void pit_init(void)

Inputs:             none
Outputs:            none
Side Effects:       Initialize the PIT
Description:        
int divisor = 1193180 / hz;       Calculate our divisor 
outportb(0x43, 0x36);             Set our command byte 0x36 
outportb(0x40, divisor & 0xFF);   Set low byte of divisor 
outportb(0x40, divisor >> 8);     Set high byte of divisor 
*/
void pit_init(void)
{
    int32_t divisor = PIT_INIT_CONST / PIT_INIT_FREQ;
    outb(PIT_CMD_NUM, PIT_CMD_PORT);
    outb(divisor & LOW_BYTE, CHAN_0);
    outb(divisor >> EIGHT_SHIFT, CHAN_0);
    // enable_irq(0);
    return;
}

/*
void pit_handler(void)
Description:        Calls Scheduler
Input:             none
Outputs:            none
Side Effects:       Calls scheduler.
*/
void pit_handler(void)
{
    cnt_pit_handler++;
    scheduler();
    //send_eoi(PIT_IRQ);
    return;
}




/* Obtain the PCB of the new terminal -
terminalArray[terminalrunning].pcbptr
*/
/* Check if the base shell has started yet

if not started: {
    initialize all values, set the flag to 1.
    copy in the ESP, EBP into the PCB
    adjust pcb to terminal pointers to each other
    switch to a new terminal
    sent the EOI
    execute the shell
}

otherwise {
    save esp, ebp
    change current terminal
        ?? might have to see if that is also not initialized (pit interrupr #2, 3)
        ?? if not initialized we must recall the scheduler so we send eoi and call pit handler again
    change video memory / then the terminal video mapping
    now we change user page as well for the new process
    do context switch:
        1. tss.esp0 = 8MB - 8KB*newprocessid
    change esp, ebp of new process
    send end of interrupt
    ^^ do we switch order of these both?? eoi stuff

}
*/

/*
T1        T2        T3      - structs
[P0]     [P1]       [P2]
         [P3]       [P5]
         [P4]

    - pcb ptr / processnum
[P0] - Terminal 1 Shell - esp ebp
[P1] - Terminal 2 Shell
[P2] - Terminal 3 Shell
[P3] - Terminal 2 Shell
[P4] - Terminal 2 Counter
[P5] - Terminal 3 Ls

Currterminal -> terminalseen                Variable 1: Which terminal the user is looking at
Currterminal/globalpcb -> terminalrunning   Variable 2: Which terminal is actually running in the background

M1: [T1 - kernel][Pit handler][Pit handler][][][]

M2: [][][][][][] - 1. ask a ta about this - all 3 by pit handler

M3: use execute shell - weird edge cases

*/
//need to enable irq 0 after 1st excute 

/*
void scheduler()
Description:       switches between 3 terminals's cur process round robin
Input:             none
Outputs:            none
Side Effects:       context switches between 3 terminals cur processes 
*/
void scheduler(){
    
    cli();

    send_eoi(PIT_IRQ); //after the asm? //ADDED

    pcb_t * prev_pcb;
    pcb_t * next_pcb;
    int oldterm = terminalrun;
    if(terminalrun == -1){
        oldterm = 2;
    }
    int newterm =  ((terminalrun + 1) % 3);
    terminalrun = newterm;
    prev_pcb = terminalArray[oldterm].cur_PCB;
    next_pcb = terminalArray[newterm].cur_PCB;

    /* Old esp ebp from old terminal */
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    
    terminalArray[oldterm].savedt_esp = saved_esp;
    terminalArray[oldterm].savedt_ebp = saved_ebp;

    if(next_pcb == NULL){
        currpid++;
        execute_on_term((const uint8_t *)("shell"),currpid);
        
    }
    //else{
        /* Switch execution to current terminal's user program */
        // uint32_t physaddr = (PDE_PROCESS_START + terminalArray[next_pcb->termid].currprocessid) * FOUR_MB;
        uint32_t physaddr = (PDE_PROCESS_START + next_pcb->pid) * FOUR_MB;
        map_helper(PDE_VIRTUAL_MEM, physaddr);

        /* Context switch */
        tss.esp0 = EIGHT_MEGA_BYTE - EIGHT_KILO_BYTE * next_pcb->pid;

        /* (b) Set TSS for parent. ksp = kernel stack pointer */
        uint32_t args_esp = terminalArray[newterm].savedt_esp;
        uint32_t args_ebp = terminalArray[newterm].savedt_ebp ;
        
        asm volatile(
            /* set esp, ebp as esp ebp args */
            "   pushl %0 \n"
            "   pushl %1 \n"
            "   popl %%ebp \n"
            "   popl %%esp \n"
            :
            : "r"(args_esp), "r"(args_ebp) // input
            : "cc"                         // ?
        );
   //}
    
    return;

}


