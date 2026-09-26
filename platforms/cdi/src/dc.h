#ifndef __DC_H__
#define __DC_H__

/*
 * Prototypes for the CD-i SDK's dc_* (SS_DC / Display Control) functions.
 * These are real, linkable routines in cdi.l/cdisys.l (see funcs.a's SS_DC
 * opcode table), but the SDK ships no header declaring them anywhere.
 * Signatures below are reverse-engineered from every call site in this
 * project (video.c, graphics.c, title.c, intro.c, nob_game.c, main.c) --
 * not from official documentation, which doesn't exist for these.
 * Return types are assumed int (OS-9 status-code convention) except where
 * a call site assigns the result to a typed variable, confirming it.
 */

/* Field Control Table (FCT) */
int dc_crfct(int path, int plane, int size, int flags);                              /* Create Field Control Table -> FCT id */
int dc_dlfct(int path, int fct);                                                     /* Delete Field Control Table */
int dc_wrfct(int path, int fct, int offset, int count, int* buffer);                 /* Write Field Control Table (bulk instruction write) */
int dc_flnk(int path, int fct, int lct, int flags);                                  /* Link a Line Control Table into a Field Control Table */

/* Line Control Table (LCT) */
int dc_crlct(int path, int plane, int size, int flags);                              /* Create Line Control Table -> LCT id */
int dc_dllct(int path, int lct);                                                     /* Delete Line Control Table */
int dc_wrli(int path, int lct, int line, int slot, int instruction);                 /* Write one instruction at (line, slot) */
int dc_rdli(int path, int lct, int line, int slot);                                  /* Read back one instruction at (line, slot) */
int dc_wrlct(int path, int lct, int line, int col, int numLines, int numCols, int* buffer); /* Bulk-write instructions from buffer over a (line,col) range */
int dc_nop(int path, int lct, int line, int col, int numLines, int numCols);         /* Fill a (line,col) range with NOP instructions */
int dc_llnk(int path, int lct, int line, int targetLct, int targetLine);             /* Link one LCT's line to another LCT's line */

/* Display control / signal */
int dc_ssig(int path, int sigCode, int flags);                                       /* Arm one-shot signal delivery on next video interrupt (_OP_SIG hit) */
int dc_exec(int path, int fctA, int fctB);                                           /* Start executing the display program for both planes */
int dc_setcmp(int path, int flag);                                                   /* Set compatibility bit in VSC */
int dc_intl(int path, int flag);                                                     /* Enable/disable interlace */

#endif
