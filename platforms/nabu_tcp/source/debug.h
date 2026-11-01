#ifndef DEBUG_H
#define DEBUG_H

/*
 * debug_printf: print a message from a NABU program into the emulator's console, like SMS_debugPrintf in
 * devkitSMS/Emulicious.  Marduk (the build from source, with the debug trap) prints it as "[NABU] <message>" in the
 * terminal it was started from.  On a real NABU, or an emulator without the trap, nothing is printed and nothing breaks.
 *
 * To use it, copy debug.h and debug.c into your project, then:
 *
 *   #include "debug.h"
 *   debug_printf("x=%d hex=%x\n", x, y);
 *
 * debug_printf is a macro, not a function: it formats with snprintf into a buffer (DEBUG_BUFSIZE bytes, messages
 * longer than that are cut off), then debug_puts hands the finished string to the emulator.  A function wrapper
 * using va_start/vsnprintf did not work with z88dk's sccz80 compiler.
 *
 * How the emulator is told: debug_puts runs "ld d,d", a no-op on a real Z80 that Marduk watches for.  It is followed
 * by "jr" over the bytes 0x64 0x64 (the signature) and 0x0100 (print the string at HL).  This is the same layout as
 * devkitSMS's trap, with our own selector.
 */

#include <stdio.h>

#ifndef DEBUG_BUFSIZE
#define DEBUG_BUFSIZE 128
#endif

/* Prints a finished NUL-terminated string (passed in HL). */
extern void debug_puts(const char *s) __z88dk_fastcall;

/* Shared message buffer, defined in debug.c. */
extern char debug_buf[DEBUG_BUFSIZE];

#define debug_printf(...) \
    do { \
        snprintf(debug_buf, DEBUG_BUFSIZE, __VA_ARGS__); \
        debug_puts(debug_buf); \
    } while (0)

#endif
