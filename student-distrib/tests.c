#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"

#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* IDT Test 
 * Inputs: None
 * Outputs: FAIL if expection doesnt happen and passes if idt print message shown 
 * Side Effects: causes exception from divide by 0 
 * Coverage: Load IDT, IDT definition
 */
// add more tests here
int divzTest() {
	int result = FAIL;
	int i;
	i = 0;
	int j; 
	j = 3; //just a test val 
	int k;

	k = j/i;

	return result;
}

/* sysCallTest
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
//testing system call #INT(x80) 
int sysCallTest(){
	int result = FAIL;

	asm volatile ("INT $0x80"); // x80 addr for system call port 
	return result;
}

/* pageFaultTest
 * Inputs: None
 * Outputs: page fault or shows pass if valid 
 * Side Effects: None
 * Coverage: paging 
 */
int pageFaultTest() {
	int result = PASS; 
	int * testval;
	//outside of vid mem( 0xb8000 ->0xb9000), kernel mem(0x400000 -> 0x800000)
	testval = (int *)0xb7999; 			// Out of range - FAULT - prints message of page fault on console
	//testval = (int *)0xb8001; 		// Inside Range - PASS - shows as pass in test output
	// testval = (int *)0xb8FFF;		// Inside Range - PASS - shows as pass in test output
	// testval = (int *)0x3FFFFF;		// Out of range - FAULT - prints message of page fault on console
	// testval = (int *)0x400000;		// Inside Range - PASS - shows as pass in test output
	// testval = (int *)0x800001;		// Out of range - FAULT - prints message of page fault on console
	// testval = (int *)0x0;		// null check Out of range - FAULT - prints message of page fault on console
	*testval = 3; //3 is random val to test 
	return result;
}


/* Checkpoint 2 tests */
int testFilesys(){
	// const uint8_t testfname[34] = "verylargetextwithverylongname.txt";
	const uint8_t testfname[11] = "frame0.txt";
	dentry_t testdir;
	int32_t fd;

	int numb = -2;
	numb = read_dentry_by_name(testfname, (dentry_t *)(&testdir));
	// read_dir(int32_t fd, void* buf, int32_t nbytes) {
	// Z:\mp3\fsdir\verylargetextwithverylongname.txt
	// printf("reach 113");
	// printf("%u",testdir.ftype);
	// printf("%s", testdir.filename);
	uint8_t buf[187];
	read_data(testdir.inode, 0, buf, 187);
	clear();
	set_screen_x(0);
	set_screen_y(0);
	terminal_write(fd, buf, 187);
	// read_data(testdir.inode, 50, buf, 187);
	// read_data(testdir.inode, 184, buf, 187);
	// read_data(testdir.inode, 189, buf, 187);
	int i = 0;
	// for (i = 139; i<163; i++) {
	// 	printf("%c", buf[i]);
	//  }
	// printf("num bytes = %u", numb);
	//printf("%s", buf);

	return 0;
	
}

int testFileDrivers(){
	clear();
	set_screen_x(0);
	set_screen_y(0);
	int i = 0, j = 0;

	uint32_t fd_temp;

	open_dir((uint8_t *)(".")); // open the directory

	for(i = 0; i<17;i++){
		int8_t curfname[32];
		read_dir(fd_temp,curfname,32);
		printf("FILE NAME: ");

		for(j = 0; j <32; j++){
			printf("%c",curfname[j]);
		}

		printf("\n");
	}

	return 0;
}


int testRTC(){
	clear();
	set_screen_x(0);
	set_screen_y(0);
	uint32_t fd_temp;
	
	
	int i = 0, j = 0;
	// int32_t frequency = 1024;
	int32_t frequency = 128;
	// int32_t frequency = 16;
	// int32_t frequency = 2;
	// int32_t frequency = 1000;
	// int32_t frequency = 1;
	// int32_t frequency = 5;
	// read_rtc(fd_temp, &(frequency), 32);
	write_rtc(fd_temp, &(frequency), 32);
	
	
	return 0;
	
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests() {
	// launch your tests here 
	/* Checkpoint 1 */
	// 	TEST_OUTPUT("idt_test", idt_test());						// Given IDT Test
	//	TEST_OUTPUT("divz_test", divzTest());					// Divide by 0 test
	//	TEST_OUTPUT("Page Fault Test", pageFaultTest());			// Page Fault Test
	//	TEST_OUTPUT("System Call Test", sysCallTest());				// System Call Test
	// Our RTC Test is checked through rtc.c where we call test_interrupts() to check frequency.
	/* Checkpoint 2 */
	//TEST_OUTPUT("file sys test", testFilesys());				//
	//testFilesys();
	//testFileDrivers();
	testRTC();
}


