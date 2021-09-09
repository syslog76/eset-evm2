#pragma once

enum evm2_op_code
{	      
	mov,      // 000      mov arg1, arg2                   arg2 <- arg1
	load_const,// 001      loadConst constant, arg1         arg1 < -constant
	add,      // 010001   add arg1, arg2, arg3             arg3 < -arg1 + arg2
	sub,      // 010010   sub arg1, arg2, arg3             arg3 < -arg1 - arg2
	divide,   // 010011   div arg1, arg2, arg3             arg3 <- arg1 / arg2
	mod,      // 010100   mod arg1, arg2, arg3             arg3 <- arg1 % arg2
	mul,      // 010101   mul arg1, arg2, arg3             arg3 <- arg1 * arg2
	compare,  // 01100    compare arg1, arg2, arg3         arg3 <- -1 if arg1 < arg2
	          //                                           arg3 <-  0 if arg1 == arg2
	          //                                           arg3 <-  1 if arg1 > arg2
    jump_address,// 01101    jump address
	jump_equal,// 01110    jumpEqual address, arg1, arg2    Move instruction pointer to address if arg1 == arg2
	read,     // 10000    read arg1, arg2, arg3, arg4
	          //                                           Read from binary input file using
		      //                                           arg1 – offset in input file
		      //                                           arg2 – number of bytes to read
		      //                                           arg3 – memory address to which read bytes will be stored
		      //                                           After read operation, arg4 receives amount of bytes actually
		      //                                           read – may be less than arg2, if not enough data exists in
		      //                                           input file.
	write,    // 10001 write arg1, arg2, arg3              Write to binary output file using
		      //                                           arg1 – offset in output file
		      //                                           arg2 – number of bytes to write
		      //                                           arg3 – memory address from which bytes will be written
		      //                                           If requested offset is larger than current output file size, file
		      //                                           should be padded with zeroes.
	con_read, // 10010 consoleRead arg1                 Read hexadecimal value from console and store to arg1
	con_write,// 10011 consoleWrite arg1                Write arg1 to console, as hexadecimal value.
	thread_create,// 10100 createThread address arg1        Create a new thread, starting at address and store it’s
              //                                           identifier to arg1. New thread starts with copy of current
	          //                                           thread’s registers.
	thread_join,//10101 joinThread arg1                     Wait till thread identified using arg1 ends
		      //                                           and dispose its state.
		      //                                           Threads will only be joined once.
	halt,     // 10110 hlt                                 End current thread. If initial thread is ended,
		      //                                           end whole program.
	sleep,    // 10111 sleep arg1                          Delay execution of current thread by arg1 milliseconds.
	call,     // 1100 call address                         Store address of instruction after the call
		      //                                           to internal stack and continue execution at address.
	ret,      // 1101 ret                                  Take address from internal stack and continue execution from it.
	lock,     // 1110 lock arg1                            lock synchronization object identified by arg1.
	unlock,   // 1111 unlock arg1                          Unlock synchronization object identified by arg1.

	ukn01011, // unimplemented 01011 instruction
	ukn01111, // unimplemented 01111 instruction
	ukn010000,// unimplemented 010000 instruction
	padding,  // end of instruction stream detected
	stopped   // task is stopped (pseudo-instruction)
};