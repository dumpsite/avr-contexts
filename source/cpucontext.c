/*
 * CPUCONTEXT.C
 * This is version 20170107T1200ZSB
 *
 * 
 * Stephan Baerwolf (matrixstorm@gmx.de), Schwansee 2017
 * (please contact me at least before commercial use)
 */

#define CPUCONTEXT_C_946ec1db093e4a5680ae7b4f75dca09c 1

#include "cpucontext.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <avr/cpufunc.h>

static cpucontext_t	__cpucontext_main_context = {.stack=(void*)RAMEND};
cpucontext_t*   cpucontext_main_context = &__cpucontext_main_context;
cpucontext_t*   cpucontext_idle_context = &__cpucontext_main_context;
cpucontext_t*	cpucontext_current	= NULL;

EXTFUNC_void(int8_t, cpucontext_initialize) {
  if (cpucontext_current == NULL) {
    /* cpucontext not initialized before */
    cpucontext_main_context = &__cpucontext_main_context;
    cpucontext_idle_context = &__cpucontext_main_context;
    cpucontext_main_context->flags	= _BV(CPUCONTEXT_FLAGBM_initialized) | _BV(CPUCONTEXT_FLAGBM_current) | _BV(CPUCONTEXT_FLAGBM_dirty);
    cpucontext_main_context->stack	= (void*)((SPH >> 8) | SPL);
    cpucontext_current			= cpucontext_main_context;
    return 1;
  }
  return 0;
}

EXTFUNC_void(int8_t, cpucontext_finalize) {
  if (cpucontext_current) {
    cpucontext_current=NULL;
    return 1;
  }
  return 0;
}



#define __cpuswtch_flagoldreg	"r20"
#define __cpuswtch_flagreg	"r21"

#define __cpuswtch_splreg	"r22"
#define __cpuswtch_sphreg	"r23"

