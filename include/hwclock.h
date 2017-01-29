/*
 * HWCLOCK.H
 * This is version 20170129T1700ZSB
 *
 * This file includes forward definitions and macros to
 * implement an interruptless hardware clock based on timers.
 *
 * Two timer-hardwares can be interconnected to form a wider
 * time-window-area.
 * Therefore the LSB timer is configured as "toggle-before-overflow"
 * PWM and used as triggerinput of the MSB timer.
 *
 * Ususally time is tracked by configuring some timer
 * overflow interrupt. There a global static number is
 * incremented every overflow - a.k.a. "tick".
 * In order to save the interrupt: The timer just
 * increments its TCNT register. This register is read
 * by the "main thread".
 * In order to increase resolution from 16bit to 24bit
 * (and therefore increase the total timedifferences
 * measurable), timer0 is externally connected to it.
 * To do this, timer0's external trigger "T0" is
 * used.
 * Timer1 is configured in fast PWM mode, generating
 * LOW the first 32768 (prescaled) ticks on OC1B.
 * The remaining 32768 ticks (of an full period) it
 * signals HIGH. When it overflows it falls back to LOW,
 * generating an HIGH-LOW edge. This is used to tigger
 * timer0 to increment its TCNT0.
 * Seqlocked reading both TCNT1 and TCNT0 contains the
 * systems time.
 * Becareful of overflows! The maximum detectable time-
 * difference is (2^23)-1  ---  NOT 2^24 or (2^24)-1 .
 *
 * THIS NEEDS an externa electrical connection between
 * "OC1B" and "T0".
 *
 * Stephan Baerwolf (matrixstorm@gmx.de), Schwansee 2017
 * (please contact me at least before commercial use)
 */

#ifndef HWCLOCK_H_436d4b4cebb1467bb67eca7bbee5bb2e
#define HWCLOCK_H_436d4b4cebb1467bb67eca7bbee5bb2e 1

#ifdef HWCLOCKINCLUDEDEFINES
#	include "defines.h"
#endif
#include "hwclockconfig.h"

#include "extfunc.h"

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>

#if (HWCLOCK_MSBTIMER_BITS > HWCLOCK_LSBTIMER_BITS)
#	error "The timer with most bits needs to be lowest significant!"
#endif

#ifdef HWCLOCK_C_436d4b4cebb1467bb67eca7bbee5bb2e
#	define HWCLOCKPUBLIC
#else
#	define HWCLOCKPUBLIC	extern
#endif

#if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 24)
struct __hwclock_time_t {
  union {
        uint8_t  _b[4];
	uint32_t value;
	struct {
	  uint16_t lowerval;
	  uint16_t upperval;
	} __attribute__((packed));
  } __attribute__((packed));
} __attribute__((packed));
#else
#	if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 16)
struct __hwclock_time_t {
  union {
        uint8_t  _b[3];
	struct {
	  uint16_t value;
	  uint8_t  msbval;
	} __attribute__((packed));
  } __attribute__((packed));
} __attribute__((packed));
#	else
#		if (((HWCLOCK_MSBTIMER_BITS)+(HWCLOCK_LSBTIMER_BITS)) > 8)
struct __hwclock_time_t {
  union {
        uint8_t  _b[2];
	uint16_t value;
  } __attribute__((packed));
} __attribute__((packed));
#		else
struct __hwclock_time_t {
  union {
        uint8_t  _b[1];
	uint8_t  value;
  } __attribute__((packed));
} __attribute__((packed));
#		endif
#	endif
#endif

typedef struct __hwclock_time_t hwclock_time_t;
EXTFUNC_typedef(int8_t, hwclock_waitcallback_t, const hwclock_time_t the_now, const hwclock_time_t waiting_since, const hwclock_time_t the_delta, const uint16_t waiting_ticks, void* userparameter);


#ifndef __HWCLOCK_NSPERTICK
#	define __HWCLOCK_NSPERTICK (1000ULL)
#endif


/*
 * Macro for rouding division (math).
 * This Macro divides x by y and rounds the result.
 */
#define __HWCLOCK_CEILDIV(x,y)	(((x)%(y))?(((x)/(y))+1):((x)/(y)))

/*
 * Macro for calculating the microseconds to Hardware-Ticks.
 */
#define HWCLOCK_UStoTICK(x)	(__HWCLOCK_CEILDIV((x*1000ULL), __HWCLOCK_NSPERTICK))

/**
 * Macro for calculating the Hardware-Ticks to microseconds.
 */
#define HWCLOCK_TICKtoUS(x)	(__HWCLOCK_CEILDIV((x*__HWCLOCK_NSPERTICK), 1000ULL))



/* int8_t hwclock_initialize(void)
 *
 * This function will startup the library and the timer.
 * It internally calls the weak symbols "__hwclock_timer_init()"
 * and afterwards "__hwclock_timer_startup()".
 * This can be overridden by main program and be used to configure
 * the specific hardware timers.
 */
HWCLOCKPUBLIC EXTFUNC_voidhead(int8_t, hwclock_initialize);

/* int8_t hwclock_finalize(void)
 *
 * This function prepares the end of this library.
 * It internally calls the weak symbols "__hwclock_timer_stop()"
 * and afterwards "__hwclock_timer_final()".
 * This can be overridden by main program and be used to restore
 * original settings of the specific hardware timers.
 */
HWCLOCKPUBLIC EXTFUNC_voidhead(int8_t, hwclock_finalize);

/*
 * Gets the current time in an atomic fashion.
 * When returning the function result the time may be in the recent history.
 */
HWCLOCKPUBLIC EXTFUNC_voidhead(hwclock_time_t, hwclock_now);

/*
 * Calculates the time differential of two input times.
 * When returning the function result the deltatime is returned as new time.
 */
HWCLOCKPUBLIC EXTFUNC_head(hwclock_time_t, hwclock_delta, const hwclock_time_t earlier, hwclock_time_t later);

/*
 * Calculates the time differential in ticks of two input times.
 * Synonym as hwclock_delta for Hardware-Ticks instead of microseconds.
 */
HWCLOCKPUBLIC EXTFUNC_head(uint32_t, hwclock_tickspassed, const hwclock_time_t earlier, hwclock_time_t later);

/*
 * Check a given timedelta, if it has passed a given time.
 * This function is faster than checks using "hwclock_tickspassed".
 * Returns true if timedelta has passed the given time
 */
HWCLOCKPUBLIC EXTFUNC_head(bool, hwclock_ispassed, const hwclock_time_t timedelta, uint16_t passticks);

/*
 * Runs a limited loop while performing (maybe) a function.
 */
HWCLOCKPUBLIC EXTFUNC_head(int8_t, hwclock_spinwait, const uint16_t ticks, EXTFUNC_functype(hwclock_waitcallback_t) waitcallback, void* userparameter);

#endif