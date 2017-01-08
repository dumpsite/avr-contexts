/*
 * EXTFUNC.C
 * 
 * This is version 20170107T1200ZSB
 *
 * Stephan Baerwolf (matrixstorm@gmx.de), Schwansee 2017
 * (please contact me at least before commercial use)
 */

#define EXTFUNC_C_5dd52e4c69e34e8bb1670a1691f01e78 	1

#include "extfunc.h"
#include <stdlib.h>
#include <avr/io.h>

#ifdef __EXTFUNC_EIND
uint8_t __extfunc_EIND_default = 0xff;
#endif

#if (EXTFUNC_NOEXT)
#else
/* to be called indirectly - further optimizable (cycles at sts and use "in") --> weak symbol */
__attribute__ ((naked, weak))
void __extfunc_trampolineCallTarget(extfuncptr_t *__extfunc_self) {

#ifdef __EXTFUNC_SpecialTrampolineCode
#	if ((FLASHEND > 131071) && (defined(SPH)))
#		define __extfunc_trampolineCallTarget__CodeChoiceSpecial	1
#	else
#		define __extfunc_trampolineCallTarget__CodeChoiceSpecial	0
#	endif
#else
#	define __extfunc_trampolineCallTarget__CodeChoiceSpecial		0
#endif
  
#if (__extfunc_trampolineCallTarget__CodeChoiceSpecial)
  /* special function more optimized for speed, not working on all AVRs (but most of them can use "EXTFUNC_NOEXT" anyways then) */
  asm volatile (
    /* pre allocate space for returning address */
    "push	__zero_reg__					\n\t"
    "push	__zero_reg__					\n\t"
    "push	__zero_reg__					\n\t"
    "push	r26						\n\t"
    "push	r27						\n\t"
    "push	__tmp_reg__					\n\t"
    /* swap r25:r24 most likely %[selfaddr]) with r29:r28 - use X as tmp (3 cycles) */
    "movw	r26		,	%A[selfaddr]		\n\t"
    "movw	%A[selfaddr]	,	r28			\n\t"
    "movw	r28		,	r26			\n\t"
    /* Y now points to self */


    /* let X reg point to stack and is 4, 5 or 6 byte off original ptr */ 
    "in		r27		,	%[stackptrh]		\n\t"
    "in		r26		,	%[stackptrl]		\n\t"
    /* skip last 3 pushes and point to reserved return address */
    "adiw	r26		,	4			\n\t"


    /* load the target address of function to call ... */
    "adiw	r28		,	2			\n\t"

    "ld		__tmp_reg__	,	Y			\n\t"
    "st		X+		,	__tmp_reg__		\n\t"	
    "ld		__tmp_reg__	,	-Y			\n\t"
    "st		X+		,	__tmp_reg__		\n\t"	
    "ld		__tmp_reg__	,	-Y			\n\t"
    "st		X		,	__tmp_reg__		\n\t"	

    "pop	__tmp_reg__					\n\t"
    /* restore Y, r25:r24 (most likely %[selfaddr]) and X */
    /* swap r25:r24 (most likely %[selfaddr]) with r29:r28 - use X as tmp (3 cycles) */
    "movw	r26		,	%A[selfaddr]		\n\t"
    "movw	%A[selfaddr]	,	r28			\n\t"
    "movw	r28		,	r26			\n\t"
    /* pop remaining registers now */
    "pop	r27						\n\t"
    "pop	r26						\n\t"


    /* goon with the funny buisness and return to target function */
    "ret							\n\t"
      :
      : [selfaddr]	"r"	(__extfunc_self),
    /* AVR with more than 128KiB flash can be assumed to use 16bit stackpointer */
    /* all known AVRs map their stack pointer within IO-register-range          */
	[stackptrh]	"i"	(_SFR_IO_ADDR(SPH)),
	[stackptrl]	"i"	(_SFR_IO_ADDR(SPL))
    );
#else

  /* gerneral purpose function working on every AVR, but slower */
  asm volatile (
    /* pre allocate space for returning address */
    "push	__zero_reg__					\n\t"
#if		(FLASHEND > 511)
    "push	__zero_reg__					\n\t"
#	if	(FLASHEND > 131071)
    "push	__zero_reg__					\n\t"
#	endif
#endif
    "push	r26						\n\t"
    "push	r27						\n\t"
    "push	__tmp_reg__					\n\t"
    /* swap r25:r24 most likely %[selfaddr]) with r29:r28 - use X as tmp (3 cycles) */
    "movw	r26		,	%A[selfaddr]		\n\t"
    "movw	%A[selfaddr]	,	r28			\n\t"
    "movw	r28		,	r26			\n\t"
    /* Y now points to self */


    /* let X reg point to stack and is 4, 5 or 6 byte off original ptr */ 
#ifdef SPH
    "lds	r27		,	%[stackptrh]		\n\t"
#else
    "clr	r27						\n\t"
#endif
    "lds	r26		,	%[stackptrl]		\n\t"
    /* skip last 3 pushes and point to reserved return address */
    "adiw	r26		,	4			\n\t"


    /* load the target address of function to call ... */
#if	(FLASHEND > 131071)
    "adiw	r28		,	3			\n\t"
#else
#	if	(FLASHEND > 511)
    "adiw	r28		,	2			\n\t"
#	else
    "adiw	r28		,	1			\n\t"
#	endif
#endif

    "ld		__tmp_reg__	,	-Y			\n\t"
    "st		X+		,	__tmp_reg__		\n\t"	
#if		(FLASHEND > 511)
    "ld		__tmp_reg__	,	-Y			\n\t"
    "st		X+		,	__tmp_reg__		\n\t"	
#	if	(FLASHEND > 131071)
    "ld		__tmp_reg__	,	-Y			\n\t"
    "st		X+		,	__tmp_reg__		\n\t"	
#	endif
#endif

    "pop	__tmp_reg__					\n\t"
    /* restore Y, r25:r24 (most likely %[selfaddr]) and X */
    /* swap r25:r24 (most likely %[selfaddr]) with r29:r28 - use X as tmp (3 cycles) */
    "movw	r26		,	%A[selfaddr]		\n\t"
    "movw	%A[selfaddr]	,	r28			\n\t"
    "movw	r28		,	r26			\n\t"
    /* pop remaining registers now */
    "pop	r27						\n\t"
    "pop	r26						\n\t"


    /* goon with the funny buisness and return to target function */
    "ret							\n\t"
      :
      : [selfaddr]	"r"	(__extfunc_self),
#ifdef SPH
	[stackptrh]	"i"	(_SFR_MEM_ADDR(SPH)),
#endif
	[stackptrl]	"i"	(_SFR_MEM_ADDR(SPL))
    );
#endif
}
#endif


int8_t extfunc_initialize(void) {
#ifdef __EXTFUNC_EIND  
  if (__extfunc_EIND_default==0xff) {
    __extfunc_EIND_default=__EXTFUNC_EIND;
#else
  if (1) {
#endif
    return 0;
  }
  return 1;
}

int8_t extfunc_finalize(void) {
  return 0;
}

bool extfunc_PTRisNULL(extfuncptr_t ptr) {
#if (EXTFUNC_NOEXT)
  return (ptr==NULL);
#else
  return ((ptr.lower128k == 0) && (ptr.upperextension == 0));
#endif
}
void extfunc_setNULL(extfuncptr_t *ptr) {
#if (EXTFUNC_NOEXT)
  (*ptr)=NULL;
#else
  ptr->lower128k	= 0;
  ptr->upperextension	= 0;
#endif
}
