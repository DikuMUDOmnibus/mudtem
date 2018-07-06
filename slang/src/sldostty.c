/* Copyright (c) 1992, 1999 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include <dos.h>

#if defined (__EMX__)
#  define int86		_int86
#  define delay		_sleep2
#endif	/* __EMX__ */
#if defined (__WATCOMC__)
#  include <conio.h>
#  include <bios.h>
#  define int86	int386
#else
#  define _NKEYBRD_READ		0x0
#  define _NKEYBRD_READY	0x1
#  define _NKEYBRD_SHIFTSTATUS	0x2
#endif	/* __WATCOMC__ */

#define BIOSKEY		slbioskey
#if defined(__WATCOMC__)
# define keyWaiting() _bios_keybrd(_NKEYBRD_READY)
#else
# define keyWaiting()	BIOSKEY(_NKEYBRD_READY)
#endif

#include "slang.h"
#include "_slang.h"

#ifdef __cplusplus
# define _DOTS_ ...
#else
# define _DOTS_ void
#endif

#if !defined (__EMX__) && !defined (__GO32__) && !defined (__WATCOMC__)
#define HAS_INT9
#endif

#ifdef __GO32__
# include <signal.h>
#endif

#if defined (HAS_INT9)
static void interrupt (*int9_old) (_DOTS_);
static unsigned char far *shift = (unsigned char far *) 0x417;
static unsigned int Abort_Scan_Code = 34;  /* 34 = scan code for ^G */

/*----------------------------------------------------------------------*\
 * an interrupt 9 handler, not for use with most 32 bit compilers
\*----------------------------------------------------------------------*/
static void interrupt int9_handler (_DOTS_)
{
   unsigned char s, s1;

   s1 = *shift & 0xF;		/* ignore caps, ins, num lock, scroll lock */
   s = inp (0x60);
   if (s1 & 0x04)		/* control key */
     {
	if (s == Abort_Scan_Code)
	  {
	     if (SLang_Ignore_User_Abort == 0) SLang_Error = SL_USER_BREAK;
	     SLKeyBoard_Quit = 1;
	  }
     }
   (*int9_old) ();
}
#endif	/* HAS_INT9 */

static void int9_change (int set)
{
#if defined (HAS_INT9)
   if (set)			/* install a new handler */
     {
	if (int9_old != NULL) return;
	int9_old = getvect (9);
	setvect (9, int9_handler);
     }
   else	if (int9_old != NULL)	/* restore the old handler */
     {
	setvect (9, int9_old);
	int9_old = NULL;
     }
#else
   (void) set;
#endif	/* HAS_INT9 */
}

/*----------------------------------------------------------------------*\
 *  Function:	static void set_ctrl_break (int state);
 *
 * set the control-break setting
\*----------------------------------------------------------------------*/
static void set_ctrl_break (int state)
{
#if defined (__EMX__)
   (void) state;		/* not really required */
#else	/* __EMX__ */

   static int prev = 0;

# if defined (__GO32__)
   if (state == 0)
     {
#  if __DJGPP__ >= 2
	signal (SIGINT, SIG_IGN);
#  endif
	prev = getcbrk ();
	setcbrk (0);
     }
   else
     {
#  if __DJGPP__ >= 2
	signal (SIGINT, SIG_DFL);
#  endif
	setcbrk (prev);
     }
# else	/* __GO32__ */
#  if defined(__WATCOMC__)
   fprintf (stderr, "Have not yet defined set_ctrl_break for __WATCOMC__\n");
   prev = state;
#  else
   asm  mov dl, byte ptr prev
     asm  mov ax, state
     asm  cmp ax, 0
     asm  jne L1
     asm  mov ah, 33h
     asm  mov al, 0
     asm  mov dl, byte ptr prev
     asm  int 21h
     asm  xor ax, ax
     asm  mov al, dl
     asm  mov prev, ax
     asm  mov dl, 0
     L1:
   asm  mov al, 1
     asm  mov ah, 33h
     asm  int 21h
#  endif	/* __WATCOMC__ */
# endif	/* __GO32__ */
#endif	/* __EMX__ */
}

