/*
 * CPUCONTEXT.H
 * This is version 20170107T1200ZSB
 *
 * Advances usual AVR libc firmwares with contexts of execution.
 * THIS IS NOT AN PREEMTIVE SCHEDULER/DISPATCHER, althought it
 * implements a core for this: A task switcher.
 * 
 * CPUCONTEXTs allows you to have multiple statemachines existing
 * in coexistence, without having them decomposed and merged.
 * It does this by abstracting the whole CPU-state in a data type.
 * You can simply switch between these types to switch the active
 * (aka running in cpu) code (execution path).
 * Without additional work, an operative (aka non-preemtive) scheduling-
 * scheme can be implemented.
 * Because CPUCONTEXT itself does not bring any synchronization
 * against interrupts it is fast and minimizes blocking influences.
 * 
 * Important notes:
 * ================
 * - library must be initilaized for abstraction of primary execution path
 * 
 * - all functions DON'T implement ANY synchronization against interrupts
 * - free of any active interrupts disable (except SPL write in "switch(...)")
 * - interrupts are allow to be on, they just must not use calls into this library without extra synchronization
 * - all calls into this library are fully interruptable at all times
 * 
 * - each context tracks its own "SREG" and therefor the interrupt en-/disable state
 * - interrupts have NO own context, they occure in the currently active one
 * - each context must have enough stack for possible interrupts to occure
 * - an interrupt could happen in the middle of "switch()" an so registers are saved twice
 *   --> plan at least 2*40=80 byte for stack per context !
 * 
 * 
 * Stephan Baerwolf (matrixstorm@gmx.de), Schwansee 2017
 * (please contact me at least before commercial use)
 */

#ifndef CPUCONTEXT_H_946ec1db093e4a5680ae7b4f75dca09c
#define CPUCONTEXT_H_946ec1db093e4a5680ae7b4f75dca09c 1

#ifdef CPUCONTEXTINCLUDEDEFINES
#	include "defines.h"
#endif

#include "extfunc.h"

#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/cpufunc.h>


#ifdef CPUCONTEXT_C_946ec1db093e4a5680ae7b4f75dca09c
#	define CPUCONTEXTPUBLIC
#else
#	define CPUCONTEXTPUBLIC	extern
#endif

#ifndef CPUCONTEXT_EXTRASYMBOLS
#	define	CPUCONTEXT_EXTRASYMBOLS		0
#endif

/* workaround some gcc behaviour causing avr-libc macro to fail */
#if ((defined(CPUCONTEXT_REPLACEMEMBARRIER)) || (__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ <= 7)))
#	ifdef _MemoryBarrier
#		undef _MemoryBarrier
#	endif
#	define _MemoryBarrier(x)	__asm__ __volatile__("":::"memory")
#endif

/* access to specific register should be done via macros if at all */
typedef struct __cpucontext_stack_t {
  volatile uint8_t	empty_nextbyte;
#ifdef EIND
  volatile uint8_t	eind;
#endif
#ifdef RAMPD
  volatile uint8_t	rampd;
#endif
#ifdef RAMPZ
  volatile uint8_t	rampz;
#endif
#ifdef RAMPY
  volatile uint8_t	rampy;
#endif
#ifdef RAMPX
  volatile uint8_t	rampx;
#endif
  volatile uint8_t	sreg;

//   volatile uint8_t	r31;
//   volatile uint8_t	r30;
  volatile uint8_t	r29;
  volatile uint8_t	r28;
//   volatile uint8_t	r27;
//   volatile uint8_t	r26;
  volatile uint8_t	r25;
  volatile uint8_t	r24;
  volatile uint8_t	r23;
  volatile uint8_t	r22;
  volatile uint8_t	r21;
  volatile uint8_t	r20;
  
  volatile uint8_t	r19;
  volatile uint8_t	r18;
  volatile uint8_t	r17;
  volatile uint8_t	r16;
#ifndef CONFIG_CPUCONTEXT_NO_R15
  volatile uint8_t	r15;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R14
  volatile uint8_t	r14;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R13
  volatile uint8_t	r13;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R12
  volatile uint8_t	r12;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R11
  volatile uint8_t	r11;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R10
  volatile uint8_t	r10;
#endif

#ifndef CONFIG_CPUCONTEXT_NO_R9
  volatile uint8_t	r9;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R8
  volatile uint8_t	r8;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R7
  volatile uint8_t	r7;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R6
  volatile uint8_t	r6;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R5
  volatile uint8_t	r5;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R4
  volatile uint8_t	r4;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R3
  volatile uint8_t	r3;
#endif
#ifndef CONFIG_CPUCONTEXT_NO_R2
  volatile uint8_t	r2;
#endif
  volatile uint8_t	r1;
  volatile uint8_t	r0;

#if	(FLASHEND > 131071)
  volatile uint8_t	returnptr_msb;
#endif
#if	(FLASHEND > 511)
  volatile uint8_t	returnptr_med;
#endif
  volatile uint8_t	returnptr_lsb;
} __attribute__ ((packed)) cpucontext_stack_t;



