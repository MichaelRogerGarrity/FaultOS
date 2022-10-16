

int (*funcs[256]) ();

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

void init_IDT();
