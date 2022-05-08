/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
.data;	\
	.long name;	\
	.long num;	\
.text;	\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
.data;	\
	.long name;	\
	.long num;	\
.text;	\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.data
	.globl trapvectors
	trapvectors:

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(divide_handler, T_DIVIDE)
TRAPHANDLER_NOEC(debug_handler, T_DEBUG)
TRAPHANDLER_NOEC(nmi_handler, T_NMI)
TRAPHANDLER_NOEC(brkpt_handler, T_BRKPT)
TRAPHANDLER_NOEC(overflow_handler, T_OFLOW)
TRAPHANDLER_NOEC(bounds_handler, T_BOUND)
TRAPHANDLER_NOEC(illegalop_handler, T_ILLOP)
TRAPHANDLER_NOEC(device_handler, T_DEVICE)

TRAPHANDLER(double_handler, T_DBLFLT)
TRAPHANDLER(taskswitch_handler, T_TSS)
TRAPHANDLER(segment_handler, T_SEGNP)
TRAPHANDLER(stack_handler, T_STACK)
TRAPHANDLER(protection_handler, T_GPFLT)
TRAPHANDLER(page_handler, T_PGFLT)

TRAPHANDLER_NOEC(floating_handler, T_FPERR)
TRAPHANDLER_NOEC(aligment_handler, T_ALIGN)
TRAPHANDLER_NOEC(machine_handler, T_MCHK)
TRAPHANDLER_NOEC(simd_handler, T_SIMDERR)
TRAPHANDLER_NOEC(syscall_handler, T_SYSCALL)
TRAPHANDLER_NOEC(default_handler, T_DEFAULT)

TRAPHANDLER_NOEC(IRQ0_handler, IRQ_OFFSET + 0)
TRAPHANDLER_NOEC(IRQ1_handler, IRQ_OFFSET + 1)
TRAPHANDLER_NOEC(IRQ2_handler, IRQ_OFFSET + 2)
TRAPHANDLER_NOEC(IRQ3_handler, IRQ_OFFSET + 3)
TRAPHANDLER_NOEC(IRQ4_handler, IRQ_OFFSET + 4)
TRAPHANDLER_NOEC(IRQ5_handler, IRQ_OFFSET + 5)
TRAPHANDLER_NOEC(IRQ6_handler, IRQ_OFFSET + 6)
TRAPHANDLER_NOEC(IRQ7_handler, IRQ_OFFSET + 7)
TRAPHANDLER_NOEC(IRQ8_handler, IRQ_OFFSET + 8)
TRAPHANDLER_NOEC(IRQ9_handler, IRQ_OFFSET + 9)
TRAPHANDLER_NOEC(IRQ10_handler, IRQ_OFFSET + 10)
TRAPHANDLER_NOEC(IRQ11_handler, IRQ_OFFSET + 11)
TRAPHANDLER_NOEC(IRQ12_handler, IRQ_OFFSET + 12)
TRAPHANDLER_NOEC(IRQ13_handler, IRQ_OFFSET + 13)
TRAPHANDLER_NOEC(IRQ14_handler, IRQ_OFFSET + 14)
TRAPHANDLER_NOEC(IRQ15_handler, IRQ_OFFSET + 15)

/*
 * Lab 3: Your code here for _alltraps
 */

.text
.globl _alltraps
_alltraps:
    pushl %ds
	pushl %es
	pushal
    movw $GD_KD, %ax	
    movw %ax, %ds	
    movw %ax, %es
    pushl %esp
	call trap
