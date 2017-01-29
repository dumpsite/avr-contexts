/*
 * HWCLOCK.C
 * This is version 20170129T1700ZSB
 *
 *
 * Stephan Baerwolf (matrixstorm@gmx.de), Schwansee 2017
 * (please contact me at least before commercial use)
 */

#define HWCLOCK_C_436d4b4cebb1467bb67eca7bbee5bb2e 1

#include "hwclock.h"

#include "extfunc.h"

static uint8_t __hwclock_initialized = 0;


EXTFUNCFAR __attribute__((noinline,used,weak)) void __hwclock_timer_init(void) {
}

EXTFUNCFAR __attribute__((noinline,used,weak)) void __hwclock_timer_start(void) {
}

EXTFUNCFAR __attribute__((noinline,used,weak)) void __hwclock_timer_stop(void) {
}

EXTFUNCFAR __attribute__((noinline,used,weak)) void __hwclock_timer_final(void) {
}



EXTFUNC_void(int8_t, hwclock_initialize) {
  __hwclock_initialized++;
  if (__hwclock_initialized == 1) {
    __hwclock_timer_init();
    __hwclock_timer_start();
    return 0;
  }
  return 1;
}

EXTFUNC_void(int8_t, hwclock_finalize) {
  if (__hwclock_initialized > 0) {
    __hwclock_initialized--;
    if (__hwclock_initialized == 0) {
      __hwclock_timer_stop();
      __hwclock_timer_final();
      return 0;
    }
    return (-1);
  }
  return 1;
}

#if ((HWCLOCK_MSBTIMER_USEIOACCESS) > 0)
#	define __HWCMSBIN	"in"
#else
#	define __HWCMSBIN	"lds"
#endif

#if ((HWCLOCK_LSBTIMER_USEIOACCESS) > 0)
#	define __HWCLSBIN	"in"
#else
#	define __HWCLSBIN	"lds"
#endif

#if ((HWCLOCK_MSBTIMER_BITS) > 8)
#	define __HWCLKTMP	"r2"
#endif


