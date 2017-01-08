AVR Code Execution Control Package
====================================

Both packages should be quite device independent even across the different families (tiny, mega and Xmega).
While implementing, NO hard assumtions were made how the compiler arranges code and/or registers - it therefore should work across different versions of avr-gcc.


EXTFUNC
=======
EXTFUNC contains interfaces and macros to extend function pointers accross the 128k limit.
In AVR-gcc pointers are 16bit by default. This means only a fraction of flash storage can be used for code on bigger AVRs.
Althought these AVRs most likely implement extension registers ("EIND" and "RAMP*"), handling them is complex and NOT reentrant - it forces rethinking of the code construct...
EXTFUNC is not based on "EIND". Instead it pushes function call as forged return adresses to stack and then calls them via "ret". (Therefor "EIND" based code can coexist in parallel.)

The code provided by EXTFUNC generalizes function-calls and should be used instead the common C definitions.
(In situations where the extended code is not necessary (because the flash is small enough?), these macros unfold in the usual C code and constitude no overhead then.
This behaviour also can be enfored (via "EXTFUNC_NOEXT") and vice versa.)
A more detailed "how to do..." can be found in the beginning of "extfunc.h".

EXTFUNC also allows to use two more sections of code (".nearfunc" and ".farfunc").
The EXTFUNC functions then are automatically placed in ".farfunc", leaving the precious lower 128k memory to other functions which they might need it.


by Stephan Baerwolf, Schwansee 2017




CPUCONTEXT
=========
CPUCONTEXTs allows you to have multiple statemachines existing in coexistence, without having them decomposed and merged.
It does this by abstracting the whole CPU-state in data types, which then can be addressed and switch between. It provides multiple independet execution paths within one single AVR cpu.
Without additional work, an operative (aka non-preemtive) scheduling-scheme can be implemented.

Because CPUCONTEXT itself does not bring any synchronization against interrupts it is fast and minimizes blocking influences.
Please see head of cpucontext.h fore more details.


by Stephan Baerwolf, Schwansee 2017
