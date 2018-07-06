/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if !defined(_LOOKUP_H)
#define _LOOKUP_H
DECLARE_LOOKUP_FUN(clan_lookup		);
DECLARE_LOOKUP_FUN(position_lookup	);
DECLARE_LOOKUP_FUN(sex_lookup		);
DECLARE_LOOKUP_FUN(size_lookup		);
DECLARE_LOOKUP_FUN(term_lookup		);
DECLARE_LOOKUP_FUN(attack_lookup	);
DECLARE_LOOKUP_FUN(race_lookup		);
PETITION_DATA *	petition_lookup		args( (const CHAR_DATA *) );
PETITION_DATA * petition_lookup_name	args( (const char *) );
HELP_DATA *	help_lookup		args( (char *) );
HELP_AREA *	had_lookup		args( (char *) );
EXIT_DATA *	exit_lookup		args( (ROOM_INDEX_DATA *, sh_int) );
FIGHT_DATA *	fdata_lookup		args( (CHAR_DATA *, long) );
int		flag_lookup		args( ( const char *, const struct flag_type * ) );
#endif // _LOOKUP_H
