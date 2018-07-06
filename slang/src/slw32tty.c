/* Copyright (c) 1992, 1999 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include <windows.h>
#include <winbase.h>

#include "slang.h"
#include "_slang.h"

#ifdef __cplusplus
# define _DOTS_ ...
#else
# define _DOTS_ void
#endif

static int Process_Mouse_Events;

/*----------------------------------------------------------------------*\
 *  Function:	static void set_ctrl_break (int state);
 *
 * set the control-break setting
\*----------------------------------------------------------------------*/
static void set_ctrl_break (int state)
{
}

/*----------------------------------------------------------------------*\
 *  Function:	int SLang_init_tty (int abort_char, int no_flow_control,
 *				    int opost);
 *
 * initialize the keyboard interface and attempt to set-up the interrupt 9
 * handler if ABORT_CHAR is non-zero.
 * NO_FLOW_CONTROL and OPOST are only for compatiblity and are ignored.
\*----------------------------------------------------------------------*/

HANDLE _SLw32_Hstdin = INVALID_HANDLE_VALUE;

int SLang_init_tty (int abort_char, int no_flow_control, int opost)
{
   (void) opost;
   (void) no_flow_control;

   if (_SLw32_Hstdin != INVALID_HANDLE_VALUE)
     return 0;

   if (INVALID_HANDLE_VALUE == (_SLw32_Hstdin = GetStdHandle(STD_INPUT_HANDLE)))
     return -1;

   if (FALSE == SetConsoleMode(_SLw32_Hstdin, ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT))
     {
	_SLw32_Hstdin = INVALID_HANDLE_VALUE;
	return -1;
     }

   return 0;
}
/* SLang_init_tty */

/*----------------------------------------------------------------------*\
 *  Function:	void SLang_reset_tty (void);
 *
 * reset the tty before exiting
\*----------------------------------------------------------------------*/
void SLang_reset_tty (void)
{
   _SLw32_Hstdin = INVALID_HANDLE_VALUE;
   set_ctrl_break (1);
}

static int process_mouse_event (MOUSE_EVENT_RECORD *m, int check_only)
{
   char buf [8];

   if (Process_Mouse_Events == 0)
     return -1;

   if (m->dwEventFlags)
     return -1;			       /* double click or movement event */

   /* A button was either pressed or released.  Now make sure that
    * the shift keys were not also pressed.
    */
   if (m->dwControlKeyState
       & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED
	  |LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED
	  |SHIFT_PRESSED))
     return -1;

   /* We have a simple press or release.  Encode it as an escape sequence
    * and buffer the result.  The encoding is:
    *   'ESC [ M b x y'
    *  where b represents the button state, and x,y represent the coordinates.
    * The ESC is handled by the calling routine.
    */
   if (m->dwButtonState & 1) buf[2] = ' ';
   else if (m->dwButtonState & 2) buf[2] = ' ' + 2;
   else if (m->dwButtonState & 4) buf[2] = ' ' + 1;
   else return -1;

   if (check_only)
     return 0;

   buf[3] = 1 + ' ' + m->dwMousePosition.X;
   buf[4] = 1 + ' ' + m->dwMousePosition.Y;

   buf[0] = '[';
   buf[1] = 'M';

   return SLang_ungetkey_string (buf, 5);
}

/* Return TRUE if the Shift, Ctrl, Alt, Caps Lock, or Num Lock key was pressed.
 */
static int is_shift_key (INPUT_RECORD *pRecord)
{
   unsigned int k;
#if defined(__MINGW32__)
   k = (pRecord->Event.KeyEvent.AsciiChar |
	(pRecord->Event.KeyEvent.wVirtualScanCode << 8));
#else
   k = (pRecord->Event.KeyEvent.uChar.AsciiChar |
	(pRecord->Event.KeyEvent.wVirtualScanCode << 8));
#endif

   return ((k == 0x1d00)
	   || (k == 0x2a00)
	   || (k == 0x3600)
	   || (k == 0x3800)
	   || (k == 0x3a00)
	   || (k == 0x4500));
}

/*----------------------------------------------------------------------*\
 *  Function:	int _SLsys_input_pending (int tsecs);
 *
 *  sleep for *tsecs tenths of a sec waiting for input
\*----------------------------------------------------------------------*/
int _SLsys_input_pending (int tsecs)
{
   INPUT_RECORD record;
   DWORD events_read;
   DWORD ms;

   if (_SLw32_Hstdin == INVALID_HANDLE_VALUE)
     return -1;

   if (tsecs < 0) ms = -tsecs;	       /* specifies 1/1000 */
   else ms = tsecs * 100L;	       /* convert 1/10 to 1/1000 secs */

   if (ms)
     (void) WaitForSingleObject (_SLw32_Hstdin, ms);

   while (1)
     {
	if (0 == PeekConsoleInput(_SLw32_Hstdin, &record, 1, &events_read))
	  /* function failed */
	  return 0;

	if (events_read != 1)
	  return 0;

	switch (record.EventType)
	  {
	   default:
	     break;

	   case MOUSE_EVENT:
	     if (0 == process_mouse_event (&record.Event.MouseEvent, 1))
	       return 1;

	     break;

	   case KEY_EVENT:
	     if (record.Event.KeyEvent.bKeyDown
		 && (0 == is_shift_key (&record)))
	       return 1;
	     break;
	  }

	/* something else is here, so read it and try again */
	(void) ReadConsoleInput(_SLw32_Hstdin, &record, 1, &events_read);
     }
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
   unsigned int ch;
   DWORD events_read;
   INPUT_RECORD record;

   if (_SLw32_Hstdin == INVALID_HANDLE_VALUE)
     return SLANG_GETKEY_ERROR;

   while (1)
     {
	if (!ReadConsoleInput(_SLw32_Hstdin, &record, 1, &events_read))
	  return 0;

	switch (record.EventType)
	  {
	   default:
	     break;

	   case MOUSE_EVENT:
	     if (0 == process_mouse_event (&record.Event.MouseEvent, 0))
	       return 27;	       /* ESC */
	     break;

	   case KEY_EVENT:
	     if ((0 == record.Event.KeyEvent.bKeyDown)
		 || is_shift_key (&record))
	       break;

#if defined(__MINGW32__)
	     ch = record.Event.KeyEvent.AsciiChar;
#else
	     ch = record.Event.KeyEvent.uChar.AsciiChar;
#endif
	     switch (ch)
	       {
		case 8:		       /* ^H --> 127 */
		  if (record.Event.KeyEvent.wVirtualScanCode == 0x0E)
		    ch = 127;
		  break;

		case 0:
		case 0xE0:
		  ch = _SLpc_convert_scancode (record.Event.KeyEvent.wVirtualScanCode);
		  break;

		default:
		  break;
	       }
	     return ch;
	  }
     }
}

/*----------------------------------------------------------------------*\
 *  Function:	int SLang_set_abort_signal (void (*handler)(int));
\*----------------------------------------------------------------------*/
int SLang_set_abort_signal (void (*handler)(int))
{
   if (_SLw32_Hstdin == INVALID_HANDLE_VALUE)
     return -1;

   return 0;
}

int SLtt_set_mouse_mode (int mode, int force)
{
   (void) force;

   Process_Mouse_Events = mode;
   return 0;
}
