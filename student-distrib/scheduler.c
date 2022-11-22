/** PIT Handler + Scheduler
 * scheduler.c
 *
 */
#include "lib.h"
#include "i8259.h"
#include "scheduler.h"

int cnt_pit_handler = 0;

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
    int32_t divisor = 1193180 / 100;
    outb(0x36, PIT_CMD_PORT);
    outb(divisor & LOW_BYTE, CHAN_0);
    outb(divisor >> EIGHT_SHIFT, CHAN_0);
    
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
void scheduler(){
    cli();
     //printf("%d",currpid);
    pcb_t * prev_pcb;
    prev_pcb = globalpcb;
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    prev_pcb->saved_esp = (void *)saved_esp;
    prev_pcb->saved_ebp = (void *) saved_ebp;
    terminalrun = ((terminalrun + 1) % 3);
    globalpcb = terminalArray[terminalrun].cur_PCB;
    if(terminalArray[terminalrun].cur_PCB == NULL){
        currpid++;
       // printf("%d",currpid);
        send_eoi(PIT_IRQ);
        sti();
        execute((const uint8_t *)("shell"));
       ;
    }
    else{
        /* Switch execution to current terminal's user program */
        uint32_t physaddr = (PDE_PROCESS_START + terminalArray[globalpcb->termid].currprocessid) * FOUR_MB;
        map_helper(PDE_VIRTUAL_MEM, physaddr);
        tss.esp0 = EIGHT_MEGA_BYTE - EIGHT_KILO_BYTE * globalpcb->pid;
        /* (b) Set TSS for parent. ksp = kernel stack pointer */
        uint32_t args_esp = globalpcb->saved_esp;
        uint32_t args_ebp = globalpcb->saved_ebp;
        send_eoi(PIT_IRQ); //after the asm?
        sti();
        asm volatile(
            /* set esp, ebp as esp ebp args */
            "   movl %0, %%esp \n"
            "   movl %1, %%ebp \n"
            :
            : "r"(args_esp), "r"(args_ebp) // input
            : "cc"                         // ?
        );
        
        
    }
    
    


    return;

}


// void scheduler()
// {
    
//     /* Obtain the PCB of the new terminal - */
//     pcb *sched_pcb = terminalArray[/* New terminal*/].cur_PCB;
//     pcb *prev_pcb = terminalArray[/* New terminal*/].cur_PCB;

//     /* Check if the base shell has started yet*/

//     if (terminalArray[terminalrun].cur_PCB == NULL)
//     {
//         /* NOT STARTED
//         initialize all values, set the flag to 1.
//         copy in the ESP, EBP into the PCB
//         adjust pcb to terminal pointers to each other
//         switch to a new terminal
//         send the EOI
//         execute the shell
//         } */

//         /* Initialize all values, set the flag to 1. */
//         if (terminalrunning == 1)
//             term_2_flag = 1;
//         if (terminalrunning == 2)
//             term_3_flag = 1;

//         /* Copy in the ESP, EBP into the PCB */
//         uint32_t save_esp = 0;
//         uint32_t save_ebp = 0;
//         asm volatile(
//             "movl %%ebp, %0; \n"
//             "movl %%esp, %1; \n"
//             : "=g"(save_ebp), "=g"(save_esp)
//             :);
//         // sched_pcb->saved_esp = save_esp;
//         // sched_pcb->saved_ebp = save_ebp;
//         terminalArray[terminalrunning].savedt_esp = save_esp;
//         terminalArray[terminalrunning].savedt_esp = save_esp;

//         /* Adjust pcb and terminal pointers to each other */
//         terminalArray[terminalrunning].cur_pcb = sched_pcb; // checked
//         pcb.terminalnum = terminalrunning;

//         /* Switch to a new terminal */
//         // terminal_switch(terminalrunning);
//         currpid++;

//         /* Send the EOI */
//         send_eoi(PIT_IRQ);

//         /* Execute the shell */
//         execute("shell");
//     }


//     /* OTHERWISE IT EXISTS, DO DIFF STUFF WITHOUT EXECUTING
//      save esp, ebp
//      change current terminal
//          ?? might have to see if that is also not initialized (pit interrupr #2, 3)
//          ?? if not initialized we must recall the scheduler so we send eoi and call pit handler again
//      change video memory / then the terminal video mapping
//      now we change user page as well for the new process
//      do context switch:
//          1. tss.esp0 = 8MB - 8KB*newprocessid
//      change esp, ebp of new process
//      send end of interrupt
//      ^^ do we switch order of these both?? eoi stuff
//      */

//     /* Save esp, ebp */
//     uint32_t save_esp = 0;
//     uint32_t save_ebp = 0;
//     asm volatile(
//         "movl %%ebp, %0; \n"
//         "movl %%esp, %1; \n"
//         : "=g"(save_ebp), "=g"(save_esp)
//         :);
//     // sched_pcb.saved_esp = save_esp;
//     // sched_pcb.saved_ebp = save_ebp;

//     terminalArray[terminalrunning].savedt_esp = save_esp;
//     terminalArray[terminalrunning].savedt_ebp = save_ebp;

//     /* change current terminal
//         ?? might have to see if that is also not initialized (pit interrupr #2, 3)
//         ?? if not initialized we must recall the scheduler so we send eoi and call pit handler again
//     */

//     terminalrunning = ((terminalrunning + 1) % 3);

//     if ((terminalrunning == 1 && term_2_flag == 0) || (terminalrunning == 2 && term_3_flag == 0))
//     {
//         send_eoi(PIT_IRQ);
//         return;
//     }
//     // TODO remap the terminals ??
//     // TODO remap the vidmem

// // T2 -> T0
// //tr = 0
    
//     /* Find new process's PCB so we can now do context switch */
//     pcb_t *nextprocesspcb = terminalArray[terminalrunning].curr_pcb;

//     // TODO remap the user prog using new pcb

//     tss.esp0 = 8MB - 8KB * nextprocesspcb.processid;

//     send_eoi(PIT_IRQ);

//     /* (b) Set TSS for parent. ksp = kernel stack pointer */
//     uint32_t args_esp = nextprocesspcb->saved_esp;
//     uint32_t args_ebp = nextprocesspcb->saved_ebp;

//     asm volatile(

//         /* set esp, ebp as esp ebp args */
//         "   pushl %0 \n"
//         "   pushl %1 \n"
//         "   popl %%esp          \n"
//         "   popl %%ebp          \n"
//         :
//         : "r"(args_esp), "r"(args_ebp) // input
//         : "cc"                         // ?
//     );
//     return;
// }