/* defining the function-type of the entry-function a new context is started with */
EXTFUNC_typedef(int8_t, CPUCONTEXT_entry_t, void* parameters);


#define	CPUCONTEXT_FLAGBM_initialized	0 /* when created successful */
#define	CPUCONTEXT_FLAGBM_finalized	1 /* when finished running entry-function */
#define CPUCONTEXT_FLAGBM_usr1		2
#define CPUCONTEXT_FLAGBM_usr2		3
#define	CPUCONTEXT_FLAGBM_virgin	4 /* when never before a switch was made (it is the first time running) */
#define CPUCONTEXT_FLAGBM_notcurrent	5
#define	CPUCONTEXT_FLAGBM_current	6 /* when currently running */
#define	CPUCONTEXT_FLAGBM_dirty		7 /* when conext-information is not accessable */
typedef struct __cpucontext_t cpucontext_t;
struct __cpucontext_t {
  volatile uint8_t			flags;
  volatile cpucontext_stack_t*		stack;

  cpucontext_t*				previous_running;

  void*					entryparams;
  union {
    struct {
      EXTFUNC_functype(CPUCONTEXT_entry_t)	entrypoint;
    };
    struct {
      int8_t					exitcode; /* only valid when CPUCONTEXT_FLAGBM_finalized */
      uint8_t					reserved;
    };
  };
} __attribute__ ((packed));


/* the primary context after initializing the library */
CPUCONTEXTPUBLIC cpucontext_t*	cpucontext_main_context;
CPUCONTEXTPUBLIC cpucontext_t*	cpucontext_idle_context;
CPUCONTEXTPUBLIC cpucontext_t*	cpucontext_current;


/* int8_t cpucontext_initialize(void) */
CPUCONTEXTPUBLIC EXTFUNC_voidhead(int8_t, cpucontext_initialize);

/* int8_t cpucontext_finalize(void) */
CPUCONTEXTPUBLIC EXTFUNC_voidhead(int8_t, cpucontext_finalize);

/* create a new context entering extfunction "entry" but not running it */
CPUCONTEXTPUBLIC EXTFUNC_head(int8_t, cpucontext_create,cpucontext_t* context, void* stack, size_t stacksize, EXTFUNC_functype(CPUCONTEXT_entry_t) entry, void* entryparams);

/* switch to another context of execution - returns pointer to active context before */
CPUCONTEXTPUBLIC EXTFUNC_head(cpucontext_t*, cpucontext_switch, cpucontext_t* tocontext);

/* checks if a context has expired */
#define CPUCONTEXT_isFinal(ctx)		(((ctx)->flags) & _BV(CPUCONTEXT_FLAGBM_finalized))

/* checks if a context is dirty - meaning its stack data is NOT valid  */
#define CPUCONTEXT_isDirty(ctx)		(((ctx)->flags) & _BV(CPUCONTEXT_FLAGBM_dirty))


#endif