#define __cpuswtch_bckreg	"r24"
#define __cpuswtch_bckregh	"r25"
EXTFUNCFAR __attribute__((noinline,used,naked,weak)) void __cpucontext_switch(void) {
  /* expects current context in "X"-reg and nextcontext in "Z"-reg */
  asm volatile (
#if (CPUCONTEXT_EXTRASYMBOLS)
"__cpucontext_switch_corebackup%=: \n\t"
#endif
    "push	r0							\n\t"
    "push	r1							\n\t"
    "push	r2							\n\t"
    "push	r3							\n\t"
    "push	r4							\n\t"
    "push	r5							\n\t"
    "push	r6							\n\t"
    "push	r7							\n\t"
    "push	r8							\n\t"
    "push	r9							\n\t"
    "push	r10							\n\t"
    "push	r11							\n\t"
    "push	r12							\n\t"
    "push	r13							\n\t"
    "push	r14							\n\t"
    "push	r15							\n\t"

    "push	r16							\n\t"
    "push	r17							\n\t"
    "push	r18							\n\t"
    "push	r19							\n\t"
    "push	r20							\n\t"
    "push	r21							\n\t"
    "push	r22							\n\t"
    "push	r23							\n\t"
    "push	r24							\n\t"
    "push	r25							\n\t"
//     "push	r26							\n\t"
//     "push	r27							\n\t"
    "push	r28							\n\t"
    "push	r29							\n\t"
//     "push	r30							\n\t"
//     "push	r31							\n\t"


#if (CPUCONTEXT_EXTRASYMBOLS)
"__cpucontext_switch_iobackup%=: \n\t"
#endif
    /* start saving some IO-registers   : SREG, RAMPX, RAMPY, RAMPZ, RAMPD and EIND */
    "lds	"__cpuswtch_bckreg"	,	%[__sreg]		\n\t"
    "push	"__cpuswtch_bckreg"					\n\t"
#ifdef RAMPX
    "lds	"__cpuswtch_bckreg"	,	%[__rampx]		\n\t"
    "push	"__cpuswtch_bckreg"					\n\t"
#endif
#ifdef RAMPY
    "lds	"__cpuswtch_bckreg"	,	%[__rampy]		\n\t"
    "push	"__cpuswtch_bckreg"					\n\t"
#endif
#ifdef RAMPZ
    "lds	"__cpuswtch_bckreg"	,	%[__rampz]		\n\t"
    "push	"__cpuswtch_bckreg"					\n\t"
#endif
#ifdef RAMPD
    "lds	"__cpuswtch_bckreg"	,	%[__rampd]		\n\t"
    "push	"__cpuswtch_bckreg"					\n\t"
#endif
#ifdef EIND
    "lds	"__cpuswtch_bckreg"	,	%[__eind]		\n\t"
    "push	"__cpuswtch_bckreg"					\n\t"
#endif

#if (CPUCONTEXT_EXTRASYMBOLS)
"__cpucontext_switch_stackswitch%=: \n\t"
#endif
    /* save current stackpointer and process flags of current context (X) */
#ifdef SPH
    "lds	"__cpuswtch_sphreg"	,	%[__sph]		\n\t"
#else
    "clr	"__cpuswtch_sphreg"					\n\t"
#endif
    "lds	"__cpuswtch_splreg"	,	%[__spl]		\n\t"
    "ld		"__cpuswtch_flagreg"	,	X			\n\t"
    "andi	"__cpuswtch_flagreg"	,	%[__flgandoff]		\n\t"
    "ori	"__cpuswtch_flagreg"	,	%[__flgoroff]		\n\t"    
    "st		X+			,	"__cpuswtch_flagreg"	\n\t"
    "st		X+			,	"__cpuswtch_splreg"	\n\t"
    "st		X+			,	"__cpuswtch_sphreg"	\n\t"
    "sbiw	r26			,	3			\n\t"   /* repair X pointer */

    /* basically we can switch now - so load flags and stackpointer of tocontext (Z) */
    "ld		"__cpuswtch_flagreg"	,	Z			\n\t"
    "mov	"__cpuswtch_flagoldreg"	,	"__cpuswtch_flagreg"	\n\t"	/* perhap we need to know how original flags looked like */
    "andi	"__cpuswtch_flagreg"	,	%[__flgandon]		\n\t"
    "ori	"__cpuswtch_flagreg"	,	%[__flgoron]		\n\t"    
    "st		Z+			,	"__cpuswtch_flagreg"	\n\t"
    "ld		"__cpuswtch_splreg"	,	Z+			\n\t"
    "ld		"__cpuswtch_sphreg"	,	Z+			\n\t"
    "sbiw	r30			,	3			\n\t"   /* repair Z pointer */
    
    /*  **SWITCH**	(accessing SPL will disable interrupts for 4 cycles */
    "sts	%[__spl]		,	"__cpuswtch_splreg"	\n\t"
    "sts	%[__sph]		,	"__cpuswtch_sphreg"	\n\t"

#if 0
#if (CPUCONTEXT_EXTRASYMBOLS)
"__cpucontext_switch_contextswap%=: \n\t"
#endif
    //swap X and Z
    /* backup "tocontext" register Z, as it will become new current (X)   */
    "movw	"__cpuswtch_bckreg"	,	r30			\n\t"
    /* return last running context into "tocontext" - X-reg becomes Z-reg */
    "movw	r30			,	r26			\n\t"
    /* prepare new "current" (X), which was "tocontext" before            */
    "movw	r26			,	"__cpuswtch_bckreg"	\n\t"
    /*                        ** SWITCH DONE **                           */
#endif

    "nop \n\t"
    "nop \n\t"

#if (CPUCONTEXT_EXTRASYMBOLS)
"__cpucontext_switch_iorestore%=: \n\t"
#endif
    /* start restoring some IO-registers: SREG, RAMPX, RAMPY, RAMPZ, RAMPD and EIND */
#ifdef EIND
    "pop	"__cpuswtch_bckreg"					\n\t"
    "sts	%[__eind]		,	"__cpuswtch_bckreg"	\n\t"
#endif
#ifdef RAMPD
    "pop	"__cpuswtch_bckreg"					\n\t"
    "sts	%[__rampd]		,	"__cpuswtch_bckreg"	\n\t"
#endif
#ifdef RAMPZ
    "pop	"__cpuswtch_bckreg"					\n\t"
    "sts	%[__rampz]		,	"__cpuswtch_bckreg"	\n\t"
#endif
#ifdef RAMPY
    "pop	"__cpuswtch_bckreg"					\n\t"
    "sts	%[__rampy]		,	"__cpuswtch_bckreg"	\n\t"
#endif
#ifdef RAMPX
    "pop	"__cpuswtch_bckreg"					\n\t"
    "sts	%[__rampx]		,	"__cpuswtch_bckreg"	\n\t"
#endif
    "pop	"__cpuswtch_bckreg"					\n\t"
    "sts	%[__sreg]		,	"__cpuswtch_bckreg"	\n\t"


#if (CPUCONTEXT_EXTRASYMBOLS)
"__cpucontext_switch_corerestore%=: \n\t"
#endif
//     "pop	r31							\n\t"
//     "pop	r30							\n\t"
    "pop	r29							\n\t"
    "pop	r28							\n\t"
//     "pop	r27							\n\t"
//     "pop	r26							\n\t"
    "pop	r25							\n\t"
    "pop	r24							\n\t"
    "pop	r23							\n\t"
    "pop	r22							\n\t"
    "pop	r21							\n\t"
    "pop	r20							\n\t"
    "pop	r19							\n\t"
    "pop	r18							\n\t"
    "pop	r17							\n\t"
    "pop	r16							\n\t"

    "pop	r15							\n\t"
    "pop	r14							\n\t"
    "pop	r13							\n\t"
    "pop	r12							\n\t"
    "pop	r11							\n\t"
    "pop	r10							\n\t"
    "pop	r9							\n\t"
    "pop	r8							\n\t"
    "pop	r7							\n\t"
    "pop	r6							\n\t"
    "pop	r5							\n\t"
    "pop	r4							\n\t"
    "pop	r3							\n\t"
    "pop	r2							\n\t"
    "pop	r1							\n\t"
    "pop	r0							\n\t"
#if (CPUCONTEXT_EXTRASYMBOLS)
"__cpucontext_switch_end%=: \n\t"
#endif
    "ret								\n\t"
      : 
      : [__flgandoff]	"M"	((~(_BV(CPUCONTEXT_FLAGBM_current) | _BV(CPUCONTEXT_FLAGBM_dirty) | _BV(CPUCONTEXT_FLAGBM_virgin))) & 0xff),
	[__flgoroff]	"M"	(( (_BV(CPUCONTEXT_FLAGBM_notcurrent)                                                            )) & 0xff),
	[__flgandon]	"M"	((~(_BV(CPUCONTEXT_FLAGBM_notcurrent)                                                            )) & 0xff),
	[__flgoron]	"M"	(( (_BV(CPUCONTEXT_FLAGBM_current) | _BV(CPUCONTEXT_FLAGBM_dirty)                                )) & 0xff),
	[__flgvirgin]	"M"	(( (_BV(CPUCONTEXT_FLAGBM_virgin)                                                                )) & 0xff),
	[__sreg]	"i"	(_SFR_MEM_ADDR(SREG)),
#ifdef RAMPX
	[__rampx]	"i"	(_SFR_MEM_ADDR(RAMPX)),
#endif
#ifdef RAMPY
	[__rampy]	"i"	(_SFR_MEM_ADDR(RAMPY)),
#endif
#ifdef RAMPZ
	[__rampz]	"i"	(_SFR_MEM_ADDR(RAMPZ)),
#endif
#ifdef RAMPD
	[__rampd]	"i"	(_SFR_MEM_ADDR(RAMPD)),
#endif
#ifdef EIND
	[__eind]	"i"	(_SFR_MEM_ADDR(EIND)),
#endif
#ifdef SPH
	[__sph]		"i"	(_SFR_MEM_ADDR(SPH)),
#endif
	[__spl]		"i"	(_SFR_MEM_ADDR(SPL))
    );
}