EXTFUNC_void(hwclock_time_t, hwclock_now) {
  hwclock_time_t result;
  asm volatile (
    "hwclock_now_seqlock_again%=:			\n\t"
#if ((HWCLOCK_MSBTIMER_BITS) > 0)
    __HWCMSBIN"	%[_b2]		,	%[tcnthl]	\n\t"
#	if ((HWCLOCK_MSBTIMER_BITS) > 8)
    __HWCMSBIN" %[_b3]		,	%[tcnthh]	\n\t"
#	endif
#endif

    __HWCLSBIN" %[_b0]		,	%[tcntll]	\n\t"
#if ((HWCLOCK_LSBTIMER_BITS) > 8)
    __HWCLSBIN" %[_b1]		,	%[tcntlh]	\n\t"
#endif

#if ((HWCLOCK_MSBTIMER_BITS) > 0)
    __HWCMSBIN"	__tmp_reg__	,	%[tcnthl]	\n\t"
#	if ((HWCLOCK_MSBTIMER_BITS) > 8)
    __HWCMSBIN" "__HWCLKTMP"	,	%[tcnthh]	\n\t"
    "cp		__tmp_reg__	,	%[_b2]		\n\t"
    "cpc	"__HWCLKTMP"	,	%[_b3]		\n\t"
    "brne hwclock_now_seqlock_again%=			\n\t"
#	else
    "cpse	__tmp_reg__	,	%[_b2]		\n\t"
    "rjmp hwclock_now_seqlock_again%=			\n\t"
#	endif
#endif
    :
#if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 24)
      [_b3]		"=r"	(result._b[3]),
#endif
#if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 16)
      [_b2]		"=r"	(result._b[2]),
#endif
#if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) >  8)
      [_b1]		"=r"	(result._b[1]),
#endif
      [_b0]		"=r"	(result._b[0])

    :
#if ((HWCLOCK_MSBTIMER_BITS) > 0)
#	if ((HWCLOCK_MSBTIMER_USEIOACCESS) > 0)
#		if ((HWCLOCK_MSBTIMER_BITS) > 8)
      [tcnthh]		"i"	(_SFR_IO_ADDR(HWCLOCK_MSBTIMER_VALUEREG_HIGH)),
#		endif
      [tcnthl]		"i"	(_SFR_IO_ADDR(HWCLOCK_MSBTIMER_VALUEREG_LOW)),
#	else
#		if ((HWCLOCK_MSBTIMER_BITS) > 8)
      [tcnthh]		"i"	(_SFR_MEM_ADDR(HWCLOCK_MSBTIMER_VALUEREG_HIGH)),
#		endif
      [tcnthl]		"i"	(_SFR_MEM_ADDR(HWCLOCK_MSBTIMER_VALUEREG_LOW)),
#	endif
#endif

#if ((HWCLOCK_LSBTIMER_USEIOACCESS) > 0)
#	if ((HWCLOCK_LSBTIMER_BITS) > 8)
      [tcntlh]		"i"	(_SFR_IO_ADDR(HWCLOCK_LSBTIMER_VALUEREG_HIGH)),
#	endif
      [tcntll]		"i"	(_SFR_IO_ADDR(HWCLOCK_LSBTIMER_VALUEREG_LOW))
#else
#	if ((HWCLOCK_LSBTIMER_BITS) > 8)
      [tcntlh]		"i"	(_SFR_MEM_ADDR(HWCLOCK_LSBTIMER_VALUEREG_HIGH)),
#	endif
      [tcntll]		"i"	(_SFR_MEM_ADDR(HWCLOCK_LSBTIMER_VALUEREG_LOW))
#endif

    :
#if ((HWCLOCK_MSBTIMER_BITS) > 8)
      __HWCLKTMP,
#endif
      "memory"
   );
  return result;
}