/*----------------------------------------------------------------------*\
 * static unsigned int slbioskey (int op);
 *
 * op 0-2 (standard) and 0x10-0x12 (extended) are valid
 *
 * 0, 0x10	_NKEYBRD_READ	- read the key
 * 1, 0x11	_NKEYBRD_READY	- check if a key is waiting
 * 		if so give a peek of its value, otherwise return 0
 * 2, 0x12	_NKEYBRD_SHIFTSTATUS	- get shift flags
 *		(Ins, Cap, Num, Scroll, Alt, ^Ctrl L_shift, R_shift)
 *		flags = ICNSA^LR	only the lower byte is valid!
\*----------------------------------------------------------------------*/
static int bios_key_f = 0;
static unsigned int slbioskey (int op)
{
   union REGS r;
   r.h.ah = (op & 0x03) | bios_key_f;
   int86 (0x16, &r, &r);
#if defined(__WATCOMC__)
   /* return (_bios_keybrd ((op & 0x03) | bios_key_f)); */
# if 1			/* the correct zero flag for watcom? */
   /* is zero flag set? (no key waiting) */
   if ((op & _NKEYBRD_READY) && (r.x.cflag & 0x40) == 0x40) return 0;
# else			/* the correct zero flag for watcom? */
   /* is zero flag set? (no key waiting) */
   if ((op & _NKEYBRD_READY) && (r.x.cflag & 0x4)) return 0;
# endif
   return (r.x.eax & 0xffff);
#else
   /* is zero flag set? (no key waiting) */
   if (op & _NKEYBRD_READY)
     {
	if ((r.x.flags & 0x40) == 0x40)
	  return 0;
	if (r.x.ax == 0)		       /* CTRL-BREAK */
	  return -1;
     }
   return (r.x.ax & 0xffff);
#endif
}

/*----------------------------------------------------------------------*\
 *  Function:	int SLang_init_tty (int abort_char, int no_flow_control,
 *				    int opost);
 *
 * initialize the keyboard interface and attempt to set-up the interrupt 9
 * handler if ABORT_CHAR is non-zero.
 * NO_FLOW_CONTROL and OPOST are only for compatiblity and are ignored.
\*----------------------------------------------------------------------*/
int SLang_init_tty (int abort_char, int no_flow_control, int opost)
{
   (void) no_flow_control;
   (void) opost;

   bios_key_f = 0x10;		/* assume it's an enhanced keyboard */
#if defined (HAS_INT9)
   bios_key_f &= peekb (0x40,0x96);	/* verify it's true */
   if (abort_char > 0) Abort_Scan_Code = (unsigned int) abort_char;
#else
   (void) abort_char;
#endif

   set_ctrl_break (0);
   /* clear keyboard buffer */
   /* while (keyWaiting()) BIOSKEY(_NKEYBRD_READ); */

   return 0;
}

/*----------------------------------------------------------------------*\
 *  Function:	void SLang_reset_tty (void);
 *
 * reset the tty before exiting
\*----------------------------------------------------------------------*/
void SLang_reset_tty (void)
{
   int9_change (0);
   set_ctrl_break (1);
}

/*----------------------------------------------------------------------*\
 *  Function:	int _SLsys_input_pending (int tsecs);
 *
 *  sleep for *tsecs tenths of a sec waiting for input
\*----------------------------------------------------------------------*/
int _SLsys_input_pending (int tsecs)
{
   if (keyWaiting()) return 1;

   /* Convert tsecs to units of 20 ms */
   tsecs = tsecs * 5;

   /* If tsecs is less than 0, it represents millisecs */
   if (tsecs < 0)
     tsecs = -tsecs / 100;

   while (tsecs > 0)
     {
	delay (20);	/* 20 ms or 1/50 sec */
	if (keyWaiting()) break;
	tsecs--;
     }
   return (tsecs);
}

/*----------------------------------------------------------------------*\
 *  Function:	unsigned int _SLsys_getkey (void);
 *
 * wait for and get the next available keystroke.
 * Also re-maps some useful keystrokes.
 *
 *	Backspace (^H)	=>	Del (127)
 *	Ctrl-Space	=>	^@	(^@^3 - a pc NUL char)
 *	extended keys are prefixed by a null character
\*----------------------------------------------------------------------*/
unsigned int _SLsys_getkey (void)
{
   unsigned int key, scan, ch, shift;

#if 0
   int tsecs = SL_input_time;	/* tsecs = 300 */
   if ((SLgetkey_hook != NULL) && !keyWaiting())
     {
	while (!_SLsys_input_pending(tsecs))
	  getkey_hook ();
     }
#endif

   key  = BIOSKEY(_NKEYBRD_READ);
   ch   = key & 0xff;
   scan = key >> 8;
   shift = BIOSKEY(_NKEYBRD_SHIFTSTATUS) & 0xf;

   if (key == 0x0e08)
     return 127;			/* Backspace key */

   switch (ch)
     {
      case 32:
	if (0 == (shift & 0x04))
	  break;
	/* ^space = ^@ */
	scan = 3;		/* send back Ctrl-@ => ^@^C */
	/* drop */
      case 0xe0:
      case 0:			/* extended key code */
	ch = _SLpc_convert_scancode (scan);
     }
   return (ch);
}

/*----------------------------------------------------------------------*\
 *  Function:	void SLang_set_abort_signal (void (*handler)(int));
\*----------------------------------------------------------------------*/
int SLang_set_abort_signal (void (*handler)(int))
{
   if (handler == NULL) int9_change (1);
   return 0;
}

int SLtt_set_mouse_mode (int mode, int force)
{
   /* FIXME: Priority=low */
   (void) mode;
   (void) force;

   return -1;
}