__attribute__ ((weak))
EXTFUNC(cpucontext_t*, cpucontext_switch, cpucontext_t* tocontext) {
  if (cpucontext_current != tocontext) {
    cpucontext_t* __previous			= cpucontext_current;
    cpucontext_current				= tocontext;
    cpucontext_current->previous_running	= __previous;
    asm volatile (
#if (FLASHEND < 8192)
      "rcall __cpucontext_switch					\n\t"
#else
      "call __cpucontext_switch						\n\t"
#endif
	: [curr]		"+x"	(__previous),
	  [toctx]		"+z"	(tocontext)
	:
	: "memory"
      );
    _MemoryBarrier();
    return cpucontext_current->previous_running; /* do not use __previous - use the barrior */
  }
  return cpucontext_current;
}


__attribute__ ((naked, weak))
EXTFUNCFAR __attribute__((noinline,used)) void __cpucontext_start(void) {
  _MemoryBarrier();

  /* start up users entry point */
  cpucontext_current->exitcode=EXTFUNC_callptr(cpucontext_current->entrypoint, CPUCONTEXT_entry_t, cpucontext_current->entryparams);

  _MemoryBarrier();

  /* mark context as finalized */
  cpucontext_current->flags|=_BV(CPUCONTEXT_FLAGBM_finalized);

  /* do some other stuff */

  
  /* never leave or return from stack - the context is marked as finalized, better don't switch to it anymore */
  while (1) {
    EXTFUNC_callByName(cpucontext_switch, cpucontext_idle_context);
  }
}

EXTFUNC(int8_t, cpucontext_create,cpucontext_t* context, void* stack, size_t stacksize, EXTFUNC_functype(CPUCONTEXT_entry_t) entry, void* entryparams) {
  uint8_t *buffer = stack;
  uint16_t extptr;

  memset(stack, 0, stacksize);
  context->stack=(void*)&buffer[stacksize-sizeof(cpucontext_stack_t)];
#ifdef EIND
  context->stack->eind=EIND;
#endif
#ifdef RAMPD
  context->stack->rampd=RAMPD;
#endif
#ifdef RAMPZ
  context->stack->rampz=RAMPZ;
#endif
#ifdef RAMPY
  context->stack->rampy=RAMPY;
#endif
#ifdef RAMPX
  context->stack->rampx=RAMPX;
#endif
  context->stack->sreg=SREG;
  extptr=(uint16_t)&__cpucontext_start;
#if (FLASHEND > 131071)
  context->stack->returnptr_msb=__EXTFUNC__builtin_avr_flash_segment(__cpucontext_start);
#endif
#if (FLASHEND > 511)
  context->stack->returnptr_med=extptr>>8;
#endif
  context->stack->returnptr_lsb=extptr&0xff;
  
  context->entrypoint=entry;
  context->entryparams=entryparams;
  context->flags=_BV(CPUCONTEXT_FLAGBM_initialized) | _BV(CPUCONTEXT_FLAGBM_virgin) | _BV(CPUCONTEXT_FLAGBM_notcurrent);
  return 0;
}