EXTFUNC(hwclock_time_t, hwclock_delta, const hwclock_time_t earlier, hwclock_time_t later) {
#if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 24)
  asm volatile (
    "sub	%[_a0]		,	%[_b0]		\n\t"
    "sbc	%[_a1]		,	%[_b1]		\n\t"
    "sbc	%[_a2]		,	%[_b2]		\n\t"
    "sbc	%[_a3]		,	%[_b3]		\n\t"
    "sbrs	%[_a3]		,	7		\n\t"
    "rjmp hwclock_delta_finish%=			\n\t"
    "neg	%[_a3]					\n\t"
    "neg	%[_a2]					\n\t"
    "neg	%[_a1]					\n\t"
    "neg	%[_a0]					\n\t"
    "sec						\n\t"
    "adc	%[_a0],		__zero_reg__		\n\t"
    "adc	%[_a1],		__zero_reg__		\n\t"
    "adc	%[_a2],		__zero_reg__		\n\t"
    "adc	%[_a3],		__zero_reg__		\n\t"
    "hwclock_delta_finish%=:				\n\t"
    : [_a0]		"+r"	(later._b[0]),
      [_a1]		"+r"	(later._b[1]),
      [_a2]		"+r"	(later._b[2]),
      [_a3]		"+r"	(later._b[3])
    : [_b0]		"r"	(earlier._b[0]),
      [_b1]		"r"	(earlier._b[1]),
      [_b2]		"r"	(earlier._b[2]),
      [_b3]		"r"	(earlier._b[3])
    : "memory"
   );
#elif (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 16)
  asm volatile (
    "sub	%[_a0]		,	%[_b0]		\n\t"
    "sbc	%[_a1]		,	%[_b1]		\n\t"
    "sbc	%[_a2]		,	%[_b2]		\n\t"
    "sbrs	%[_a2]		,	7		\n\t"
    "rjmp hwclock_delta_finish%=			\n\t"
    "neg	%[_a2]					\n\t"
    "neg	%[_a1]					\n\t"
    "neg	%[_a0]					\n\t"
    "sec						\n\t"
    "adc	%[_a0],		__zero_reg__		\n\t"
    "adc	%[_a1],		__zero_reg__		\n\t"
    "adc	%[_a2],		__zero_reg__		\n\t"
    "hwclock_delta_finish%=:				\n\t"
    : [_a0]		"+r"	(later._b[0]),
      [_a1]		"+r"	(later._b[1]),
      [_a2]		"+r"	(later._b[2])
    : [_b0]		"r"	(earlier._b[0]),
      [_b1]		"r"	(earlier._b[1]),
      [_b2]		"r"	(earlier._b[2])
    : "memory"
   );
#elif (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 8)
  asm volatile (
    "sub	%[_a0]		,	%[_b0]		\n\t"
    "sbc	%[_a1]		,	%[_b1]		\n\t"
    "sbrs	%[_a1]		,	7		\n\t"
    "rjmp hwclock_delta_finish%=			\n\t"
    "neg	%[_a1]					\n\t"
    "neg	%[_a0]					\n\t"
    "sec						\n\t"
    "adc	%[_a0],		__zero_reg__		\n\t"
    "adc	%[_a1],		__zero_reg__		\n\t"
    "hwclock_delta_finish%=:				\n\t"
    : [_a0]		"+r"	(later._b[0]),
      [_a1]		"+r"	(later._b[1])
    : [_b0]		"r"	(earlier._b[0]),
      [_b1]		"r"	(earlier._b[1])
    : "memory"
   );
#else
  asm volatile (
    "sub	%[_a0]		,	%[_b0]		\n\t"
    "sbrs	%[_a0]		,	7		\n\t"
    "rjmp hwclock_delta_finish%=			\n\t"
    "neg	%[_a0]					\n\t"
    "sec						\n\t"
    "adc	%[_a0],		__zero_reg__		\n\t"
    "hwclock_delta_finish%=:				\n\t"
    : [_a0]		"+r"	(later._b[0])
    : [_b0]		"r"	(earlier._b[0])
    : "memory"
   );
#endif
  return later;
}

EXTFUNC(uint32_t, hwclock_tickspassed, const hwclock_time_t earlier, hwclock_time_t later) {
#if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) < 25)
  uint32_t result=0;
  hwclock_time_t *temp=(void*)&result;
  (*temp) = EXTFUNC_callByName(hwclock_delta, earlier, later);
  return result;
#else
  /* work around internal compiler error: in make_decl_rtl, at varasm.c:1147 */
  hwclock_time_t result = EXTFUNC_callByName(hwclock_delta, earlier, later);
  return result.value;
#endif
}

EXTFUNC(bool, hwclock_ispassed, const hwclock_time_t timedelta, uint16_t passticks) {
#if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 24)
  return ((timedelta.upperval > 0) || (timedelta.lowerval>=passticks));
#elif (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 16)
  return ((timedelta.msbval > 0) || (timedelta.value>=passticks));
#else
  return (timedelta.value>=passticks);
#endif
}

EXTFUNC(int8_t, hwclock_spinwait, const uint16_t ticks, EXTFUNC_functype(hwclock_waitcallback_t) waitcallback, void* userparameter) {
  int8_t result = 0;
  hwclock_time_t the_since, the_now, the_delta;
  the_since=EXTFUNC_callByName(hwclock_now);
  do {
    the_now=EXTFUNC_callByName(hwclock_now);
    the_delta=EXTFUNC_callByName(hwclock_delta, the_since, the_now);
    if (EXTFUNC_callByName(hwclock_ispassed, the_delta, ticks)) {
      break;
    } else if (!EXTFUNC_isNULL(waitcallback)) {
      if ((result=EXTFUNC_callptr(waitcallback, hwclock_waitcallback_t, the_now, the_since, the_delta, ticks, userparameter))<0) break;
    }
  } while(true);
  return result;
}

