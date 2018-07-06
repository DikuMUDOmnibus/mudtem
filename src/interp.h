/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/* this is a listing of all the commands and command related data */

/* for command types */
#define ML 	MAX_LEVEL	/* implementor */
#define L1	MAX_LEVEL - 1  	/* creator */
#define L2	MAX_LEVEL - 2	/* supreme being */
#define L3	MAX_LEVEL - 3	/* deity */
#define L4 	MAX_LEVEL - 4	/* god */
#define L5	MAX_LEVEL - 5	/* immortal */
#define L6	MAX_LEVEL - 6	/* demigod */
#define L7	MAX_LEVEL - 7	/* angel */
#define L8	MAX_LEVEL - 8	/* avatar */
#define IM	LEVEL_IMMORTAL 	/* avatar */
#define HE	LEVEL_HERO	/* hero */

typedef struct  disabled_data		DISABLED_DATA;

/* one disabled command */
struct disabled_data
{
      DISABLED_DATA *next; /* pointer to next node */
      struct cmd_type const *command; /* pointer to the command struct*/
      char *disabled_by; /* name of disabler */
      sh_int level; /* level of disabler */
};

/* the command table itself */
extern	struct	cmd_type	*cmd_table;
extern		int		MAX_CMD;
extern		DISABLED_DATA	*disabled_first;

#include "command.h"
